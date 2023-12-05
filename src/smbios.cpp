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

#include <vector>
#include <fstream>
#include <cstring>
#include <iostream>

using namespace std;

int slb_smbios_get(slb_smbios_entry_t** entries,int* count)
{
    vector<slb_smbios_entry_t> data;

    ifstream file;

    try {
        file.open("/sys/firmware/dmi/tables/DMI",std::ifstream::binary);
        while (file.good()) {
            slb_smbios_entry_t entry;
            streampos start = file.tellg();

            file.read((char*)&entry.type,1);
            file.read((char*)&entry.length,1);
            file.read((char*)&entry.handle,2);

            file.seekg(start);

            uint8_t raw[256];

            file.read((char*)raw,entry.length);

            vector<string> strings;
            string current;
            bool end = false;

            current.clear();

            do {
                char tmp;
                file.read(&tmp,1);
                if (tmp == 0) {
                    if (end) {
                        break;
                    }
                    else {
                        if (current.size() > 0) {
                            strings.push_back(current);
                            current.clear();
                        }

                        end = true;
                    }
                }
                else {
                    end = false;
                    current = current + tmp;
                }
            } while (true);

            if (entry.type == 4) {
                entry.data.processor.cores = raw[0x23];
                string name = strings[raw[0x10]-1];
                strncpy(entry.data.processor.version,name.c_str(),SLB_MAX_PROCESSOR_VERSION - 1);
            }

            if (entry.type == 17) {

                entry.data.memory_device.size = *((uint16_t*)(&raw[0x0c]));
                entry.data.memory_device.speed = *((uint16_t*)(&raw[0x15]));
                entry.data.memory_device.type = raw[0x12];
            }

            data.push_back(entry);
        }

        file.close();
    }
    catch(...) {
        return EIO;
    }

    *entries = (slb_smbios_entry_t* ) malloc(sizeof(slb_smbios_entry_t) * data.size());
    *count = data.size();

    memcpy(*entries,data.data(),sizeof(slb_smbios_entry_t) * data.size());

    return 0;
}

int slb_smbios_free(slb_smbios_entry_t* entries)
{
    if (entries) {
        free(entries);
    }
    
    return 0;
}

