/*
Copyright (C) 2025 Slimbook <dev@slimbook.es>

This file is part of libslimbook.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "amdsmu.h"
#include "pci.h"
#include "common.h"

#include <map>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "slimbook.h"

#define DEV_PCI_REG_ADDR_ADDR 0xB8
#define DEV_PCI_REG_DATA_ADDR 0xBC

#define MSG_MSG_ADDR 0x03B10A20
#define MSG_RES_ADDR 0x03B10A80
#define MSG_ARG_BASE_ADDR 0x03B10A88

/* Uses switch range C++ extension, not standard but impl in important compilers */
void _get_design_amd(uint32_t family, uint32_t model, uint32_t* design){
    switch(family){
        case 0x17:
            switch(model){
                case 0x11:
                    *design = DESIGN_RAVEN;
                    return;
                case 0x18:
                    *design = DESIGN_PICASSO;
                    return;
                case 0x20:
                    *design = DESIGN_DALI;
                    return;
                case 0x31:
                    *design = DESIGN_STARSHIP;
                    return;
                case 0x47:
                case 0x87:
                    // figure out
                    return;
                case 0x60:
                    *design = DESIGN_RENOIR;
                    return;
                case 0x68:
                    *design = DESIGN_LUCIENNE;
                    return;
                case 0x71:
                    *design = DESIGN_MATISSE;
                    return;
                case 0x90:
                    *design = DESIGN_VAN_GOGH;
                    return;
                case 0x98:
                    *design = DESIGN_MERO;
                case 0xA0:
                    *design = DESIGN_MENDOCINO;
                    return;
                default:
                    *design = DESIGN_UNKNOWN;
                    return;
            }
            return;
        case 0x19:
            switch(model){
                case 0x1:
                    *design = DESIGN_MILAN;
                    return;
                case 0x8:
                    *design = DESIGN_CHAGALL;
                    return;
                case 0x20 ... 0x2F :
                    *design = DESIGN_VERMEER;
                    return;
                case 0x30 ... 0x3F:
                    *design = DESIGN_BADAMI;
                    return;
                case 0x40 ... 0x4F:
                    *design = DESIGN_REMBRANDT;
                    return;
                case 0x50 ... 0x5F:
                    *design = DESIGN_CEZANNE;
                    return;
                case 0x10 ... 0x1F:
                    *design = DESIGN_STORM_PEAK;
                    return;
                case 0x60 ... 0x6F:
                    *design = DESIGN_RAPHAEL;
                    return;
                case 0x70 ... 0x77:
                    *design = DESIGN_PHOENIX;
                    return;
                case 0x78 ... 0x7F:
                    *design = DESIGN_PHOENIX_2;
                    return;
                case 0xA0 ... 0xAF:
                    *design = DESIGN_GENOA;
                    return;
                default:
                    *design = DESIGN_UNKNOWN;
                    return;
            } 
            case 0x1A:
                switch(model){
                    case 0x0 ... 0xF:
                        *design = DESIGN_TURIN;
                        return;
                    case 0x10 ... 0x1F:
                        *design = DESIGN_TURIN_DENSE;
                        return;
                    case 0x20 ... 0x2F:
                        *design = DESIGN_STRIX_POINT_1;
                        return;
                    case 0x30 ... 0x37:
                        *design = DESIGN_STRIX_POINT_2;
                        return;
                    case 0x38 ... 0x3F:
                        *design = DESIGN_STRIX_HALO;
                        return;
                    case 0x40 ... 0x4F:
                        *design = DESIGN_GRANITE_RIDGE;
                        return;
                    case 0x50 ... 0x5F:
                        *design = DESIGN_FIRE_RANGE;
                        return;
                    case 0x60 ... 0x6F:
                        *design = DESIGN_KRACKAN_POINT_1;
                        return;
                    case 0x70 ... 0x77:
                        *design = DESIGN_SARLAK;
                        return;
                    default:
                        *design = DESIGN_UNKNOWN;
                        return;        
                }  

            default:
                *design = DESIGN_UNKNOWN;
                return;
    }   
}

static smu_amd* _get_smu_amd(){
    pci_access* o = pci_access_alloc();
    pci_dev* dev;
    smu_amd* smu;
    
    smu = (smu_amd*)((char*)malloc(sizeof(smu_amd)));

    pci_init_dev(o);

    /* Assumes device 0000:00:00.0 is free */
    dev = pci_get_dev(o, 0,0,0,0);

    smu->dev = dev;
    smu->msg = MSG_MSG_ADDR;
    smu->res = MSG_RES_ADDR;
    smu->arg_base = MSG_ARG_BASE_ADDR;
    
    return smu;
}

void _clear_smu_amd(smu_amd* smu){
    if(smu != nullptr){
        free(smu);
    }
}

