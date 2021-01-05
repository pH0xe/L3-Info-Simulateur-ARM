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
    uint32_t* data;
};

memory memory_create(size_t size, int is_big_endian) {
    memory mem = malloc(sizeof(memory));

    // size = nb octet
    mem->size = size;

    // Valeur a stocker : 11 22 33 44
    // Si Big endian : 44 22 33 11
    // Si Little endian : 11 22 33 44
    mem->is_big_endian = is_big_endian;

    // 1 indice = 4 octet
    // exemple taille de 8 == [[11 22 33 44],[11 22 33 44]]
    // si size%4 != 0 ajout de 1 indice
    int oneMore = size%4 > 0 ? 1 : 0;
    mem->data = malloc(sizeof(uint32_t) * (size/4 + oneMore));

    return mem;
}

size_t memory_get_size(memory mem) {
    return mem->size;
}

void memory_destroy(memory mem) {
    free(mem->data);
    free(mem);
}

int memory_read_byte(memory mem, uint32_t address, uint8_t *value) {
    if (address >= memory_get_size(mem)){
        return FAILURE;
    }

    uint32_t word = mem->data[address / 4];

    int addr = address%4;
    int bitDeb = addr*8, bitFin = addr*8+7;
    *value = get_bits(word, bitFin, bitDeb);

    return SUCCESS;
}

int memory_read_half(memory mem, uint32_t address, uint16_t *value) {
    if (address >= memory_get_size(mem) || address % 2 != 0) {
        return FAILURE;
    }

    uint32_t word = mem->data[address / 4];
    int addr = address%4;
    int bitDeb = addr*8, bitFin = addr*8+15;

    *value = get_bits(word, bitFin, bitDeb);

    if (mem->is_big_endian) {
        *value = reverse_2(*value);
    }

    return SUCCESS;
}

int memory_read_word(memory mem, uint32_t address, uint32_t *value) {
    if (address >= memory_get_size(mem) || address % 4 != 0){
        return FAILURE;
    }

    *value = mem->data[address / 4];

    if (mem->is_big_endian) {
        *value = reverse_4(*value);
    }

    return SUCCESS;
}

int memory_write_byte(memory mem, uint32_t address, uint8_t value) {
    if (address >= memory_get_size(mem)) {
        return FAILURE;
    }

    uint32_t* addrWord = &mem->data[address / 4];
    int addr = address%4;
    int bitDeb = addr*8, bitFin = addr*8+7;
    if (bitFin == 31) {
        bitFin = 30;
        *addrWord = clr_bit(*addrWord, 31);
    }

   *addrWord = set_bits(*addrWord, bitFin, bitDeb, value);

    return SUCCESS;
}

int memory_write_half(memory mem, uint32_t address, uint16_t value) {
    if (address >= memory_get_size(mem) || address % 2 != 0) {
        return FAILURE;
    }

    if (mem->is_big_endian) {
        value = reverse_2(value);
    }

    uint32_t* addrWord = &mem->data[address / 4];
    int addr = address%4;
    int bitDeb = addr*8, bitFin = addr*8+15;

    if (bitFin == 31) {
        bitFin = 30;
        *addrWord = clr_bit(*addrWord, 31);
    }

    *addrWord = set_bits(*addrWord, bitFin, bitDeb, value);

    return SUCCESS;
}

int memory_write_word(memory mem, uint32_t address, uint32_t value) {
    if (address >= memory_get_size(mem) || address % 4 != 0) {
        return FAILURE;
    }

    if (mem->is_big_endian) {
        value = reverse_4(value);
    }

    mem->data[address / 4] = value;

    return SUCCESS;
}

/*
data = [
[0 - 3]
[4 - 7]
[8 - 11]
[12 - 15]
[16 - 19]
[20 - 23] //
[24 - 27]
[28 - 31]
[32 - 35]
[36- 39]
        ]
*/