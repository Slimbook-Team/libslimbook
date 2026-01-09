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

#include "slimbook.h"
#include "configuration.h"
#include "common.h"
#include "amdsmu.h"
#include "pci.h"

#include <cpuid.h>
#include <sys/sysinfo.h>

#include <string>
#include <cstring>
#include <fstream>
#include <thread>
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <filesystem>

using namespace std;

#define SYSFS_DMI "/sys/devices/virtual/dmi/id/"
#define SYSFS_QC71 "/sys/devices/platform/qc71_laptop/"
#define SYSFS_CLEVO "/sys/devices/platform/clevo_platform/"
#define SYSFS_LED_KBD "/sys/class/leds/rgb:kbd_backlight/"

#define MODULE_QC71 "qc71_laptop"
#define MODULE_CLEVO "clevo_platform"

#define SLB_SUCCESS 0

#define SLB_MICROWATTS 1000000

thread_local std::string buffer;

struct database_entry_t
{
    const char* product_name;
    const char* product_sku;
    const char* board_vendor;
    uint32_t platform;
    uint32_t model;
};

database_entry_t database [] = {
    {"PROX-AMD", 0, "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_PROX_AMD},
    {"PROX15-AMD", 0, "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_PROX_15_AMD},
    {"PROX-AMD5", 0, "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_PROX_AMD5},
    {"PROX15-AMD5", 0, "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_PROX_15_AMD5},

    {"Executive", "Executive-RPL", "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_EXECUTIVE_13TH},
    {"Executive", "0001", "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_EXECUTIVE_12TH},
    {"EXECUTIVE-14", 0, "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_EXECUTIVE_11TH},
    {"Executive-14-UC2", 0, "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_EXECUTIVE_UC2},

    {"TITAN", 0, "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_TITAN},
    {"HERO-RPL-RTX", 0, "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_HERO_RPL_RTX},
    {"HERO-S-TGL-RTX", 0, "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_HERO_S_TGL_RTX},
    {"SLIMBOOK", 0, "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ESSENTIAL_SLIMBOOK},
    {"ESSENTIAL", 0, "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ESSENTIAL_ESSENTIAL},
    {"Essential15L", 0, "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ESSENTIAL_15L},
    {"ESSENTIAL-15-AMD", 0, "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ESSENTIAL_15_AMD_4700},
    {"ESS-15-AMD-5", 0, "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ESSENTIAL_15_AMD_5000},
    {"ESSENTIAL-15-11", 0, "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ESSENTIAL_15_11},
    {"ESSENTIAL-15-11 ", 0, "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ESSENTIAL_15_11},

    {"Elemental15-I12", 0, "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ELEMENTAL_15_I12},
    {"Elemental14-I12", 0, "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ELEMENTAL_14_I12},
    {"Elemental15-I12B", 0, "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ELEMENTAL_15_I12B},
    {"ELEMENTAL 15-I12b", 0, "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ELEMENTAL_15_I12B},
    {"Elemental14-I12B", 0, "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ELEMENTAL_14_I12B},
    {"Elemental15-I13", 0, "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ELEMENTAL_15_I13},
    {"Elemental14-I13", 0, "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ELEMENTAL_14_I13},
    {"ELEMENTAL 15-I13", 0, "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ELEMENTAL_15_I13},
    {"ELEMENTAL 14-I13", 0, "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ELEMENTAL_14_I13},

    {"EXCALIBUR-16-AMD7", 0, "SLIMBOOK", SLB_PLATFORM_Z16, SLB_MODEL_EXCALIBUR_16_AMD7},
    {"EXCALIBUR-16-AMD8", 0, "SLIMBOOK", SLB_PLATFORM_Z16, SLB_MODEL_EXCALIBUR_16_AMD8},
    {"EXCALIBUR-16R-AMD8", 0, "SLIMBOOK", SLB_PLATFORM_HMT16, SLB_MODEL_EXCALIBUR_16R_AMD8},
    {"EXCALIBUR-AMD-AI", 0, "SLIMBOOK", SLB_PLATFORM_HMT16, SLB_MODEL_EXCALIBUR_AMD_AI},

    {"EVO14-A8", 0, "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_EVO_14_A8},
    {"EVO15-A8", 0, "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_EVO_15_A8},
    {"EVO14-AI9-STP", 0, "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_EVO_14_AI9_STP},
    {"EVO15-AI9-STP", 0, "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_EVO_15_AI9_STP},

    {"CREA15-A8-RTX", 0, "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_CREATIVE_15_A8_RTX},
    {"CREA15-AI9-RTX5", 0, "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_CREATIVE_15_AI9_RTX5},

    {"ZERO-N100-4RJ", 0, "SLIMBOOK", SLB_PLATFORM_UNKNOWN, SLB_MODEL_ZERO_N100_4RJ},
    {"ZERO-V5", 0, "SLIMBOOK", SLB_PLATFORM_UNKNOWN, SLB_MODEL_ZERO_V5},

    {"ONE-AMD8", 0, "SLIMBOOK", SLB_PLATFORM_UNKNOWN, SLB_MODEL_ONE_AMD8},

    {"NAS-AMD8-8HDD-4RJ", 0, "SLIMBOOK", SLB_PLATFORM_UNKNOWN, SLB_MODEL_NAS_AMD8_8HDD_4RJ},

    {0,0,0,0,0}
};

struct family_t
{
    uint32_t family;
    const char* name;
};

family_t family_database [] = {
    {SLB_MODEL_EXECUTIVE,"executive"},
    {SLB_MODEL_PROX,"prox"},
    {SLB_MODEL_TITAN,"titan"},
    {SLB_MODEL_HERO,"hero"},
    {SLB_MODEL_ESSENTIAL,"essential"},
    {SLB_MODEL_ELEMENTAL,"elemental"},
    {SLB_MODEL_EXCALIBUR,"excalibur"},
    {SLB_MODEL_HERO_S,"hero-s"},
    {SLB_MODEL_ZERO,"zero"},
    {SLB_MODEL_EVO,"evo"},
    {SLB_MODEL_CREATIVE,"creative"},
    {SLB_MODEL_ONE,"one"},
    {SLB_MODEL_NAS,"nas"},
    {SLB_MODEL_UNKNOWN,"unknown"}
};

/* cached info */
bool info_cached = false;
string info_vendor;
string info_product;
string info_sku;
string info_bios_version;
string info_ec_firmware_release;
string info_serial;

uint32_t info_platform;
uint32_t info_model;
int32_t info_confidence;

static vector<string> split(string input,char sep)
{
    vector<string> tmp;
    bool knee = false;
    string current;
    
    for (char c:input) {
        
        if (c != sep) {
            current.push_back(c);
            knee = true;
        }
        else {
            if (knee == true) {
                tmp.push_back(current);
                current="";
                knee = false;
            }
            
        }
    }
    
    if (current.size() > 0) {
        tmp.push_back(current);
    }
    
    return tmp;
}

static int min3i(int a,int b,int c)
{
    if (a <= b && a <= c) {
        return a;
    }
    else {
        if(b <= a && b <= c) {
            return b;
        }
        else {
            if(c <= a && c <= b) {
                return c;
            }
        }
    }

    // may we land here?
    return -1;
}

/*
 Based on this:
 https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C
*/
static int levenshtein(const char* s1, const char* s2)
{
    unsigned int x, y, s1len, s2len;
    s1len = strlen(s1);
    s2len = strlen(s2);
    unsigned int matrix[s2len+1][s1len+1];
    matrix[0][0] = 0;
    for (x = 1; x <= s2len; x++)
        matrix[x][0] = matrix[x-1][0] + 1;
    for (y = 1; y <= s1len; y++)
        matrix[0][y] = matrix[0][y-1] + 1;
    for (x = 1; x <= s2len; x++)
        for (y = 1; y <= s1len; y++)
            matrix[x][y] = min3i(matrix[x-1][y] + 1, matrix[x][y-1] + 1, matrix[x-1][y-1] + (s1[y-1] == s2[x-1] ? 0 : 1));

    return(matrix[s2len][s1len]);
}

static string pretty_string(string src)
{

    bool start = false;
    size_t first = 0;
    size_t end = 0;
    
    for (size_t n=0;n<src.size();n++) {
        char c = src[n];
        
        if (c > 32) {
            if (start == false) {
                first = n;
                start = true;
            }
            end = n;
        }
    }
    
    if (start == false) {
        return "";
    }
    
    string out;
    
    for (size_t n=first;n<(end+1);n++) {
        char c = std::tolower(src[n]);
        out+=c;
    }
    
    return out;
}

static uint32_t get_model_platform(uint32_t model)
{
    database_entry_t* entry = database;

    while (entry->model > 0) {
        if (model == entry->model) {
            return entry->platform;
        }

        entry++;
    }

    return SLB_PLATFORM_UNKNOWN;
}

static void _get_info_dev(string type, string* str){
    try{
        read_device(SYSFS_DMI + type, *str);
    }
    catch(...){
        *str = "<empty>";
    }
}

int32_t slb_info_retrieve()
{
    if (info_cached) {
        return 0;
    }

    _get_info_dev("product_name", &info_product);
    _get_info_dev("product_sku", &info_sku);
    _get_info_dev("board_vendor", &info_vendor);
    _get_info_dev("bios_version", &info_bios_version);
    _get_info_dev("ec_firmware_release", &info_ec_firmware_release);
    _get_info_dev("product_serial", &info_serial);
    
    string pretty_product = pretty_string(info_product);
    string pretty_vendor = pretty_string(info_vendor);
    string pretty_sku = pretty_string(info_sku);
    
    database_entry_t* entry = database;
    database_entry_t* min_entry = entry;
    int min_dist = 0xFFFF;
    
    vector<database_entry_t*> drawn;
    
    while (entry->model > 0) {
        string source_vendor = pretty_vendor;
        string target_vendor = pretty_string(entry->board_vendor);

        // vendor is a strict comparision
        if (source_vendor == target_vendor) {

            string source = pretty_product;
            string target = pretty_string(entry->product_name);

            int dist = levenshtein(source.c_str(),target.c_str());

            if (dist < min_dist) {
                min_dist = dist;
                min_entry = entry;
            }

            if (dist == 0) {
                drawn.push_back(entry);
            }
        }
        entry++;
    }
    
    info_confidence = min_dist;
    
    if (min_dist == 0) {
        if (drawn.size() > 0) {
            database_entry_t* min_sku = drawn[0];
            int min_dist_sku = 0xFFFF;
            
            for (database_entry_t* drawn_entry: drawn) {
                if (drawn_entry->product_sku) {
                    int dist = levenshtein(drawn_entry->product_sku, pretty_sku.c_str());
                    
                    if (dist < min_dist_sku) {
                        min_sku = drawn_entry;
                        min_dist_sku = dist;
                    }
                    
                }
            }
            
            min_entry = min_sku;
        }
    }
    else {
        if (min_dist > 2) {
            info_model = SLB_MODEL_UNKNOWN;
            info_platform = SLB_PLATFORM_UNKNOWN;

            info_cached = true;
            
            return 1;
        }
    }
    
    info_model = min_entry->model;
    info_platform = min_entry->platform;
    
    info_cached = true;
    
    return 0;
}

int32_t slb_info_confidence()
{
    slb_info_retrieve();
    
    return info_confidence;
}

const char* slb_info_product_name()
{
    slb_info_retrieve();
    
    return info_product.c_str();
}

const char* slb_info_product_sku()
{
    slb_info_retrieve();
    
    return info_sku.c_str();
}

const char* slb_info_board_vendor()
{
    slb_info_retrieve();
    
    return info_vendor.c_str();
}

const char* slb_info_product_serial()
{
    slb_info_retrieve();
    
    return info_serial.c_str();
}

const char* slb_info_bios_version()
{
    slb_info_retrieve();
    
    return info_bios_version.c_str();
}

const char* slb_info_ec_firmware_release()
{
    slb_info_retrieve();
    
    return info_ec_firmware_release.c_str();
}

uint32_t slb_info_get_model()
{
    slb_info_retrieve();
    
    return info_model;
}

uint32_t slb_info_get_family()
{
    return slb_info_get_model() & SLB_FAMILY_MASK;
}

const char* slb_info_get_family_name()
{
    uint32_t family = slb_info_get_family();
    
    family_t* f = family_database;
    
    while(f->family != SLB_MODEL_UNKNOWN) {
        if (f->family == family) {
            break;
        }
        f++;
    }
    
    buffer = f->name;
    
    return buffer.c_str();
}

uint32_t slb_info_get_platform()
{
    slb_info_retrieve();

    return info_platform;
}

uint32_t slb_info_find_platform(uint32_t model)
{
    database_entry_t* entry = database;
    
    while (entry->model > 0) {
        if (model == entry->model) {
            return entry->platform;
        }
        
        entry++;
    }

    return SLB_PLATFORM_UNKNOWN;
}

uint32_t slb_info_is_module_loaded()
{
    uint32_t platform = slb_info_get_platform();
    
    if (platform == SLB_PLATFORM_UNKNOWN) {
        return SLB_MODULE_UNKNOWN;
    }
    
    if (platform == SLB_PLATFORM_Z16) {
        return SLB_MODULE_NOT_NEEDED;
    }

    if (platform == SLB_PLATFORM_HMT16) {
        return SLB_MODULE_NOT_NEEDED;
    }

    vector<string> modules = get_modules();
    
    for (string mod : modules) {
        if (platform == SLB_PLATFORM_QC71 and mod == MODULE_QC71) {
            return SLB_MODULE_LOADED;
        }
        
        if (platform == SLB_PLATFORM_CLEVO and mod == MODULE_CLEVO) {
            return SLB_MODULE_LOADED;
        }
    }
    
    return SLB_MODULE_NOT_LOADED;
}

uint32_t slb_info_get_performance_profiles()
{
    uint32_t count = 0;

    uint32_t model = slb_info_get_model();
    uint32_t family = model & SLB_FAMILY_MASK;

    switch (family) {

        case SLB_MODEL_PROX:
        case SLB_MODEL_EXECUTIVE:
            count = 2;

            if (model == SLB_MODEL_EXECUTIVE_UC2) {
                count = 3;
            }
        break;

        case SLB_MODEL_TITAN:
        case SLB_MODEL_HERO:
        case SLB_MODEL_EVO:
        case SLB_MODEL_CREATIVE:
        case SLB_MODEL_EXCALIBUR:
            count = 3;
        break;

    }

    return count;
}

int64_t slb_info_uptime()
{
    struct sysinfo info;
    
    sysinfo(&info);
    
    return info.uptime;
}

const char* slb_info_kernel()
{
    try {
        buffer.clear();
        read_device("/proc/version",buffer);
        return buffer.c_str();
    }
    catch (...) {
        return nullptr;
    }
}

const char* slb_info_cmdline()
{
    try {
        buffer.clear();
        read_device("/proc/cmdline",buffer);
        return buffer.c_str();
    }
    catch (...) {
        return nullptr;
    }
}

uint64_t slb_info_total_memory()
{
    struct sysinfo info;
    
    sysinfo(&info);
    
    return info.totalram;

}

uint64_t slb_info_available_memory()
{
    struct sysinfo info;
    
    sysinfo(&info);
    
    return info.freeram;

}

/* Gets TDP from Zone 0 in CPU */
slb_tdp_info_t _get_TDP_intel()
{
    #define INTEL_RAPL_PATH "/sys/class/powercap/intel-rapl/intel-rapl:0/"
    slb_tdp_info_t tdp = {0};

    if(filesystem::exists(INTEL_RAPL_PATH)){
        try {
            string svalue;
            read_device(INTEL_RAPL_PATH"constraint_0_power_limit_uw", svalue);

            tdp.sustained = atoll(svalue.c_str()) / SLB_MICROWATTS;
            tdp.type = SLB_TDP_TYPE_INTEL;
        }
        catch(...) {
        }
        
        if (tdp.type != 0) {
            // if we achieved to read at least long-term limit, try other ones but may not exist
            try {
                string svalue;
                read_device(INTEL_RAPL_PATH"constraint_1_power_limit_uw", svalue);

                tdp.slow = atoll(svalue.c_str()) / SLB_MICROWATTS;
                
                read_device(INTEL_RAPL_PATH"constraint_2_power_limit_uw", svalue);

                tdp.fast = atoll(svalue.c_str()) / SLB_MICROWATTS;
            }
            catch(...) {
            }
        }
    }
    
    return tdp;
};

/* Gets TDP from smu driver in PCI */
slb_tdp_info_t _get_TDP_amd()
{
    slb_tdp_info_t tdp = {0};

    uint32_t cpuregs[4];
    uint32_t smuargs[2] = {0};

    uint32_t family;
    uint32_t model;
    uint32_t design = 0;

    uintptr_t addr = -1;

    smu_amd* smu = nullptr;

    void** phys_addr = get_phys_map();

    cpuid(1, cpuregs);

    /* follows CPUID AMD spec standard, however, AMD CPUs after 2003 all have base family as 0xF */
    family = ((cpuregs[0] & 0xF00) >> 8) == 0xF ? ((cpuregs[0] & 0xF00) >> 0x8) + (( cpuregs[0] & 0xFF00000) >> 0x14) : (cpuregs[0] & 0xF00) >> 8;
    model = ((cpuregs[0] & 0xF00) >> 8) == 0xF ? ((cpuregs[0] & 0xF0000) >> 0xC ) | ((cpuregs[0] & 0xF0) >> 4): (cpuregs[0] & 0xF0) >> 4;

    _get_design_amd(family, model, &design);

    if(_request_addr(design, &addr, &smu, smuargs) == (uint32_t)-1){
        return tdp;
    }

    if(addr != (uint64_t)-1){
        if(_map_dev_addr(addr)){
            return tdp;
        } 
        _refresh_table(design, &smu, smuargs);

        #define get_prop_from_offs(addr, offs) ((uint8_t)(*(float*)((uintptr_t)*(addr) + (offs))))

        tdp.sustained = get_prop_from_offs(phys_addr, 0x0);
        tdp.fast = get_prop_from_offs(phys_addr, 0x8);
        tdp.slow = get_prop_from_offs(phys_addr, 0x10);
    }

    pci_cleanup(smu->dev);
    _clear_smu_amd(smu);
    _free_map_dev();

    tdp.type = SLB_TDP_TYPE_AMD;

    return tdp;
}

static string _get_cpu_name(){
    slb_smbios_entry_t* entries = nullptr;
    int count = 0;   
    
    if (slb_smbios_get(&entries,&count) == 0) {
        for (int n=0;n<count;n++) {
            if (entries[n].type == 4) {
                return entries[n].data.processor.version;
            }
        }
    }

    return "";
}

slb_tdp_info_t slb_info_get_tdp_info()
{
    slb_tdp_info_t tdp = {0,0,0, .type = SLB_TDP_TYPE_UNKNOWN};
    int32_t cpu_type;
    
    try {
        string name = _get_cpu_name();

        cpu_type = name.find("AMD") != std::string::npos ? SLB_TDP_TYPE_AMD : name.find("Intel") != std::string::npos ? SLB_TDP_TYPE_INTEL : -1;

        switch(cpu_type){
            case SLB_TDP_TYPE_INTEL:
                tdp = _get_TDP_intel();
                break;
            case SLB_TDP_TYPE_AMD:
                tdp = _get_TDP_amd();
                break;
            default:
                break;
        }
    }
    catch(...) {
        // no need to take actions
    }
    
    return tdp;
}

uint32_t slb_info_tdp_get(slb_tdp_info_t* info)
{
    slb_tdp_info_t tdp = {0,0,0, .type = SLB_TDP_TYPE_UNKNOWN};
    int32_t cpu_type;
    
    try {
        string name = _get_cpu_name();

        cpu_type = name.find("AMD") != std::string::npos ? SLB_TDP_TYPE_AMD : name.find("Intel") != std::string::npos ? SLB_TDP_TYPE_INTEL : -1;

        switch(cpu_type){
            case SLB_TDP_TYPE_INTEL:
                tdp = _get_TDP_intel();
                break;
            case SLB_TDP_TYPE_AMD:
                tdp = _get_TDP_amd();
                break;
            default:
                break;
        }
    }
    catch(...) {
        return ENOENT; //select any better?
    }
    
    if (!info) {
        return ENOENT;
    }
    
    *info = tdp;
    
    return 0;
}

const char* slb_info_keyboard_device()
{
    uint32_t platform = slb_info_get_platform();
    
    switch (platform) {
        case SLB_PLATFORM_QC71:
        case SLB_PLATFORM_Z16:
        case SLB_PLATFORM_HMT16:
            buffer = "/dev/input/by-path/platform-i8042-serio-0-event-kbd";
            return buffer.c_str();
        break;
        
        default:
            return nullptr;
    }
}

const char* slb_info_module_device()
{
    uint32_t platform = slb_info_get_platform();
    
    switch (platform) {
        case SLB_PLATFORM_QC71:
            buffer = "/dev/input/by-path/platform-qc71_laptop-event";
            return buffer.c_str();
        break;
        
        default:
            return nullptr;
    }
}

const char* slb_info_touchpad_device()
{
    
    uint32_t platform = slb_info_get_platform();
    
    switch (platform) {
        case SLB_PLATFORM_QC71:
            buffer = "/dev/input/by-path/platform-AMDI0010:01-event-mouse";
            return buffer.c_str();
        break;
        
        default:
            return nullptr;
    }
}

uint32_t slb_info_get_ac_state(int ac,int* state)
{
    stringstream ss;
    
    ss<<"/sys/class/power_supply/AC"<<ac<<"/online";
    
    try {
        string value;
        
        read_device(ss.str(),value);
        *state = std::stoi(value);
    }
    catch (...) {
        return ENOENT;
    }
    
    return 0;
}

int slb_kbd_backlight_get(uint32_t model, uint32_t* color)
{
    if (color == nullptr) {
        return EINVAL;
    }
    
    if (model == 0) {
        model = slb_info_get_model();
    }
    
    if (model == 0) {
        return ENOENT;
    }
    
    if (model == SLB_MODEL_HERO_RPL_RTX or model == SLB_MODEL_CREATIVE_15_A8_RTX) {
        try {
            string svalue;
            
            read_device(SYSFS_LED_KBD"multi_intensity",svalue);
            vector<string> pl = split(svalue,' ');
            
            uint32_t red,green,blue;
            
            red = std::stoi(pl[0],0,0);
            green = std::stoi(pl[1],0,0);
            blue = std::stoi(pl[2],0,0);
            
            uint32_t rgb = blue;
            rgb = rgb | (green << 8);
            rgb = rgb | (red << 16);
            
            *color = rgb;
            
            return 0;
        }
        catch(...) {
            return EIO;
        }
    }
    
    if ((model & SLB_MODEL_ELEMENTAL) > 0 or model == SLB_MODEL_HERO_S_TGL_RTX) {
        try {
            string svalue;
            uint32_t ival;
            
            read_device(SYSFS_CLEVO"color_left",svalue);
            ival = std::stoi(svalue,0,16);
            
            *color = ival;
            
            return 0;
        }
        catch (...) {
            return EIO;
        }
            
    }
    
    return ENOENT;
}

int slb_kbd_backlight_set(uint32_t model, uint32_t color)
{
    if (model == 0) {
        model = slb_info_get_model();
    }
    
    if (model == 0) {
        return ENOENT;
    }
    
    if (model == SLB_MODEL_HERO_RPL_RTX or model == SLB_MODEL_CREATIVE_15_A8_RTX) {
        stringstream ss;
        try {
            uint32_t red = (color & 0x00ff0000) >> 16;
            uint32_t green = (color & 0x0000ff00) >> 8;
            uint32_t blue = (color & 0x000000ff);
            ss<<red<<" "<<green<<" "<<blue;
            write_device(SYSFS_LED_KBD"multi_intensity",ss.str());
            
            return 0;
        }
        catch(...) {
            return EIO;
        }
    }
    
    if ((model & SLB_MODEL_ELEMENTAL) > 0 or model == SLB_MODEL_HERO_S_TGL_RTX) {
        try {
            stringstream ss;
            ss<<std::hex<<"0x"<<std::setfill('0')<<std::setw(6)<<color;
            write_device(SYSFS_CLEVO"color_left",ss.str());
            
            return 0;
        }
        catch (...) {
            return EIO;
        }
    }
    
    return ENOENT;
}

int slb_kbd_brightness_get(uint32_t model, uint32_t* brightness)
{
    string svalue;
    
    if (model == 0) {
        model = slb_info_get_model();
    }
    
    if (model == 0) {
        return ENOENT;
    }
    
    if (model == SLB_MODEL_HERO_RPL_RTX or model == SLB_MODEL_CREATIVE_15_A8_RTX) {
        try {
            read_device(SYSFS_LED_KBD"brightness",svalue);
            *brightness = std::stoi(svalue,0,0);

            return 0;
        }
        catch(...) {
            return EIO;
        }
    }
    else {
        /* this is workaround for rgb-keyboard on clevo based models */
        *brightness = 0xff;

        return 0;
    }
    
    return ENOENT;
}

int slb_kbd_brightness_set(uint32_t model, uint32_t brightness)
{
    if (model == 0) {
        model = slb_info_get_model();
    }
    
    if (model == 0) {
        return ENOENT;
    }
    
    if (model == SLB_MODEL_HERO_RPL_RTX or model == SLB_MODEL_CREATIVE_15_A8_RTX) {
        try {
            stringstream ss;
            ss<<brightness;
            write_device(SYSFS_LED_KBD"brightness",ss.str());

            return 0;
        }
        catch(...) {
            return EIO;
        }
    }
    
    return ENOENT;
}

int slb_kbd_brightness_max(uint32_t model, uint32_t* max)
{
    string svalue;
    
    if (model == 0) {
        model = slb_info_get_model();
    }
    
    if (model == 0) {
        return ENOENT;
    }
    
    if (model == SLB_MODEL_HERO_RPL_RTX or model == SLB_MODEL_CREATIVE_15_A8_RTX) {
        try {
            read_device(SYSFS_LED_KBD"max_brightness",svalue);
            *max = std::stoi(svalue,0,0);
        }
        catch(...) {
            return EIO;
        }
    }
    else {
        /* this is workaround for rgb-keyboard on clevo based models */
        *max = 0xff;
    }
    
    return ENOENT;
}

int slb_config_load(uint32_t model)
{
    if (model == 0) {
        model = slb_info_get_model();
    }
    
    if (model == 0) {
        return ENOENT;
    }
    
    // uint32_t platform = get_model_platform(model);
    bool module_loaded = slb_info_is_module_loaded();

    Configuration conf;
    try {
        conf.load();
    }
    catch(...) {
        return EIO;
    }
    
    if (module_loaded and model == SLB_MODEL_HERO_RPL_RTX) {
        uint32_t backlight;

        if (conf.find_u32("qc71.hero.backlight",backlight)) {
            slb_kbd_backlight_set(model,backlight);
        }
    }

    if (module_loaded and model == SLB_MODEL_CREATIVE_15_A8_RTX) {
        uint32_t backlight;

        if (conf.find_u32("qc71.creative.backlight",backlight)) {
            slb_kbd_backlight_set(model,backlight);
        }
    }
    
    if (module_loaded and ((model & SLB_MODEL_ELEMENTAL) > 0 or model == SLB_MODEL_HERO_S_TGL_RTX)) {
        uint32_t backlight;

        if (conf.find_u32("clevo.backlight",backlight)) {
            slb_kbd_backlight_set(model,backlight);
        }
    }

    return 0;
}

int slb_config_store(uint32_t model)
{
    if (model == 0) {
        model = slb_info_get_model();
    }
    
    if (model == 0) {
        return ENOENT;
    }

    uint32_t platform = get_model_platform(model);
    bool module_loaded = slb_info_is_module_loaded();
    
    Configuration conf;

    try {
        conf.load();

        conf.set_u32("version",1);
        conf.set_u32("model",model);
        conf.set_u32("platform",platform);

        if (module_loaded and model == SLB_MODEL_HERO_RPL_RTX) {
            uint32_t backlight = 0;

            slb_kbd_backlight_get(model,&backlight);
            conf.set_u32("qc71.hero.backlight",backlight);
        }

        if (module_loaded and model == SLB_MODEL_CREATIVE_15_A8_RTX) {
            uint32_t backlight = 0;

            slb_kbd_backlight_get(model,&backlight);
            conf.set_u32("qc71.creative.backlight",backlight);
        }
        
        if (module_loaded and ((model & SLB_MODEL_ELEMENTAL) > 0 or model == SLB_MODEL_HERO_S_TGL_RTX)) {
        
            uint32_t backlight = 0;

            slb_kbd_backlight_get(model,&backlight);
            conf.set_u32("clevo.backlight",backlight);
        }

        conf.store();
    }
    catch(...) {
        cerr<<"Something went wrong"<<endl;
        return EIO;
    }
    
    return 0;
}

int slb_qc71_manual_control_get(uint32_t* value)
{
    if (value == nullptr) {
        return EINVAL;
    }
    
    try {
        string svalue;
        read_device(SYSFS_QC71"manual_control",svalue);
        *value = std::stoi(svalue,0,10);
    }
    catch (...) {
        return EIO;
    }
    
    return SLB_SUCCESS;
}

int slb_qc71_manual_control_set(uint32_t value)
{
    try {
        stringstream ss;
        ss<<value;
        write_device(SYSFS_QC71"manual_control",ss.str());
    }
    catch (...) {
        return EIO;
    }
    
    return SLB_SUCCESS;
}

int slb_qc71_fn_lock_get(uint32_t* value)
{
    if (value == nullptr) {
        return EINVAL;
    }
    
    try {
        string svalue;
        read_device(SYSFS_QC71"fn_lock",svalue);
        *value = std::stoi(svalue,0,10);
    }
    catch (...) {
        return EIO;
    }
    
    return SLB_SUCCESS;
}

int slb_qc71_fn_lock_set(uint32_t value)
{
    try {
        stringstream ss;
        ss<<value;
        write_device(SYSFS_QC71"fn_lock",ss.str());
    }
    catch (...) {
        return EIO;
    }
    
    return SLB_SUCCESS;
}

int slb_qc71_super_lock_get(uint32_t* value)
{
    if (value == nullptr) {
        return EINVAL;
    }
    
    try {
        string svalue;
        read_device(SYSFS_QC71"super_key_lock",svalue);
        *value = std::stoi(svalue,0,10);
    }
    catch (...) {
        return EIO;
    }
    
    return SLB_SUCCESS;
}

int slb_qc71_super_lock_set(uint32_t value)
{
    try {
        stringstream ss;
        ss<<value;
        write_device(SYSFS_QC71"super_key_lock",ss.str());
    }
    catch (...) {
        return EIO;
    }

    return SLB_SUCCESS;
}

#define QC71_HWMON SYSFS_QC71"hwmon/"

static int _slb_qc71_fan_get_common(string fan, uint32_t* value){
    string spath;

    find_file(QC71_HWMON, fan, spath);

    if (value == nullptr ) {
        return EINVAL;
    }
    
    try {
        if(spath.size() == 0){
            *value = -1;
        }
        else{
            string svalue;

            read_device(spath+fan,svalue);
            *value = std::stoi(svalue,0,10);
        }
    }
    catch (...) {
        return EIO;
    }
    
    return SLB_SUCCESS;
}

int slb_qc71_primary_fan_get(uint32_t* value){
    return _slb_qc71_fan_get_common("fan1_input", value);
}

int slb_qc71_secondary_fan_get(uint32_t* value){
    return _slb_qc71_fan_get_common("fan2_input", value);

}

#define SYS_PWS "/sys/class/power_supply/"

int slb_battery_info_get(slb_sys_battery_info* info){
    if(info == nullptr){
        return EINVAL;
    }

    if(!filesystem::exists(SYS_PWS"/BAT0/")){
        return ENOENT;
    }
    
    try {
        string svalue;

        read_device(SYS_PWS"/BAT0/capacity",svalue);
        info->capacity = std::stoi(svalue,0,10);

        read_device(SYS_PWS"/BAT0/charge_now", svalue);
        info->charge = (std::stoi(svalue,0,10) / 100);
        
        read_device(SYS_PWS"/BAT0/status",svalue);

        if(strcmp(svalue.c_str(), "Charging") == 0){
            info->status = 1;
        }
        else if(strcmp(svalue.c_str(), "Discharging") == 0){
            info->status = 2;
        }
        else if(strcmp(svalue.c_str(), "Not charging") == 0){
            info->status = 3;
        }
        else if(strcmp(svalue.c_str(), "Full") == 0){
            info->status = 4;
        }
        else {
            info->status = 0;
        }
    }
    catch (...) {
        return EIO;
    }

    return SLB_SUCCESS;
}

int slb_qc71_silent_mode_get(uint32_t* value)
{
    if (value == nullptr) {
        return EINVAL;
    }
    
    try {
        string svalue;
        read_device(SYSFS_QC71"silent_mode",svalue);
        *value = std::stoi(svalue,0,10);
    }
    catch (...) {
        return EIO;
    }
    
    return SLB_SUCCESS;
}

int slb_qc71_silent_mode_set(uint32_t value)
{
    try {
        stringstream ss;
        ss<<value;
        write_device(SYSFS_QC71"silent_mode",ss.str());
    }
    catch (...) {
        return EIO;
    }

    return SLB_SUCCESS;
}

int slb_qc71_turbo_mode_get(uint32_t* value)
{
    if (value == nullptr) {
        return EINVAL;
    }
    
    try {
        string svalue;
        read_device(SYSFS_QC71"turbo_mode",svalue);
        *value = std::stoi(svalue,0,10);
    }
    catch (...) {
        return EIO;
    }
    
    return SLB_SUCCESS;
}

int slb_qc71_turbo_mode_set(uint32_t value)
{
    try {
        stringstream ss;
        ss<<value;
        write_device(SYSFS_QC71"turbo_mode",ss.str());
    }
    catch (...) {
        return EIO;
    }

    return SLB_SUCCESS;
}

int slb_qc71_profile_get(uint32_t* value)
{
    if (value == nullptr) {
        return EINVAL;
    }
    
    try {
        string svalue;
        read_device(SYSFS_QC71"performance_mode",svalue);
        *value = std::stoi(svalue,0,10);
    }
    catch (...) {
        return EIO;
    }
    
    return SLB_SUCCESS;
}

int slb_qc71_profile_set(uint32_t value)
{
    try {
        stringstream ss;
        ss<<value;
        write_device(SYSFS_QC71"performance_mode",ss.str());
    }
    catch (...) {
        return EIO;
    }

    return SLB_SUCCESS;
}

int slb_qc71_custom_tdp_get(uint32_t* pl1, uint32_t* pl2, uint32_t* pl4)
{
    if (pl1 == nullptr or pl2 == nullptr or pl4 == nullptr) {
        return EINVAL;
    }
    
    try {
        string svalue;
        read_device(SYSFS_QC71"custom_tdp",svalue);
        vector<string> pl = split(svalue,' ');
        
        *pl1 = std::stoi(pl[0],0,0);
        *pl2 = std::stoi(pl[1],0,0);
        *pl4 = std::stoi(pl[2],0,0);
    }
    catch (...) {
        return EIO;
    }
    
    return SLB_SUCCESS;
}

int slb_qc71_custom_tdp_set(uint32_t pl1, uint32_t pl2, uint32_t pl4)
{
    const uint32_t max_tdp = 80;
    
    pl1 = std::min(pl1,max_tdp);
    pl2 = std::min(pl2,max_tdp);
    pl4 = std::min(pl4,max_tdp);
    
    try {
        stringstream ss;
        ss<<pl1<<" "<<pl2<<" "<<pl4;
        write_device(SYSFS_QC71"custom_tdp",ss.str());
    }
    catch (...) {
        return EIO;
    }

    return SLB_SUCCESS;
}
