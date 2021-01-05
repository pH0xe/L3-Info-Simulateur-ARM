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

struct registers_data {
    uint8_t mode;
    uint8_t name;
    uint32_t **data;
};

registers registers_create() {
    registers r = malloc(sizeof(registers));
    r->mode=0;
    r->name=0;
    r->data=malloc(16*sizeof(uint32_t*));
    for (int i=0;i<16;i++){
        r->data[i]=calloc(18,sizeof(uint32_t));
    }
    return r;
}

void registers_destroy(registers r) {
    for(int i=0;i<16;i++){
        free(r->data[i]);
    }
    free(r->data);
    free(r);
}

uint8_t get_mode(registers r) {
    return r->mode;
} 

int current_mode_has_spsr(registers r) {
    char* res=arm_get_register_name(r->name);
    return strcmp(res,"SPSR")==0;
     
}

int in_a_privileged_mode(registers r) {
    uint8_t res = get_mode(r);
    return res!=USR;
}

uint32_t read_register(registers r, uint8_t reg) {
    uint32_t value=r->data[get_mode(r)][reg];
    return value;
}

uint32_t read_usr_register(registers r, uint8_t reg) {
    uint32_t value=r->data[get_mode(r)][reg];
    
    return value;
}

uint32_t read_cpsr(registers r) {
    uint32_t value=r->data[get_mode(r)][CPSR];
    return value;
}

uint32_t read_spsr(registers r) {
    uint32_t value=r->data[get_mode(r)][SPSR];
    return value;
}

void write_register(registers r, uint8_t reg, uint32_t value) {
    uint32_t mask = ~(0xFFFFFFFF);
    r->data[get_mode(r)][reg]&= mask;
    r->data[get_mode(r)][reg]|= value;
}

void write_usr_register(registers r, uint8_t reg, uint32_t value) {
    uint32_t mask = ~(0xFFFFFFFF);
    r->data[get_mode(r)][reg]&= mask;
    r->data[get_mode(r)][reg]|= value;
}

void write_cpsr(registers r, uint32_t value) {
    uint32_t mask = ~(0xFFFFFFFF);
    r->data[get_mode(r)][CPSR]&= mask;
    r->data[get_mode(r)][CPSR]|= value;
}

void write_spsr(registers r, uint32_t value) {
    uint32_t mask = ~(0xFFFFFFFF);
    r->data[get_mode(r)][SPSR]&= mask;
    r->data[get_mode(r)][SPSR]|= value;
}
