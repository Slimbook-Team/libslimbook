/*
 Copyright (C) 2025 Slimbook <dev@slimbook.es>

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

#include "common.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>

using namespace std;

bool find_file(string path, string file, string& out) {
    out = "";

    for (const auto &entryRecDir : filesystem::recursive_directory_iterator(path)) {
        string entryPath = entryRecDir.path().string();
        if (entryPath.find(file) != string::npos) {
            out = entryPath.substr(0, entryPath.size() - file.size());
            return true;
        }
    }

    return false;
}

void read_device(string path, string &out) {
    ifstream file;

    file.open(path.c_str());
    std::getline(file, out);
    file.close();
}

void write_device(string path, string in) {
    ofstream file;

    file.open(path.c_str());
    file << in;
    file.close();
}

vector<string> get_modules()
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
