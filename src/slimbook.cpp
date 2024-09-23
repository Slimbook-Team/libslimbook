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

#include <sys/sysinfo.h>

#include <string>
#include <cstring>
#include <fstream>
#include <thread>
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>

using namespace std;

#define SYSFS_DMI "/sys/devices/virtual/dmi/id/"
#define SYSFS_QC71 "/sys/devices/platform/qc71_laptop/"
#define SYSFS_CLEVO "/sys/devices/platform/clevo_platform/"

#define MODULE_QC71 "qc71_laptop"
#define MODULE_CLEVO "clevo_platform"

#define SLB_SUCCESS 0

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

    {"TITAN", 0, "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_TITAN},
    {"HERO-RPL-RTX", 0, "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_HERO_RPL_RTX},
    {"HERO-S-TGL-RTX", 0, "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_HERO_S_TGL_RTX},
    {"SLIMBOOK", 0, "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ESSENTIAL_SLIMBOOK},
    {"ESSENTIAL", 0, "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ESSENTIAL_ESSENTIAL},
    {"Essential15L", 0, "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ESSENTIAL_15L},
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

    {"EXCALIBUR-14-AMD7", 0, "SLIMBOOK", SLB_PLATFORM_Z16, SLB_MODEL_EXCALIBUR_14_AMD7},
    {"EXCALIBUR-16-AMD7", 0, "SLIMBOOK", SLB_PLATFORM_Z16, SLB_MODEL_EXCALIBUR_16_AMD7},
    
    {"ONE-AMD8", 0, "SLIMBOOK", SLB_PLATFORM_UNKNOWN, SLB_MODEL_ONE_AMD8},
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
    {SLB_MODEL_ONE,"one"},
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

static void read_device(string path,string& out)
{
    ifstream file;

    file.open(path.c_str());
    std::getline(file,out);
    file.close();
}

static void write_device(string path,string in)
{
    ofstream file;

    file.open(path.c_str());
    file<<in;
    file.close();
}

static vector<string> get_modules()
{
    vector<string> modules;
    
    ifstream file;
    
    file.open("/proc/modules");
    
    while (file.good()) {
        string module_name;
        string tmp;
        
        file>>module_name;
        std::getline(file,tmp);
        modules.push_back(module_name);
    }
    
    file.close();
    
    return modules;
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

int32_t slb_info_retrieve()
{
    if (info_cached) {
        return 0;
    }
    
    try {
        read_device(SYSFS_DMI"product_name", info_product);
    }
    catch (...) {
        info_product = "<empty>";
    }
    
    try {
        read_device(SYSFS_DMI"product_sku", info_sku);
    }
    catch (...) {
        info_sku = "<empty>";
    }
    
    try {
        read_device(SYSFS_DMI"board_vendor", info_vendor);
    }
    catch (...) {
        info_vendor = "<empty>";
    }
    
    try {
        read_device(SYSFS_DMI"bios_version", info_bios_version);
    }
    catch (...) {
        info_bios_version = "<empty>";
    }
    
    try {
        read_device(SYSFS_DMI"ec_firmware_release", info_ec_firmware_release);
    }
    catch (...) {
        info_ec_firmware_release = "<empty>";
    }
    
    try {
        read_device(SYSFS_DMI"serial", info_serial);
    }
    catch (...) {
        info_serial = "<empty>";
    }
    
    string pretty_product = pretty_string(info_product);
    //string pretty_vendor = pretty_string(info_vendor);
    string pretty_sku = pretty_string(info_sku);
    
    database_entry_t* entry = database;
    database_entry_t* min_entry = entry;
    int min_dist = 0xFFFF;
    
    vector<database_entry_t*> drawn;
    
    while (entry->model > 0) {
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

const char* slb_info_keyboard_device()
{
    uint32_t platform = slb_info_get_platform();
    
    switch (platform) {
        case SLB_PLATFORM_QC71:
        case SLB_PLATFORM_Z16:
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
    
    if (model == SLB_MODEL_HERO_RPL_RTX) {
        try {
            string svalue;
            uint32_t rgb;
            uint32_t ival;
            
            read_device(SYSFS_QC71"kbd_backlight_rgb_red",svalue);
            ival = std::stoi(svalue,0,16);
            float mp = ival/200.0f; //0xC8
            ival = mp * 255.0f;
            rgb = ival<<16;
            
            read_device(SYSFS_QC71"kbd_backlight_rgb_green",svalue);
            ival = std::stoi(svalue,0,16);
            mp = ival/200.0f; //0xC8
            ival = mp * 255.0f;
            rgb = rgb | (ival<<8);
            
            read_device(SYSFS_QC71"kbd_backlight_rgb_blue",svalue);
            ival = std::stoi(svalue,0,16);
            mp = ival/200.0f; //0xC8
            ival = mp * 255.0f;
            rgb = rgb | ival;
            
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
    
    if (model == SLB_MODEL_HERO_RPL_RTX) {
        stringstream ss;
        try {
            uint32_t red = (color & 0x00ff0000) >> 16;
            float mp = red/255.0f;
            red = mp * 0xC8;
            ss<<std::hex<<"0x"<<std::setfill('0')<<std::setw(2)<<red;
            write_device(SYSFS_QC71"kbd_backlight_rgb_red",ss.str());
            
            
            uint32_t green = (color & 0x0000ff00) >> 8;
            mp = green/255.0f;
            green = mp * 0xC8;
            ss.str("");
            ss<<"0x"<<std::setfill('0')<<std::setw(2)<<green;
            write_device(SYSFS_QC71"kbd_backlight_rgb_green",ss.str());
            
            uint32_t blue = (color & 0x000000ff);
            mp = blue/255.0f;
            blue = mp * 0xC8;
            ss.str("");
            ss<<"0x"<<std::setfill('0')<<std::setw(2)<<blue;
            write_device(SYSFS_QC71"kbd_backlight_rgb_blue",ss.str());
            
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


