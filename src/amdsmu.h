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

#ifndef SLB_AMDSMU_H
#define SLB_AMDSMU_H

#include <cstdint>

typedef enum {
    DESIGN_UNKNOWN = -1,
    /* Family 0x17 */
    DESIGN_RAVEN,
    DESIGN_PICASSO,
    DESIGN_DALI,
    DESIGN_STARSHIP,
    DESIGN_RENOIR,
    DESIGN_LUCIENNE,
    DESIGN_MATISSE,
    DESIGN_VAN_GOGH,
    DESIGN_MERO,
    DESIGN_MENDOCINO,
    /* Family 0x19 */
    DESIGN_MILAN,
    DESIGN_CHAGALL,
    DESIGN_VERMEER,
    DESIGN_BADAMI,
    DESIGN_REMBRANDT,
    DESIGN_CEZANNE,
    DESIGN_STORM_PEAK,
    DESIGN_RAPHAEL,
    DESIGN_PHOENIX,
    DESIGN_PHOENIX_2,
    DESIGN_GENOA,
    /* Family 0x1A */
    DESIGN_TURIN,
    DESIGN_TURIN_DENSE,
    DESIGN_STRIX_POINT_1,
    DESIGN_STRIX_POINT_2,
    DESIGN_STRIX_HALO,
    DESIGN_GRANITE_RIDGE,
    DESIGN_FIRE_RANGE,
    DESIGN_KRACKAN_POINT_1,
    DESIGN_SARLAK,
}_amd_design;

typedef struct _smu_amd{
    struct pci_dev* dev;
    uint32_t msg;
    uint32_t res;
    uint32_t arg_base;
}smu_amd;

/* Frees the smu */
void _clear_smu_amd(smu_amd* smu);

/* Sends request to the smu driver */
uint32_t _smu_amd_send_req(smu_amd* smu, uint32_t msg, uint32_t* args);

/* Gets internal physical map */
void** get_phys_map();

/* Maps internal physical map to addr */
void _map_dev_addr(uintptr_t addr);

/* Frees internal physical map */
void _free_map_dev();

/* Requests the address of the table from the smu driver */
uint32_t _request_addr(uint32_t design, uintptr_t* addr, smu_amd** smu, uint32_t* smuargs);

/* Updates the table values */
uint32_t _refresh_table(uint32_t design, smu_amd** smu, uint32_t* smuargs);

/* Gets current AMD architecture used in the CPU (design) */
void _get_design_amd(uint32_t family, uint32_t model, uint32_t* design);

#endif
