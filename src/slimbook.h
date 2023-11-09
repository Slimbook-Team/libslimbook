/*
Copyright (C) 2023 Slimbook <dev@slimbook.es>

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

#include <stdint.h>

#define SLB_MODEL_UNKNOWN               0x0000

#define SLB_MODEL_EXECUTIVE             0x0100
#define SLB_MODEL_EXECUTIVE_14_11TH     0x0101
#define SLB_MODEL_EXECUTIVE_12TH        0x0102

#define SLB_MODEL_PROX                  0x0200
#define SLB_MODEL_PROX_AMD              0x0201
#define SLB_MODEL_PROX_15_AMD           0x0202
#define SLB_MODEL_PROX_AMD5             0x0203
#define SLB_MODEL_PROX_15_AMD5          0x0204

#define SLB_MODEL_TITAN                 0x0400

#define SLB_MODEL_HERO                  0x0800
#define SLB_MODEL_HERO_RPL_RTX          0x0801
#define SLB_MODEL_HERO_S_TGL_RTX        0x0802

#define SLB_MODEL_ESSENTIAL             0x1000
#define SLB_MODEL_ESSENTIAL_SLIMBOOK    0x1001
#define SLB_MODEL_ESSENTIAL_ESSENTIAL   0x1002
#define SLB_MODEL_ESSENTIAL_15L         0x1003
#define SLB_MODEL_ESSENTIAL_15_AMD_5000 0x1004
#define SLB_MODEL_ESSENTIAL_15_11       0x1005

#define SLB_MODEL_ELEMENTAL             0x2000
#define SLB_MODEL_ELEMENTAL_15_I12      0x2001
#define SLB_MODEL_ELEMENTAL_14_I12      0x2002

#define SLB_PLATFORM_UNKNOWN            0x0000
#define SLB_PLATFORM_QC71               0x0100
#define SLB_PLATFORM_CLEVO              0x0200

/* Gets DMI product name */
extern "C" const char* slb_info_product_name();

/* Gets DMI board vendor */
extern "C" const char* slb_info_board_vendor();

/* Gets DMI serial number. Needs root privileges */
extern "C" const char* slb_info_product_serial();

/* Gets DMI BIOS version */
extern "C" const char* slb_info_bios_version();

/* Gets DMI EC firmware release */
extern "C" const char* slb_info_ec_firmware_release();

/* Guess Slimbook model */
extern "C" uint32_t slb_info_get_model();

/* Guess Slimbook platform */
extern "C" uint32_t slb_info_get_platform();

/* Checks if platform module is loaded */
extern "C" uint32_t slb_info_is_module_loaded();

/* Sets keyboard backlight color. Set model to 0 to guess it */
extern "C" int slb_kbd_backlight_get(uint32_t model, uint32_t* color);

/* Gets keyboard backlight color. Set model to 0 to guess it */
extern "C" int slb_kbd_backlight_set(uint32_t model, uint32_t color);

/* Loads configuration from disk to driver. ie: backlight color */
extern "C" int slb_config_load(uint32_t model);

/* Stores configuration from driver to disk */
extern "C" int slb_config_store(uint32_t model);
