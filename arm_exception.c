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
#include "arm_exception.h"
#include "arm_constants.h"
#include "arm_core.h"
#include "util.h"

// Not supported below ARMv6, should read as 0
#define CP15_reg1_EEbit 0

#define Exception_bit_9 (CP15_reg1_EEbit << 9)

void arm_exception(arm_core p, unsigned char exception) {
    /* We only support RESET initially */
    /* Semantics of reset interrupt (ARM manual A2-18) */
    uint32_t cpsr_value = arm_read_cpsr(p);
    uint32_t spsr = cpsr_value;
    uint8_t pc_offset = 0;
    uint8_t pc_value = 0;
    cpsr_value = clr_bit(cpsr_value, 5);
    cpsr_value = set_bit(cpsr_value, 7);
    cpsr_value = clr_bit(cpsr_value, 9);
    switch (exception)
    {
        case RESET:
            cpsr_value = set_bits(cpsr_value, 4 , 0, SVC);
            cpsr_value = set_bit(cpsr_value, 6);
            cpsr_value = set_bit(cpsr_value, 8);
            break;
        case UNDEFINED_INSTRUCTION:
            cpsr_value = set_bits(cpsr_value, 4 , 0, UND);
            pc_value = 4;
            break;
        case SOFTWARE_INTERRUPT:
            cpsr_value = set_bits(cpsr_value, 4 , 0, SVC);
            pc_value = 8;
            break;
        case PREFETCH_ABORT:
            cpsr_value = set_bits(cpsr_value, 4 , 0, ABT);
            cpsr_value = set_bit(cpsr_value, 8);
            pc_offset = 4;
            pc_value = 12;
            break;
        case DATA_ABORT:
            cpsr_value = set_bits(cpsr_value, 4 , 0, ABT);
            cpsr_value = set_bit(cpsr_value, 8);
            pc_offset = 8;
            pc_value = 16;
            break;
        case INTERRUPT:
            cpsr_value = set_bits(cpsr_value, 4 , 0, IRQ);
            cpsr_value = set_bit(cpsr_value, 8);
            pc_offset = 4;
            pc_value = 24;
            break;
        case FAST_INTERRUPT:
            cpsr_value = set_bits(cpsr_value, 4 , 0, FIQ);
            cpsr_value = set_bit(cpsr_value, 6);
            cpsr_value = set_bit(cpsr_value, 8);
            pc_offset = 4;
            pc_value = 28;
            break;
    }
    if(exception != RESET){
        arm_write_cpsr(p, cpsr_value);
        arm_write_spsr(p, spsr);
        arm_write_register(p, 14, arm_read_register(p, 15)+pc_offset);
    }
    arm_write_usr_register(p, 15, pc_value);
}
