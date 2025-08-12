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
#include <stddef.h>

#define SLB_FAMILY_MASK                 0xffffff00

#define SLB_MODEL_UNKNOWN               0x0000

#define SLB_MODEL_EXECUTIVE             0x0100
#define SLB_MODEL_EXECUTIVE_14_11TH     0x0101 /* left here for compatibility */
#define SLB_MODEL_EXECUTIVE_11TH        0x0101
#define SLB_MODEL_EXECUTIVE_12TH        0x0102
#define SLB_MODEL_EXECUTIVE_13TH        0x0103
#define SLB_MODEL_EXECUTIVE_UC2         0x0104

#define SLB_MODEL_PROX                  0x0200
#define SLB_MODEL_PROX_AMD              0x0201
#define SLB_MODEL_PROX_15_AMD           0x0202
#define SLB_MODEL_PROX_AMD5             0x0203
#define SLB_MODEL_PROX_15_AMD5          0x0204

#define SLB_MODEL_TITAN                 0x0400

#define SLB_MODEL_HERO                  0x0800
#define SLB_MODEL_HERO_RPL_RTX          0x0801

#define SLB_MODEL_ESSENTIAL             0x1000
#define SLB_MODEL_ESSENTIAL_SLIMBOOK    0x1001
#define SLB_MODEL_ESSENTIAL_ESSENTIAL   0x1002
#define SLB_MODEL_ESSENTIAL_15L         0x1003
#define SLB_MODEL_ESSENTIAL_15_AMD_5000 0x1004
#define SLB_MODEL_ESSENTIAL_15_11       0x1005
#define SLB_MODEL_ESSENTIAL_15_AMD_4700 0x1006

#define SLB_MODEL_ELEMENTAL             0x2000
#define SLB_MODEL_ELEMENTAL_15_I12      0x2001
#define SLB_MODEL_ELEMENTAL_14_I12      0x2002
#define SLB_MODEL_ELEMENTAL_15_I12B     0x2003
#define SLB_MODEL_ELEMENTAL_14_I12B     0x2004
#define SLB_MODEL_ELEMENTAL_15_I13      0x2005
#define SLB_MODEL_ELEMENTAL_14_I13      0x2006

#define SLB_MODEL_EXCALIBUR             0x4000
#define SLB_MODEL_EXCALIBUR_16_AMD7     0x4002
#define SLB_MODEL_EXCALIBUR_16_AMD8     0x4003
#define SLB_MODEL_EXCALIBUR_16R_AMD8    0x4004
#define SLB_MODEL_EXCALIBUR_AMD_AI      0x4005

#define SLB_MODEL_HERO_S                0x8000
#define SLB_MODEL_HERO_S_TGL_RTX        0x8001

#define SLB_MODEL_EVO                 0x010000
#define SLB_MODEL_EVO_14_A8           0x010001
#define SLB_MODEL_EVO_15_A8           0x010002
#define SLB_MODEL_EVO_14_AI9_STP      0x010003
#define SLB_MODEL_EVO_15_AI9_STP      0x010004

#define SLB_MODEL_CREATIVE            0x020000
#define SLB_MODEL_CREATIVE_15_A8_RTX  0x020001

#define SLB_MODEL_ZERO              0x01000000
#define SLB_MODEL_ZERO_V4           0x01000001
#define SLB_MODEL_ZERO_V5           0x01000002
#define SLB_MODEL_ZERO_N100_4RJ     0x01000003

#define SLB_MODEL_ONE               0x02000000
#define SLB_MODEL_ONE_AMD8          0x02000001

#define SLB_MODEL_NAS               0x04000000
#define SLB_MODEL_NAS_AMD8_8HDD_4RJ 0x04000001

#define SLB_PLATFORM_UNKNOWN            0x0000
#define SLB_PLATFORM_QC71               0x0100
#define SLB_PLATFORM_CLEVO              0x0200
#define SLB_PLATFORM_Z16                0x0400
#define SLB_PLATFORM_HMT16              0x0800

#define SLB_MAX_PROCESSOR_VERSION  48