static uint32_t _pci_reg_rd(pci_dev* dev, uint32_t addr){
    pci_write_long(dev, DEV_PCI_REG_ADDR_ADDR, ALIGN(addr, 4));
    return pci_read_long(dev, DEV_PCI_REG_DATA_ADDR);
}

static void _pci_reg_wr(pci_dev* dev, uint32_t addr, uint32_t data){
    pci_write_long(dev, DEV_PCI_REG_ADDR_ADDR, addr);
    pci_write_long(dev, DEV_PCI_REG_DATA_ADDR, data);
}

#define MSG_ARG_ADDR(base, num) ((base) + 4 * (num))

uint32_t _smu_amd_send_req(smu_amd* smu, uint32_t msg, uint32_t* args){
    uint32_t res = 0;

    _pci_reg_wr(smu->dev, smu->res, 0);

    _pci_reg_wr(smu->dev, MSG_ARG_ADDR(smu->arg_base, 0), args[0]);
    _pci_reg_wr(smu->dev, MSG_ARG_ADDR(smu->arg_base, 1), args[1]);

    _pci_reg_wr(smu->dev, smu->msg, msg);

    while(res == 0){
        res = _pci_reg_rd(smu->dev, smu->res);
    }

    args[0] = _pci_reg_rd(smu->dev, MSG_ARG_ADDR(smu->arg_base, 0));
    args[1] = _pci_reg_rd(smu->dev, MSG_ARG_ADDR(smu->arg_base, 1));

    return res;
}

static void* phys_map = MAP_FAILED;

void** get_phys_map(){
    return &phys_map;
}

int _map_dev_addr(uintptr_t addr){
    int dev_fd = open("/dev/mem", O_RDONLY);
    int dev_errno = errno;

    if(dev_fd > 0){
        phys_map = mmap(NULL, 4096, PROT_READ, MAP_SHARED, dev_fd, addr);
        int map_errno = errno;
        close(dev_fd);

        if (phys_map == MAP_FAILED) {
            return map_errno;
        }

        return 0;
    }

    return dev_errno;
}

void _free_map_dev(){
    if(phys_map != MAP_FAILED){
        munmap((void*)phys_map, 4096);
    }
}

uint32_t _request_addr(uint32_t design, uintptr_t* addr, smu_amd** smu, uint32_t* smuargs){
    uint32_t table_msg = -1;

    switch(design){
        case DESIGN_RAVEN:
        case DESIGN_PICASSO:
        case DESIGN_DALI:
            smuargs[0] = 3;
            table_msg = 0xB;
            break;
        
        case DESIGN_RENOIR:
        case DESIGN_LUCIENNE:
        case DESIGN_CEZANNE:
        case DESIGN_REMBRANDT:
        case DESIGN_PHOENIX:
        case DESIGN_PHOENIX_2:
        case DESIGN_STRIX_POINT_1:
        case DESIGN_STRIX_POINT_2:
            table_msg = 0x66;
            break;
        default:
            return -1;
            break;
    }

    if(table_msg != (uint32_t)-1){
        uint32_t res;

        *smu = _get_smu_amd();
        res = _smu_amd_send_req(*smu, table_msg, smuargs);
        
        if(res != 1){
            return -1;
        }

        switch(design){
            case DESIGN_REMBRANDT:
            case DESIGN_PHOENIX:
            case DESIGN_PHOENIX_2:
            case DESIGN_STRIX_POINT_1:
            case DESIGN_STRIX_POINT_2:
                *addr = (uint64_t) smuargs[1] << 32 | smuargs[0];
                return 0;
            default:
                *addr = (uint64_t)smuargs[0];
                return 0;
        }
    }
    else{
        return -1;
    }

}

uint32_t _refresh_table(uint32_t design, smu_amd** smu, uint32_t* smuargs){
    uint32_t table_msg = -1;

    switch(design){
        case DESIGN_RAVEN:
        case DESIGN_PICASSO:
        case DESIGN_DALI:
            smuargs[0] = 3;
            table_msg = 0x3D;
            break;
        
        case DESIGN_RENOIR:
        case DESIGN_LUCIENNE:
        case DESIGN_CEZANNE:
        case DESIGN_REMBRANDT:
        case DESIGN_PHOENIX:
        case DESIGN_PHOENIX_2:
        case DESIGN_STRIX_POINT_1:
        case DESIGN_STRIX_POINT_2:
            table_msg = 0x65;
            break;
        default:
            return -1;
            break;
    }

    if(table_msg != (uint32_t)-1){
        uint32_t res;

        res = _smu_amd_send_req(*smu, table_msg, smuargs);

        if(res == 253){
            usleep(200 * 1000);
            res = _smu_amd_send_req(*smu, table_msg, smuargs);
        }

        if(res == 253){
            return -1;
        }
    }

    return 0;
}
