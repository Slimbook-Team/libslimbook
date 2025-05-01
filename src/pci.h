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

#ifndef SLB_PCI_H
#define SLB_PCI_H

#include <cstdint>
#include <string>
#include "stddef.h"

typedef struct pci_access pci_access;
typedef struct pci_dev pci_dev;

typedef void (*init_proc)(pci_access*);
typedef void (*read_proc)(pci_dev* d, int32_t pos, char* buf, size_t len);
typedef size_t (*write_proc)(pci_dev* d, int32_t pos, char* buf, size_t len);

typedef struct pci_procs {
    init_proc init;
    read_proc read;
    write_proc write;
} pci_procs;

struct pci_access {
    std::string name;
    uint32_t id;
    std::string path;
    int32_t fd;
    pci_procs* procs;
};

struct pci_dev {
    struct pci_access* access;
    int32_t dom;
    int32_t bus;
    int32_t dev;
    int32_t fun;
    pci_procs* procs; 
};

/* Allocates a pci_access struct */
pci_access* pci_access_alloc();

/* Gets a pci_dev from access, domain, dev, bus and func */
pci_dev* pci_get_dev(pci_access* a, int32_t domain, int32_t dev, int32_t bus, int32_t func);

/* Initialises the pci_access */
void pci_init_dev(pci_access* a);

/* Reads a byte from pci_dev at pos */
uint8_t pci_read_char(pci_dev* dev, int32_t pos);

/* Reads two bytes from pci_dev at pos */
uint16_t pci_read_short(pci_dev* dev, int32_t pos);

/* Reads four bytes from pci_dev at pos */
uint32_t pci_read_long(pci_dev* dev, int32_t pos);

/* Writes a byte from pci_dev at pos */
void pci_write_char(pci_dev* dev, int32_t pos, uint8_t data);

/* Writes two bytes from pci_dev at pos */
void pci_write_short(pci_dev* dev, int32_t pos, uint16_t data);

/* Writes four bytes from pci_dev at pos */
void pci_write_long(pci_dev* dev, int32_t pos, uint32_t data);

/* Frees the pci_dev */
void pci_cleanup(pci_dev* dev);

#endif