#define SLB_SCAN_ENERGY_SAVER_MODE  0xf2
#define SLB_SCAN_BALANCED_MODE      0xf9
#define SLB_SCAN_PERFORMANCE_MODE   0xe2
#define SLB_SCAN_TOUCHPAD_SWITCH    0x76

#define SLB_SCAN_QC71_SUPER_LOCK        0x68
#define SLB_SCAN_QC71_SILENT_MODE       0x69
#define SLB_SCAN_QC71_TOUCHPAD_SWITCH   SLB_SCAN_TOUCHPAD_SWITCH

#define SLB_SCAN_Z16_ENERGY_SAVER_MODE  SLB_SCAN_ENERGY_SAVER_MODE
#define SLB_SCAN_Z16_BALANCED_MODE      SLB_SCAN_BALANCED_MODE
#define SLB_SCAN_Z16_PERFORMANCE_MODE   SLB_SCAN_PERFORMANCE_MODE

#define SLB_SCAN_HMT16_ENERGY_SAVER_MODE  SLB_SCAN_ENERGY_SAVER_MODE
#define SLB_SCAN_HMT16_BALANCED_MODE      SLB_SCAN_BALANCED_MODE
#define SLB_SCAN_HMT16_PERFORMANCE_MODE   SLB_SCAN_PERFORMANCE_MODE

#define SLB_MODULE_NOT_LOADED           0x00
#define SLB_MODULE_LOADED               0x01
#define SLB_MODULE_NOT_NEEDED           0x02
#define SLB_MODULE_UNKNOWN              0x03

#define SLB_QC71_PROFILE_UNKNOWN        0x00
#define SLB_QC71_PROFILE_SILENT         0x01
#define SLB_QC71_PROFILE_NORMAL         0x02
#define SLB_QC71_PROFILE_ENERGY_SAVER   0x01
#define SLB_QC71_PROFILE_BALANCED       0x02
#define SLB_QC71_PROFILE_PERFORMANCE    0x03

#define SLB_TDP_TYPE_UNKNOWN            0x00
#define SLB_TDP_TYPE_INTEL              0x01
#define SLB_TDP_TYPE_AMD                0x02

#define SLB_BAT_STATE_UNKNOWN           0x00
#define SLB_BAT_STATE_CHARGING          0x01
#define SLB_BAT_STATE_DISCHARGING       0x02
#define SLB_BAT_STATE_NOT_CHARGING      0x03
#define SLB_BAT_STATE_FULL              0x04

typedef struct {
    /* device size in bytes */
    uint64_t size;
    
    /* device size unit : 0 MB, 1 KB */
    uint8_t size_unit : 1;

    /* device speed in MT/s */
    uint32_t speed;

    /* device type: see SMBIOS Type 17 for reference */
    uint8_t type;
} slb_smbios_memory_device_t;

typedef struct {
    uint16_t cores;
    uint16_t threads;
    char version[SLB_MAX_PROCESSOR_VERSION];
} slb_smbios_processor_t;

typedef struct {
    uint8_t type;
    uint8_t length;
    uint16_t handle;

    union {
        slb_smbios_memory_device_t memory_device;
        slb_smbios_processor_t processor;
    } data;
} slb_smbios_entry_t;

typedef struct {
    /* battery capacity */
    uint8_t capacity;
    /* battery charge */
    uint32_t charge;
    /*  battery status : 0 Unknown, 1 Charging, 2 Discharging, 3 Not charging, 4 Full */
    uint8_t status : 3;
} slb_sys_battery_info;

typedef struct {
    uint8_t slow;
    uint8_t fast;
    uint8_t sustained;

    /* AMD mentions 3 types of TDP while Intel only shows max, 0 Intel, 1 AMD */
    uint8_t type : 2;
} slb_tdp_info_t;

/* Retrieves DMI info and cache it. No need to call this function */
extern "C" int32_t slb_info_retrieve();

/* Confidence of model guessing. 0 equals exact matching, beyond 2 is assigned as unknown device */
extern "C" int32_t slb_info_confidence();

