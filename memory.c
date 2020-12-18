/*
Armator - simulateur de jeu d'instruction ARMv5T � but p�dagogique
Copyright (C) 2011 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique G�n�rale GNU publi�e par la Free Software
Foundation (version 2 ou bien toute autre version ult�rieure choisie par vous).

Ce programme est distribu� car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but sp�cifique. Reportez-vous � la
Licence Publique G�n�rale GNU pour plus de d�tails.

Vous devez avoir re�u une copie de la Licence Publique G�n�rale GNU en m�me
temps que ce programme ; si ce n'est pas le cas, �crivez � la Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
�tats-Unis.

Contact: Guillaume.Huard@imag.fr
	 B�timent IMAG
	 700 avenue centrale, domaine universitaire
	 38401 Saint Martin d'H�res
*/
#include <stdlib.h>
#include "memory.h"
#include "util.h"
#include <stdio.h>

struct memory_data {
    size_t size;
    int is_big_endian;
    uint32_t data;
};

memory memory_create(size_t size, int is_big_endian) {
    memory mem = malloc(sizeof(memory));
    mem->size = size;
    mem->is_big_endian = is_big_endian;
    return mem;
}

size_t memory_get_size(memory mem) {
    return mem->size;
}

void memory_destroy(memory mem) {
    free(mem);
}

int memory_read_byte(memory mem, uint32_t address, uint8_t *value) {
    uint32_t val = mem->data;
    if (mem->is_big_endian) {
        val = reverse_4(val);
    }

    uint8_t byte = get_bits(val, address*8 + 8, address*8);
    *value = byte;
    return 0;
}

int memory_read_half(memory mem, uint32_t address, uint16_t *value) {
    uint32_t val = mem->data;
    if (mem->is_big_endian) {
        val = reverse_4(val);
    }

    uint16_t half = get_bits(val, address*8 + 16, address*8);
    if (mem->is_big_endian) {
        half = reverse_2(half);
    }
    *value = half;
    return 0;
}

int memory_read_word(memory mem, uint32_t address, uint32_t *value) {
    uint32_t val = mem->data;
    if (mem->is_big_endian) {
        val = reverse_4(val);
    }

    uint32_t word = get_bits(val, address + 30, 0);
    if (mem->is_big_endian) {
        word = reverse_4(word);
    }
    *value = word;
    return 0;
}

int memory_write_byte(memory mem, uint32_t address, uint8_t value) {
    uint32_t val = mem->data;

    if (mem->is_big_endian) {
        val = reverse_4(val);
    }
    val = set_bits(val, address * 8 + 8, address * 8, value);

    if (mem->is_big_endian) {
        val = reverse_4(val);
    }
    mem->data = val;

    return 0;
}

int memory_write_half(memory mem, uint32_t address, uint16_t value) {
    uint32_t val = mem->data;
    if (mem->is_big_endian) {
        val = reverse_4(val);
        value = reverse_2(value);
    }
    val = set_bits(val, address*8 + 15, address*8, value);

    if (mem->is_big_endian) {
        val = reverse_4(val);
    }

    mem->data = val;

    return 0;
}

int memory_write_word(memory mem, uint32_t address, uint32_t value) {
    uint32_t val = mem->data;
    if (mem->is_big_endian) {
        val = reverse_4(val);
    }
    val = set_bits(val, address + 30 , address*8, value);

    mem->data = val;

    return 0;
}
