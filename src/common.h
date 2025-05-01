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

#ifndef SLB_COMMON_H
#define SLB_COMMON_H

#include <string>
#include <vector>
#include <cstdint>

#define ALIGN(data, alignto) ((data) & ~((alignto)-1))

#define cpuid(level, regs) do{ __cpuid((level), (regs)[0],(regs)[1],(regs)[2],(regs)[3]); }while(0)

/* Tries to find file recursively in path */
bool find_file(std::string path, std::string file, std::string& out);

/* Reads from the device's file */
void read_device(std::string path, std::string& out);

/* Writes to the device's file */
void write_device(std::string in, std::string out);

/* Retrieves all modules loaded */
std::vector<std::string> get_modules(void);

/* Swaps 16 bit data for endianness */
uint16_t swap16(uint16_t data);

/* Swaps 32 bit data for endianness */
uint32_t swap32(uint32_t data);

/* Retrieves endianness 0 LE 1 BE */
int32_t check_endianness(void);

#endif