/* Gets DMI product name */
extern "C" const char* slb_info_product_name();

/* Gets DMI product SKU */
extern "C" const char* slb_info_product_sku();

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

/* Get Slimbook model family */
extern "C" uint32_t slb_info_get_family();

/* Get a human readable family name text */
extern "C" const char* slb_info_get_family_name();

/* Guess Slimbook platform */
extern "C" uint32_t slb_info_get_platform();

/* Finds platform from a given model without query DMI */
extern "C" uint32_t slb_info_find_platform(uint32_t model);

/* Checks if platform module is loaded */
extern "C" uint32_t slb_info_is_module_loaded();

/* Gets system uptime in seconds */
extern "C" int64_t slb_info_uptime();

/* Gets kernel version (long string) */
extern "C" const char* slb_info_kernel();

/* Gets current boot cmdline */
extern "C" const char* slb_info_cmdline(); 

/* Gets installed system memory */
extern "C" uint64_t slb_info_total_memory();

/* Gets available system memory (not used by any process or buffer) */
extern "C" uint64_t slb_info_available_memory();

/* Gets current TDP */
extern "C" slb_tdp_info_t slb_info_get_tdp_info();

/* Gets keyboard device path, or null if does not apply */
extern "C" const char* slb_info_keyboard_device();

/* Gets module evdev device path, or null if does not apply */
extern "C" const char* slb_info_module_device();

/* Gets touchpad device path, or null if does not apply */
extern "C" const char* slb_info_touchpad_device();

/* Gets AC state, usually there is only one AC device so first argument 
will be zero most of times. State value can be 0,1 or 2. See sysfs docs for details */
extern "C" uint32_t slb_info_get_ac_state(int ac,int* state);

/* Query DMI tables  */
extern "C" int slb_smbios_get(slb_smbios_entry_t** entries,int* count);

/* Free smbios entries */
extern "C" int slb_smbios_free(slb_smbios_entry_t* entries);

/* Sets keyboard backlight color. Set model to 0 to guess it */
extern "C" int slb_kbd_backlight_get(uint32_t model, uint32_t* color);

/* Gets keyboard backlight color. Set model to 0 to guess it */
extern "C" int slb_kbd_backlight_set(uint32_t model, uint32_t color);

/* Loads configuration from disk to driver. ie: backlight color */
extern "C" int slb_config_load(uint32_t model);

/* Stores configuration from driver to disk */
extern "C" int slb_config_store(uint32_t model);

/* Gets Manual control status */
extern "C" int slb_qc71_manual_control_get(uint32_t* value);

/* Sets Manual control */
extern "C" int slb_qc71_manual_control_set(uint32_t value);

/* Gets Fn lock status */
extern "C" int slb_qc71_fn_lock_get(uint32_t* value);

/* Sets Fn lock */
extern "C" int slb_qc71_fn_lock_set(uint32_t value);

/* Gets Super lock status */
extern "C" int slb_qc71_super_lock_get(uint32_t* value);

/* Gets Super lock status */
extern "C" int slb_qc71_super_lock_set(uint32_t value);

/* Gets RPM for primary fan */
extern "C" int slb_qc71_primary_fan_get(uint32_t* value);

/* Gets RPM for secondary fan */
extern "C" int slb_qc71_secondary_fan_get(uint32_t* value);

/* Gets battery info */
extern "C" int slb_battery_info_get(slb_sys_battery_info* info);

/* Gets Silent mode status */
extern "C" int slb_qc71_silent_mode_get(uint32_t* value);

/* Sets Silent mode */
extern "C" int slb_qc71_silent_mode_set(uint32_t value);

/* Gets Turbo mode status */
extern "C" int slb_qc71_turbo_mode_get(uint32_t* value);

/* Sets Turbo mode */
extern "C" int slb_qc71_turbo_mode_set(uint32_t value);

/* Gets profile */
extern "C" int slb_qc71_profile_get(uint32_t* value);

/* Sets profile */
extern "C" int slb_qc71_profile_set(uint32_t value);
