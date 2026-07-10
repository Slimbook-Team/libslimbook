/*
Copyright (C) 2026 Slimbook <dev@slimbook.es>

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
        m_layout = new uint8_t[ITE8291R3_ROWS * ITE8291R3_COLS * 3];
        clear_layout();
    }
}

ITE8291R3::ITE8291R3(string device) : m_device(device), m_ready(false)
{
    m_layout = new uint8_t[ITE8291R3_ROWS * ITE8291R3_COLS * 3];
    clear_layout();
}

ITE8291R3::~ITE8291R3()
{
    delete [] m_layout;
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
            case SLB_KBL_BRIGHTNESS_ZERO:
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

void ITE8291R3::set_brightness(uint32_t value)
{
    char buffer[64];

    int fd = open(m_device.c_str(), O_RDWR | O_NONBLOCK);

    if (fd > 0) {
        buffer[0] = 0;
        buffer[1] = ITE8291R3_SET_BRIGHTNESS;
        buffer[2] = 0x02;
        buffer[3] = value;
        buffer[4] = 0;
        buffer[5] = 0;
        buffer[6] = 0;
        buffer[7] = 0;
        buffer[8] = 0;

        int res = ioctl(fd, HIDIOCSFEATURE(ITE8291R3_HID_REPORT_LENGTH + 1), buffer);

        if (res < 0) {
            cerr<<"Failed to set SET_BRIGHTNESS command"<<endl;
            return;
        }

        close(fd);
    }

}

void ITE8291R3::set_layout()
{
    
    int fd = open(m_device.c_str(), O_RDWR | O_NONBLOCK);

    if (fd > 0) {

        int res;

        uint8_t* raw_data = new uint8_t[2 + (ITE8291R3_COLS * 3)];
        uint8_t* data = raw_data + 2;

        raw_data[0] = 0;
        raw_data[1] = 0;

        for (int r=0;r<ITE8291R3_ROWS;r++) {
            int moffset = r * ITE8291R3_COLS * 3;

            for (int n=0;n<ITE8291R3_COLS;n++) {
                data[n] = m_layout[moffset + n*3];
                data[n+ITE8291R3_COLS] = m_layout[moffset + n*3 + 1];
                data[n+ITE8291R3_COLS*2] = m_layout[moffset + n*3 + 2];
            }

            uint8_t buffer[16] = {0};

            buffer[0] = 0;
            buffer[1] = ITE8291R3_SET_ROW_INDEX;
            buffer[2] = 0;
            buffer[3] = r; //row

            res = ioctl(fd, HIDIOCSFEATURE(ITE8291R3_HID_REPORT_LENGTH + 1), buffer);
            if (res < 0) {
                cerr<<"Failed to set row index"<<endl;
            }

            res = ioctl(fd, HIDIOCSOUTPUT(2 + (ITE8291R3_COLS * 3)), raw_data);
            if (res < 0) {
                cerr<<"Failed to set row data"<<endl;
            }
        }

        delete [] raw_data;

        close(fd);
    }
}

void ITE8291R3::set_color(int x,int y,uint8_t r,uint8_t g,uint8_t b)
{
    if (x < 0 or y < 0 or x>= ITE8291R3_COLS or y>= ITE8291R3_ROWS) {
        cerr<<"Out of bounds"<<endl;
        return;
    }
    
    int offset = y * ITE8291R3_COLS * 3;
    m_layout[offset + x*3 + 0] = b;
    m_layout[offset + x*3 + 1] = g;
    m_layout[offset + x*3 + 2] = r;
}

void ITE8291R3::clear_layout()
{
    fill_layout(0,0,0);
}

void ITE8291R3::fill_layout(uint8_t r, uint8_t g, uint8_t b)
{
    for (int n=0;n<ITE8291R3_ROWS * ITE8291R3_COLS * 3;n+=3) {
        m_layout[n] = b;
        m_layout[n+1] = g;
        m_layout[n+2] = r;
    }

}

void ITE8291R3::shift_layout(int dx, int dy)
{
    uint8_t* target = new uint8_t[ITE8291R3_ROWS * ITE8291R3_COLS * 3];
    
    for (int i=0;i<ITE8291R3_COLS;i++) {
        for (int j=0;j<ITE8291R3_ROWS;j++) {
            
            int ii = (i + dx) % ITE8291R3_COLS;
            int jj = (j + dy) % ITE8291R3_ROWS;
            
            int soffset = i*3 + (j * ITE8291R3_COLS * 3);
            int toffset = ii*3 + (jj * ITE8291R3_COLS * 3);
            
            target[toffset + 0] = m_layout[soffset + 0];
            target[toffset + 1] = m_layout[soffset + 1];
            target[toffset + 2] = m_layout[soffset + 2];
        }
    }
    
    delete [] m_layout;
    
    m_layout = target;
}
