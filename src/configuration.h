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

#include <string>
#include <map>
#include <fstream>
#include <cstdint>

class Configuration
{
    protected:
    
    std::map<std::string,std::string> m_data;
    
    public:
    
    Configuration();
    virtual ~Configuration();
    
    void load();
    void store();
    
    std::string get(std::string key);
    uint32_t get_u32(std::string key);
    bool find(std::string key, std::string& out);
    bool find_u32(std::string key, uint32_t& out);
    
    void set(std::string key, std::string value);
    
    void set_u32(std::string key, uint32_t value);

    std::map<std::string,std::string> data() const
    {
        return m_data;
    }
};
