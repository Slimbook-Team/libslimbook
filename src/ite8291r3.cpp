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

#include "ite8291r3.h"

#include <filesystem>
#include <regex>

using namespace std;

vector<string> ITE8291R3::list()
{
    vector<string> devices;
    const std::regex hidraw_regex("hidraw[0-9]+");
    for (auto const& node : std::filesystem::directory_iterator{"/dev/"}) {
        if (!node.is_directory()) {
            string parent = node.path().filename();
            if (std::regex_match(parent,hidraw_regex)) {
                devices.push_back(node.path());
            }
        }
    }
    
    return devices;
}
