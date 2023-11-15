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

#include <iostream>
#include <iomanip>
#include <string>
#include <map>

using namespace std;

void show_help()
{
    cout<<"Slimbook control tool"<<endl;
    cout<<"Usage: slimbookctl [command]"<<endl;
    cout<<"\n"<<endl;
    cout<<"Commands:"<<endl;
    cout<<"info: display Slimbook model information"<<endl;
    cout<<"help: show this help"<<endl;
}

void show_info()
{
    map<int,string> yesno = {{0,"no"},{1,"yes"}};
    
    cout<<"kernel:"<<slb_info_kernel()<<"\n";
    
    int64_t uptime = slb_info_uptime();
    int64_t h = uptime / 3600;
    int64_t m = (uptime / 60) % 60;
    int64_t s = uptime % 60;
    
    cout<<"uptime:"<<h<<"h "<<m<<"m "<<s<<"s\n";
    uint64_t tr,ar;
    
    tr = slb_info_total_memory();
    ar = slb_info_available_memory();
    
    uint64_t mb = 1024*1024;
    
    cout<<"memory:"<<ar/mb<<"/"<<tr/mb<<" MB\n";
    
    cout<<"product:"<<slb_info_product_name()<<"\n";
    cout<<"vendor:"<<slb_info_board_vendor()<<"\n";
    cout<<"bios:"<<slb_info_bios_version()<<"\n";
    cout<<"EC:"<<slb_info_ec_firmware_release()<<"\n";
    cout<<"serial:"<<slb_info_product_serial()<<"\n";
    cout<<"model:0x"<<std::hex<<slb_info_get_model()<<"\n";
    
    uint32_t platform = slb_info_get_platform();
    cout<<"platform:0x"<<platform<<"\n";
    
    bool module_loaded = slb_info_is_module_loaded();
    cout<<"module loaded:"<<yesno[module_loaded]<<"\n";
    
    if (module_loaded and platform == SLB_PLATFORM_QC71) {
        uint32_t value = 0;
        
        slb_qc71_fn_lock_get(&value);
        cout<<"fn lock:"<<yesno[value]<<"\n";
        
        slb_qc71_super_lock_get(&value);
        cout<<"super key lock:"<<yesno[value]<<"\n";
        
        slb_qc71_silent_mode_get(&value);
        cout<<"silent mode:"<<yesno[value]<<"\n";
    }
        
    cout<<std::flush;
}

int main(int argc,char* argv[])
{
    string command;
    
    if (argc>1) {
        command = argv[1];
    }
    else {
        show_help();
        return 0;
    }
    
    if (command == "info") {
        show_info();
        return 0;
    }
    
    if (command == "help") {
        show_help();
        return 0;
    }
    
    if (command == "set-kbd-backlight") {
        if (argc<2) {
            return 1; //better return value
        }
        
        uint32_t value = std::stoi(argv[2],0,16);
        
        int status = slb_kbd_backlight_set(0,value);
        
        if (status > 0) {
            cerr<<"Failed to set keyboard backlight:"<<status<<endl;
            return status;
        }
        
        return 0;
    }
    
    if (command == "get-kbd-backlight") {
        uint32_t value;
        int status = slb_kbd_backlight_get(0,&value);
        
        if (status > 0) {
            cerr<<"Failed to retrieve keyboard backlight:"<<status<<endl;
            return status;
        }
        
        cout<<std::hex<<std::setw(6)<<std::setfill('0')<<value<<endl;
        
        return 0;
    }
    
    if (command == "config-load") {
        clog<<"loading slimbook configuration:";
        int status = slb_config_load(0);
        clog<<status<<endl;
    }

    if (command == "config-store") {
        clog<<"storing slimbook configuration:";
        int status = slb_config_store(0);
        clog<<status<<endl;
    }

    return 0;
}
