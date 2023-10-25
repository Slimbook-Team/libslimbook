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

#include <string>
#include <cstring>
#include <fstream>
#include <thread>
#include <vector>

using namespace std;

#define SYSFS_DMI "/sys/devices/virtual/dmi/id/"

thread_local std::string buffer;

struct database_entry_t
{
    const char* product_name;
    const char* board_vendor;
    uint32_t platform;
    uint32_t model;
};

database_entry_t database [] = {

    {"PROX-AMD5", "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_PROX_AMD5},
    {"HERO-RPL-RTX", "SLIMBOOK", SLB_PLATFORM_QC71, SLB_MODEL_HERO_RPL_RTX},
    {0,0,0,0}
};

static void read_device(string path,string& out)
{
    ifstream file;

    file.open(path.c_str());
    std::getline(file,out);
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
        if (platform == SLB_PLATFORM_QC71 and mod == "qc71_laptop") {
            return 1;
        }
        
        if (platform == SLB_PLATFORM_CLEVO and mod == "clevo_platform") {
            return 1;
        }
    }
    
    return 0;
}
