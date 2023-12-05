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

#include <sys/statvfs.h>
#include <mntent.h>

#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

static string trim(string in)
{
    string out;
    size_t first = 0;
    size_t last = 0;
    bool ffound = false;
    
    for (size_t n=0;n<in.size();n++) {
        char c = in[n];
        
        if (!ffound and c!=' ') {
            ffound = true;
            first = n;
        }
        
        if (c!=' ') {
            last = n;
        }
    }
    out = in.substr(first,last+1);
    
    return out;
}

static string to_human(uint64_t value)
{
    string magnitude = "";
    double tmp = value;
    
    if (tmp > 1024) {
        tmp = tmp / 1024;
        magnitude = "K";
    }
    
    if (tmp > 1024) {
        tmp = tmp / 1024;
        magnitude = "M";
    }
    
    if (tmp > 1024) {
        tmp = tmp / 1024;
        magnitude = "G";
    }
    
    stringstream ss;
    
    ss.precision(2);
    
    ss<<std::fixed<<tmp<<" "<<magnitude;
    
    return ss.str();
    
}

void show_help()
{
    cout<<"Slimbook control tool"<<endl;
    cout<<"Usage: slimbookctl [command]"<<endl;
    cout<<"\n"<<endl;
    cout<<"Commands:"<<endl;
    cout<<"info: display Slimbook model information"<<endl;
    cout<<"get-gbd-backlight: shows current keyboard backlight value in 32bit hexadecimal"<<endl;
    cout<<"set-kbd-backlight HEX: sets keyboard backlight as 32bit hexadecimal"<<endl;
    cout<<"config-load: loads module settings"<<endl;
    cout<<"config-store: stores module settings to disk"<<endl;
    cout<<"help: show this help"<<endl;
}

void show_info()
{
    map<int,string> yesno = {{0,"no"},{1,"yes"}};
    
    int64_t uptime = slb_info_uptime();
    int64_t h = uptime / 3600;
    int64_t m = (uptime / 60) % 60;
    int64_t s = uptime % 60;
    
    cout<<"uptime:"<<h<<"h "<<m<<"m "<<s<<"s\n";
    cout<<"kernel:"<<slb_info_kernel()<<"\n";
    
    uint64_t tr,ar;
    
    tr = slb_info_total_memory();
    ar = slb_info_available_memory();
    
    cout<<"memory free/total:"<<to_human(ar)<<"/"<<to_human(tr)<<"\n";
    
    std::vector<string> mounts = {"/", "/home", "/boot/efi", "/boot"};
    
    FILE* mfile = setmntent("/proc/self/mounts","r");
    struct mntent* ent = getmntent(mfile);
    
    while(ent!=nullptr) {
        string dev = ent->mnt_fsname;
        string dir = ent->mnt_dir;
        
        for (string& m : mounts) {
            
            if (dir == m) {
                struct statvfs stat;
        
                if (statvfs(dir.c_str(),&stat) == 0) {
                    uint64_t fbytes = stat.f_bsize * stat.f_bfree;
                    uint64_t tbytes = stat.f_bsize * stat.f_blocks;
                    
                    cout<<"disk free/total:"<<dir<<" "<<to_human(fbytes)<<"/"<<to_human(tbytes)<<endl;
                }
                
                break;
            }
        
        }
        //cout<<ent->mnt_fsname<<":"<<ent->mnt_dir<<endl;
        ent = getmntent(mfile);
    }
    /*
        struct statvfs stat;
        
        if (statvfs(mp.c_str(),&stat) == 0) {
            cout<<"mount:"<<mp<<endl;
        }
    */
    cout<<"product:"<<slb_info_product_name()<<"\n";
    cout<<"vendor:"<<slb_info_board_vendor()<<"\n";
    cout<<"bios:"<<slb_info_bios_version()<<"\n";
    cout<<"EC:"<<slb_info_ec_firmware_release()<<"\n";
    cout<<"serial:"<<slb_info_product_serial()<<"\n";
    
    // boot mode
    
    slb_smbios_entry_t* entries = nullptr;
    int count = 0;

    int status = slb_smbios_get(&entries,&count);
    if (status == 0) {
        for (int n=0;n<count;n++) {
            if (entries[n].type == 4) {
                string name = trim(entries[n].data.processor.version);
                cout<<"cpu:"<<name<<" x "<<(int)entries[n].data.processor.cores<<endl;
            }
            
            if (entries[n].type == 17) {
                if (entries[n].data.memory_device.type > 2) {
                    cout<<"memory device:"<<entries[n].data.memory_device.size<<" M "<<entries[n].data.memory_device.speed<<" MT/s"<<endl;
                }
            }
        }
        
        slb_smbios_free(entries);
    }
    
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

    if (command == "serial") {
        cout<<slb_info_product_serial()<<"\n";
    }

    return 0;
}
