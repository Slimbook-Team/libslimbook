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

#ifndef SLB_ITE8291R3_H
#define SLB_ITE8291R3_H

#define SLB_VENDOR_ID_ITE 0x048d
#define ITE8291R3_HID_REPORT_LENGTH 9

#define ITE8291R3_SET_EFFECT 8
#define ITE8291R3_GET_EFFECT 136

#define ITE8291R3_ROWS 6
#define ITE8291R3_COLS 21

#include <linux/hidraw.h>

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <cstdint>

class ITE8291R3
{
    public:
    
    static struct hidraw_devinfo get_info(std::string device);
    static std::vector<std::string> list();
    
    ITE8291R3();
    ITE8291R3(std::string device);
    
    virtual ~ITE8291R3();
    
    std::map<uint32_t,uint32_t> fetch();
    void set_effect(uint32_t effect, std::map<uint32_t,uint32_t> properties);
    
    void set_layout();
    
    const std::string device() const
    {
        return m_device;
    }
    
    private:
    
    uint8_t* m_layout;
    
    std::string m_device;
    bool m_ready;
};

#endif
