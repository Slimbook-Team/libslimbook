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

#include "configuration.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <filesystem>

#define DB_PATH "/var/lib/slimbook/"
#define DB_FILE DB_PATH"settings.db"

using namespace std;

Configuration::Configuration()
{

}


Configuration::~Configuration()
{

}

void Configuration::load()
{
    ifstream db;
    char buffer[256];
    
    db.open(DB_FILE);
    
    if (db.good()) {
    
        while (!db.eof()) {
            db.getline(buffer,256);
            string tmp = buffer;
            size_t sep = tmp.find(':');
            
            if (sep != std::string::npos) {
                string key = tmp.substr(0,sep);
                string value = tmp.substr(sep+1,tmp.size());
                m_data[key] = value;
            }
        }
        
        db.close();
    }
}

void Configuration::store()
{
    std::filesystem::create_directory(DB_PATH);
    
    ofstream db;
    
    db.open(DB_FILE);
    
    if (db.good()) {
        for (auto p : m_data) {
            db<<p.first<<":"<<p.second<<endl;
        }
        db.close();
    }
}

string Configuration::get(string key)
{
    return m_data[key];
}

uint32_t Configuration::get_u32(string key)
{
    uint32_t value = std::stoi(m_data[key],0,16);
    
    return value;
}

bool Configuration::find(std::string key, std::string& out)
{
    std::map<string,string>::iterator it;

    it = m_data.find(key);

    if (it != m_data.end()) {
        out = it->second;
        return true;
    }
    else {
        return false;
    }
}

bool Configuration::find_u32(std::string key, uint32_t& out)
{
    std::map<string,string>::iterator it;

    it = m_data.find(key);

    if (it != m_data.end()) {
        out = std::stoi(it->first,0,16);
        return true;
    }
    else {
        return false;
    }
}

void Configuration::set(string key, string value)
{
    m_data[key] = value;
}

void Configuration::set_u32(string key, uint32_t value)
{
    stringstream ss;
    
    ss<<std::setfill('0')<<std::setw(8)<<value;
    m_data[key] = ss.str();
}

