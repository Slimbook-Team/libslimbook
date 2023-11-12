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

thread_local std::string buffer;

struct database_entry_t
{
    const char* product_name;
    const char* board_vendor;
    uint32_t platform;
    uint32_t model;
};

database_entry_t database [] = {
    {"PROX-AMD", "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_PROX_AMD},
    {"PROX15-AMD", "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_PROX_15_AMD},
    {"PROX-AMD5", "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_PROX_AMD5},
    {"PROX15-AMD5", "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_PROX_15_AMD5},
    {"Executive", "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_EXECUTIVE_12TH},
    {"EXECUTIVE-14", "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_EXECUTIVE_14_11TH},
    {"TITAN", "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_TITAN},
    {"HERO-RPL-RTX", "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_HERO_RPL_RTX},
    {"HERO-S-TGL-RTX", "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_HERO_S_TGL_RTX},
    {"SLIMBOOK", "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ESSENTIAL_SLIMBOOK},
    {"ESSENTIAL", "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ESSENTIAL_ESSENTIAL},
    {"Essential15L", "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ESSENTIAL_15L},
    {"ESS-15-AMD-5", "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ESSENTIAL_15_AMD_5000},
    {"ESSENTIAL-15-11", "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ESSENTIAL_15_11},
    {"ESSENTIAL-15-11 ", "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ESSENTIAL_15_11},
    {"Elemental15-I12", "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ELEMENTAL_15_I12},
    {"Elemental14-I12", "SLIMBOOK", SLB_PLATFORM_CLEVO, SLB_MODEL_ELEMENTAL_14_I12},
    {0,0,0,0}
};

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

const char* slb_info_product_name()
{
    try {
        buffer.clear();
        read_device(SYSFS_DMI"product_name",buffer);
        return buffer.c_str();
    }
    catch (...) {
        return nullptr;
    }
}

const char* slb_info_board_vendor()
{
    try {
        buffer.clear();
        read_device(SYSFS_DMI"board_vendor",buffer);
        return buffer.c_str();
    }
    catch (...) {
        return nullptr;
    }
}

const char* slb_info_product_serial()
{
    try {
        buffer.clear();
        read_device(SYSFS_DMI"product_serial",buffer);
        return buffer.c_str();
    }
    catch (...) {
        return nullptr;
    }
}

const char* slb_info_bios_version()
{
    try {
        buffer.clear();
        read_device(SYSFS_DMI"bios_version",buffer);
        return buffer.c_str();
    }
    catch (...) {
        return nullptr;
    }
}

const char* slb_info_ec_firmware_release()
{
    try {
        buffer.clear();
        read_device(SYSFS_DMI"ec_firmware_release",buffer);
        return buffer.c_str();
    }
    catch (...) {
        return nullptr;
    }
}

uint32_t slb_info_get_model()
{
    string product = slb_info_product_name();
    string vendor = slb_info_board_vendor();

    database_entry_t* entry = database;
    
    while (entry->model > 0) {
        if (product == entry->product_name and vendor == entry->board_vendor) {
            return entry->model;
        }
        
        entry++;
    }
    
    return SLB_MODEL_UNKNOWN;
}

uint32_t slb_info_get_platform()
{
    string product = slb_info_product_name();
    string vendor = slb_info_board_vendor();

    database_entry_t* entry = database;
    
    while (entry->model > 0) {
        if (product == entry->product_name and vendor == entry->board_vendor) {
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
        return 0;
    }
    
    vector<string> modules = get_modules();
    
    for (string mod : modules) {
        if (platform == SLB_PLATFORM_QC71 and mod == MODULE_QC71) {
            return 1;
        }
        
        if (platform == SLB_PLATFORM_CLEVO and mod == MODULE_CLEVO) {
            return 1;
        }
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
    
    if (model == SLB_MODEL_HERO_RPL_RTX) {
        try {
            string svalue;
            uint32_t rgb;
            uint32_t ival;
            
            read_device(SYSFS_QC71"kbd_backlight_rgb_red",svalue);
            ival = std::stoi(svalue,0,16);
            rgb = ival<<16;
            
            read_device(SYSFS_QC71"kbd_backlight_rgb_green",svalue);
            ival = std::stoi(svalue,0,16);
            rgb = rgb | (ival<<8);
            
            read_device(SYSFS_QC71"kbd_backlight_rgb_blue",svalue);
            ival = std::stoi(svalue,0,16);
            rgb = rgb | ival;
            
            *color = rgb;
            
            return 0;
        }
        catch(...) {
            return EIO;
        }
    }
    
    if (model == SLB_MODEL_ELEMENTAL_15_I12 or model == SLB_MODEL_HERO_S_TGL_RTX) {
        //TODO: clevo backlight here
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
            ss<<std::hex<<"0x"<<std::setfill('0')<<std::setw(2)<<red;
            write_device(SYSFS_QC71"kbd_backlight_rgb_red",ss.str());
            
            ss.str("");
            uint32_t green = (color & 0x0000ff00) >> 8;
            ss<<"0x"<<std::setfill('0')<<std::setw(2)<<green;
            write_device(SYSFS_QC71"kbd_backlight_rgb_green",ss.str());
            
            uint32_t blue = (color & 0x000000ff);
            ss.str("");
            ss<<"0x"<<std::setfill('0')<<std::setw(2)<<blue;
            write_device(SYSFS_QC71"kbd_backlight_rgb_blue",ss.str());
            
            return 0;
        }
        catch(...) {
            return EIO;
        }
    }
    
    if (model == SLB_MODEL_ELEMENTAL_15_I12 or model == SLB_MODEL_HERO_S_TGL_RTX) {
        //TODO: clevo backlight here
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

        conf.store();
    }
    catch(...) {
        cerr<<"Something went wrong"<<endl;
        return EIO;
    }
    
    return 0;
}

