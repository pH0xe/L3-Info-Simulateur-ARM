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
#include "registers.h"
#include "arm_constants.h"
#include <stdlib.h>
#include "trace.h"
#include <string.h>
#include "util.h"

/*
 * Mode = b4-0 -> cpsr
 * USR -> 0x10
 * FIQ -> 0x11
 * IRQ -> 0x12
 * SVC -> 0x13
 * ABT -> 0x17
 * UND -> 0x1b
 * SYS -> 0x1f
 */

struct registers_data {
    uint32_t *data;
};

uint8_t getRegister(registers r, uint8_t reg) {
    uint8_t mode = get_mode(r);
    if (mode == USR || mode == SYS) return reg;

    if (reg < 8 || reg == 15 || reg == 16) return reg;

    if (mode!= FIQ && reg < 13) return reg;

    if (reg < 13) return reg + 10;

    switch (mode) {
        case SVC:
            if (reg == 13) return R13_SVC;
            if (reg == 14) return R14_SVC;
            return SPSR_SVC;
        case ABT:
            if (reg == 13) return R13_ABT;
            if (reg == 14) return R14_ABT;
            return SPSR_ABT;
        case UND:
            if (reg == 13) return R13_UND;
            if (reg == 14) return R14_UND;
            return SPSR_UND;
        case IRQ:
            if (reg == 13) return R13_IRQ;
            if (reg == 14) return R14_IRQ;
            return SPSR_IRQ;
        case FIQ:
            if (reg == 13) return R13_FIQ;
            if (reg == 14) return R14_FIQ;
            return SPSR_FIQ;
    }
    return -1;
}

registers registers_create() {
    registers r = malloc(sizeof(registers));
    if(r == NULL) return NULL;

    r->data=calloc(sizeof(uint32_t), 38);
    if(r->data == NULL) return NULL;

    // CPSR
    uint32_t cpsr = read_cpsr(r);
    cpsr = set_bits(cpsr, 4, 0, SYS);
    cpsr = clr_bit(cpsr, 7);
    cpsr = clr_bit(cpsr, 6);
    write_cpsr(r, cpsr);

    return r;
}

void registers_destroy(registers r) {
    free(r->data);
    free(r);
}

uint8_t get_mode(registers r) {
    return get_bits(read_cpsr(r), 4, 0);
}

int current_mode_has_spsr(registers r) {
    return !(get_mode(r) == USR || get_mode(r) == SYS);
}

int in_a_privileged_mode(registers r) {
    return get_mode(r)!=USR;
}

uint32_t read_register(registers r, uint8_t reg) {
    return r->data[getRegister(r, reg)];
}

uint32_t read_usr_register(registers r, uint8_t reg) {
    return r->data[reg];
}

uint32_t read_cpsr(registers r) {
    return r->data[CPSR];
}

uint32_t read_spsr(registers r) {
    if (current_mode_has_spsr(r)) return r->data[getRegister(r , SPSR)];
    return -1;
}

void write_register(registers r, uint8_t reg, uint32_t value) {
    uint32_t mask = ~(0xFFFFFFFF);
    uint8_t rg = getRegister(r, reg);
    r->data[rg] &= mask;
    r->data[rg] |= value;
}

void write_usr_register(registers r, uint8_t reg, uint32_t value) {
    if(get_mode(r) == USR){
        uint32_t mask = ~(0xFFFFFFFF);
        r->data[reg] &= mask;
        r->data[reg] |= value;
    }
}

void write_cpsr(registers r, uint32_t value) {
    uint32_t mask = ~(0xFFFFFFFF);
    r->data[CPSR] &= mask;
    r->data[CPSR] |= value;
}

void write_spsr(registers r, uint32_t value) {
    if (current_mode_has_spsr(r)){
        uint32_t mask = ~(0xFFFFFFFF);
        uint8_t rg = getRegister(r, SPSR);
        r->data[rg] &= mask;
        r->data[rg] |= value;
    }
}
