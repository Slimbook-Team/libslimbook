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
                entry.data.processor.cores = raw[0x23] == 0xFF ? *((uint16_t*)(&raw[0x23])) : raw[0x2A];
                entry.data.processor.threads = raw[0x25] == 0xFF ? *((uint16_t*)(&raw[0x2E])) : raw[0x25];
                string name = strings[raw[0x10]-1];
                strncpy(entry.data.processor.version,name.c_str(),SLB_MAX_PROCESSOR_VERSION - 1);
                //ensure string is 0 ended
                entry.data.processor.version[SLB_MAX_PROCESSOR_VERSION - 1] = 0;
            }

            if (entry.type == 17) {

                entry.data.memory_device.size = *((uint16_t*)(&raw[0x0C])) == 0x7FFF ?  *((uint32_t*)(&raw[0x1C])) : *((uint16_t*)(&raw[0x0C])) & 0x7FFF;
                entry.data.memory_device.size_unit = *((uint16_t*)(&raw[0x0C])) == 0x7FFF ? 0 : (*((uint16_t*)(&raw[0x0C])) & 0x8000) != 0;
                entry.data.memory_device.speed = *((uint16_t*)(&raw[0x15])) == 0xFFFF ? *((uint32_t*)(&raw[0x54])) : *((uint16_t*)(&raw[0x15]));
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

