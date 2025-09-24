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

#include "pci.h"
#include "common.h"

#include "stdlib.h"
#include "stdio.h"
#include "fcntl.h"
#include "unistd.h"
#include <string.h>
#include <string>
#include <iostream>

static void _pci_get_info(pci_dev*, std::string, std::string&);

static void _init_sysfs_pci(pci_access* a){
    if(a != NULL){
        a->fd = INT32_MAX;
        a->path = "/sys/bus/pci";
    } 
}

/* 0 RO, 1 read-write */
static int32_t _pci_prep_rw(pci_dev* d, int32_t type){
    pci_access* a = d->access;

    if(a->fd == INT32_MAX || a->fd < 0){
        std::string str;

        _pci_get_info(d, "config", str);

        a->fd = open(str.c_str(), type == 0 ? O_RDONLY : O_RDWR);
    }
 
    return a->fd;
}

static void _read_sysfs_pci(pci_dev* d, int32_t pos, char* buf, size_t len){
    int32_t fd = _pci_prep_rw(d, 0);
    
    pread(fd, buf, len, pos);
}

static size_t _write_sysfs_pci(pci_dev* d, int32_t pos, char* buf, size_t len){
    int32_t fd = _pci_prep_rw(d, 1);

    return pwrite(fd, buf, len, pos); 
}

pci_procs sysfs_procs = {
    &_init_sysfs_pci,
    &_read_sysfs_pci,
    &_write_sysfs_pci,
};

static void _pci_read(pci_dev* dev, void* dataPtr, int32_t pos, size_t len){
    if(!(pos & (len -1))){
        dev->procs->read(dev, pos, (char*)dataPtr, len);
    }
}

uint8_t pci_read_byte(pci_dev* dev, int32_t pos){
    uint8_t res;

    _pci_read(dev, &res, pos, sizeof(uint8_t));

    return res;
}

uint16_t pci_read_short(pci_dev* dev, int32_t pos){
    uint16_t res;

    _pci_read(dev, &res, pos, sizeof(uint16_t));

    return res;
}

uint32_t pci_read_long(pci_dev* dev, int32_t pos){
    uint32_t res;

    _pci_read(dev, &res, pos, sizeof(uint32_t));

    return res;
}

size_t _pci_write(pci_dev* dev, void* dataPtr, int32_t pos, size_t len){
    if(!(pos & (len -1))){
        return dev->procs->write(dev, pos, (char*)dataPtr, len);
    }
    return 0;
}

void pci_write_byte(pci_dev* dev, int32_t pos, uint8_t data){
    _pci_write(dev, &data, pos, sizeof(uint8_t));
}

void pci_write_short(pci_dev* dev, int32_t pos, uint16_t data){
    uint16_t vdata = check_endianness() == 0 ? data : swap16(data);
    
    _pci_write(dev, &vdata, pos, sizeof(uint16_t));
}

void pci_write_long(pci_dev* dev, int32_t pos, uint32_t data){
    uint32_t vdata = check_endianness() == 0 ? data : swap32(data);
    
    _pci_write(dev, &vdata, pos, sizeof(uint32_t));
}

pci_access* pci_access_alloc(){
    pci_access* a = (pci_access*)(char*)calloc(1, sizeof(pci_access));

    return a != nullptr ? a : nullptr;
}

static void _pci_alloc_dev(pci_dev** d, pci_access* a){
    pci_dev* dev = *d;

    dev->access = a;
    dev->procs = a->procs;
}

void _pci_get_info(pci_dev* dev, std::string info, std::string& str){
    char buf[0x400];

    snprintf(buf, sizeof(buf), "%s/devices/%04x:%02x:%02x.%d/%s", dev->access->path.c_str(), dev->dom, dev->bus, dev->dev, dev->fun, info.c_str());

    str = buf;
} 

pci_dev* pci_get_dev(pci_access* a, int32_t domain, int32_t dev, int32_t bus, int32_t func){
    pci_dev* d = (pci_dev*)(char*)calloc(1, sizeof(pci_dev));

    if(d == nullptr){
        return nullptr;
    }

    _pci_alloc_dev(&d, a);
    
    return d;
}

void pci_init_dev(pci_access* a){
    if(a != nullptr){
        a->procs = &sysfs_procs;

        a->procs->init(a);
    }
}

void pci_cleanup(pci_dev* dev){
    close(dev->access->fd);
    free(dev->access);
    free(dev);
}
