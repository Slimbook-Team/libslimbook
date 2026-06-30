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
#include "ite8291r3.h"

#include <fcntl.h>
#include <linux/hidraw.h>
#include <linux/input.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <filesystem>
#include <regex>
#include <iostream>

using namespace std;

struct hidraw_devinfo ITE8291R3::get_info(string device)
{
    struct hidraw_devinfo info = {0};

    int fd = open(device.c_str(), O_RDWR | O_NONBLOCK);
    
    if (fd < 0) {
        cerr<<"Failed to open hidraw device:"<<device<<endl;
        return info;
    }
    
    if (ioctl(fd, HIDIOCGRAWINFO, &info) < 0) {
        cerr<<"Failed to get hidraw info:"<<device<<endl;
    }
    
    close(fd);
    
    return info;
}

vector<string> ITE8291R3::list()
{
    vector<string> devices;
    const std::regex hidraw_regex("hidraw[0-9]+");
    for (auto const& node : std::filesystem::directory_iterator{"/dev/"}) {
        if (!node.is_directory()) {
            string parent = node.path().filename();
            if (std::regex_match(parent,hidraw_regex)) {
            
                struct hidraw_devinfo info = ITE8291R3::get_info(node.path());
                
                if (info.vendor == SLB_VENDOR_ID_ITE) {
                    //TODO: check product ID
                    devices.push_back(node.path());
                }
            }
        }
    }
    
    return devices;
}

ITE8291R3::ITE8291R3() : m_ready(false)
{
    vector<string> devices = ITE8291R3::list();
    
    if (devices.size() > 0) {
        m_device = devices[0];
        m_ready = true;
    }
}

ITE8291R3::ITE8291R3(string device) : m_device(device), m_ready(false)
{

}

ITE8291R3::~ITE8291R3()
{
}

map<uint32_t,uint32_t> ITE8291R3::fetch()
{
    map<uint32_t,uint32_t> values;
    
    char buffer[64];
    
    int fd = open(m_device.c_str(), O_RDWR | O_NONBLOCK);
    
    if (fd > 0) {
        
        buffer[0] = 0;
        buffer[1] = ITE8291R3_GET_EFFECT;
        
        int res = ioctl(fd, HIDIOCSFEATURE(ITE8291R3_HID_REPORT_LENGTH + 1), buffer);
        
        if (res < 0) {
            cerr<<"Failed to set GET_EFFECT command"<<endl;
            return values;
        }
        
        buffer[0] = 0;
        res = ioctl(fd, HIDIOCGFEATURE(64), buffer);
        
        if (res < 0) {
            cerr<<"Failed to fetch feature report"<<endl;
            return values;
        }
        
        values[SLB_KBL_PROPERTY_EFFECT] = buffer[3];
        values[SLB_KBL_PROPERTY_BRIGHTNESS] = buffer[5];
        
        close(fd);
    }
    
    return values;
}

void ITE8291R3::set_effect(uint32_t effect, map<uint32_t,uint32_t> properties)
{
    char buffer[64];
    
    int fd = open(m_device.c_str(), O_RDWR | O_NONBLOCK);
    
    if (fd > 0) {
    
        uint32_t brightness = properties[SLB_KBL_PROPERTY_BRIGHTNESS];
        
        switch (brightness) {
            case SLB_KBL_BRIGHTNESS_OFF:
                brightness = 0;
            break;
            
            case SLB_KBL_BRIGHTNESS_FULL:
                brightness = 0x32;
            break;
            
            case SLB_KBL_BRIGHTNESS_CURRENT: {
                    map<uint32_t,uint32_t> current = fetch();
                    
                    brightness = current[SLB_KBL_PROPERTY_BRIGHTNESS];
                }
            break;
        }
        
        buffer[0] = 0;
        buffer[1] = ITE8291R3_SET_EFFECT;
        buffer[2] = 0x02;
        buffer[3] = effect;
        buffer[4] = properties[SLB_KBL_PROPERTY_SPEED];
        buffer[5] = brightness;
        buffer[6] = properties[SLB_KBL_PROPERTY_COLOR];
        buffer[7] = properties[SLB_KBL_PROPERTY_DIRECTION] | properties[SLB_KBL_PROPERTY_REACTIVE];
        buffer[8] = properties[SLB_KBL_PROPERTY_SAVE];
        
        int res = ioctl(fd, HIDIOCSFEATURE(ITE8291R3_HID_REPORT_LENGTH + 1), buffer);
        
        if (res < 0) {
            cerr<<"Failed to set SET_EFFECT command"<<endl;
            return;
        }
        
        close(fd);
    }
    
}

void ITE8291R3::set_layout()
{
    uint8_t* buffer = new buffer[ITE8291R3_COLS * 3];
    
    for (int n=0;n<ITE8291R3_COLS*3;n+=3) {
        buffer[n] = 0x16;
        buffer[n+1] = 0xff;
        buffer[n+2] = n;
    }
    
    
}
