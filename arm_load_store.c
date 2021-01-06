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
#include "arm_load_store.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "util.h"
#include "debug.h"

/* Instructions prises en compte :
 * Simple :
 *      LDR - (A4-43)   27-26 = 1 && 22 = 0 && 20 = 1               - loads a word from a memory address
 *      LDRB - (A4-46)  27-26 = 1 && 22 = 1 && 20 = 1               - loads a byte from memory and zero-extends the byte to a 32-bit word
 *      LDRH - (A4-54)  27-25 = 0           && 20 = 1 && 7-4 = B    - loads a halfword from memory and zero-extends it to a 32-bit word.
 *      STR - (A4-193)  27-26 = 1 && 22 = 0 && 20 = 0
 *      STRB - (A4-195) 27-26 = 1 && 22 = 1 && 20 = 0
 *      STRH - (A4-204) 27-25 = 0           && 20 = 0 && 7-4 = B    - stores a halfword from the least significant halfword of a register to memory.
 * Multiple :
 *      LDM(1) - (A4-36) 27-25 = 4 && 22 = 0 && 20 = 1
 *      STM(1) - (A4-189) 27-25 = 4 && 22 = 0 && 20 = 0
 *
 */

int condition(arm_core p, uint32_t ins) {
    uint8_t cond = get_bits(ins,31,28);
    uint32_t cpsr_value=arm_read_cpsr(p);
    switch (cond){
        case 0:     //EQ
            if(get_bit(cpsr_value,Z)) return 1;
            break;
        case 1:     //NE
            if(!get_bit(cpsr_value,Z))return 1;
            break;
        case 2:     //CS/HS
            if(get_bit(cpsr_value,C)) return 1;
            break;
        case 3:     //CC/LO
            if(!get_bit(cpsr_value,C)) return 1;
            break;
        case 4:     //MI
            if(get_bit(cpsr_value,N)) return 1;
            break;
        case 5:     //PL
            if(!get_bit(cpsr_value,N)) return 1;
            break;
        case 6:     //VS
            if(get_bit(cpsr_value,V)) return 1;
            break;
        case 7:     //VC
            if(!get_bit(cpsr_value,V)) return 1;
            break;
        case 8:     //HI
            if(get_bit(cpsr_value,C) && !get_bit(cpsr_value,Z)) return 1;
            break;
        case 9:     //LS
            if(!get_bit(cpsr_value,C) || get_bit(cpsr_value,Z)) return 1;
            break;
        case 10:    //GE
            if(get_bit(cpsr_value,N) == get_bit(cpsr_value,V)) return 1;
            break;
        case 11:    //LT
            if(get_bit(cpsr_value,N) != get_bit(cpsr_value,V)) return 1;
            break;
        case 12:    //GT
            if((get_bit(cpsr_value,N) == get_bit(cpsr_value,V)) && !get_bit(cpsr_value,Z)) return 1;
            break;
        case 13:    //LE
            if((get_bit(cpsr_value,N) != get_bit(cpsr_value,V)) || get_bit(cpsr_value,Z)) return 1;
            break;
        case 14:    //AL
            return 1;
            break;
        default:    //ERROR
            return UNDEFINED_INSTRUCTION;
            break;
    }
    return UNDEFINED_INSTRUCTION;
}

uint32_t shift(arm_core p, uint32_t ins, uint32_t valRm) {
    int shift_type = get_bits(ins, 6,5);
    uint8_t shift_value = get_bits(ins, 11, 7);
    uint32_t index;

    switch (shift_type) {
        case 0:
            index = valRm << shift_value;
            break;
        case 1:
            if (shift_value == 0 ){
                index = 0;
            } else {
                index = valRm >> shift_value;
            }
            break;
        case 2:
            if (shift_value == 0) {
                if (get_bit(valRm, 31) == 1) {
                    index = 0xFFFFFFFF;
                } else {
                    index = 0;
                }
            } else {
                index = asr(valRm, shift_value);
            }
            break;
        case 3:
            if (shift_value == 0) {
                index = (get_bit(arm_read_cpsr(p), C) << 31) | (valRm >> 1);
            }
            else {
                index = ror(valRm, shift_value);
            }
            break;
        default:
            return -1;
    }
    return index;
}

uint32_t getAddressModeHalf(arm_core p, uint32_t ins) {
    int is_post = get_bit(ins, 24) == 0;
    uint32_t type = get_bits(ins, 22, 21);
    int u = get_bit(ins, 23);
    uint32_t rn = get_bits(ins, 19, 16);
    uint32_t valRn = arm_read_register(p, rn);

    uint8_t HOffset = get_bits(ins, 11, 8);
    uint8_t LOffset = get_bits(ins, 3, 0);

    if (is_post && type == 0) {
        // register post index
        uint32_t address = valRn;
        if (condition(p, ins) == 1) {
            uint32_t valRm = arm_read_register(p, LOffset);
            valRn = u == 1 ? valRn + valRm : valRn - valRm;
            arm_write_register(p, rn, valRn);
            return address;
        }
        return -1;
    } else if (is_post && type == 0x2) {
        // imm post indexed
        uint32_t address = valRn;
        if (condition(p, ins) == 1) {
            uint8_t offset_8 = (HOffset << 4) | LOffset;
            valRn = u == 1 ? valRn + offset_8 : valRn - offset_8;
            arm_write_register(p, rn, valRn);
            return address;
        }
        return -1;
    } else if (!is_post && type == 0){
        // register offset
        uint32_t valRm = arm_read_register(p, LOffset);
        return u == 1 ? valRn + valRm : valRn - valRm;
    } else if (!is_post && type == 0x1){
        // register pre indexed
        uint32_t valRm = arm_read_register(p, LOffset);
        uint32_t address = u == 1 ? valRn + valRm : valRn - valRm;

        if (condition(p, ins) == 1) {
            arm_write_register(p, rn, address);
            return address;
        } else return -1;
    } else if (!is_post && type == 0x2){
        // imm offset
        uint8_t offset_8 = (HOffset << 4) | LOffset;
        return u == 1 ? valRn + offset_8 : valRn - offset_8;
    } else if (!is_post && type == 0x3){
        // imm pre indexed
        uint8_t offset_8 = (HOffset << 4) | LOffset;
        uint32_t address = u == 1 ? valRn + offset_8 : valRn - offset_8;

        if (condition(p, ins) == 1) {
            arm_write_register(p, rn, address);
            return address;
        } else return -1;
    }
    return -1;
}

uint32_t getAddressModeBW(arm_core p, uint32_t ins) {
    int u = get_bit(ins, 23);
    uint32_t rn = get_bits(ins, 19, 16);
    uint32_t valRn = arm_read_register(p, rn);

    int is_imm = get_bit(ins, 25) == 0;

    if (is_imm) {
        uint16_t offset12 = get_bits(ins, 11, 0);
        if (get_bit(ins,21) == 0 && get_bit(ins, 24) == 1){
            //  Immediate offset
            if (u == 1) {
                return valRn + offset12;
            } else {
                return valRn - offset12;
            }
        } else if (get_bit(ins,21) == 1 && get_bit(ins, 24) == 1) {
            // immediate pre indexed
            uint32_t address;
            if (u == 1) {
                address = valRn + offset12;
            } else {
                address = valRn - offset12;
            }

            if (condition(p, ins) == 1) {
                arm_write_register(p, rn, address);
                return address;
            } else return -1;

        }else if (get_bit(ins,21) == 0 && get_bit(ins, 24) == 0) {
            // immediate post indexed
            if (condition(p, ins)) {
                if (u == 1){
                    return valRn + offset12;
                } else {
                    return valRn - offset12;
                }
            }
        }
        return -1;
    } else {
        if (get_bit(ins, 24) == 0) {
            // register post-indexed
            uint32_t valRm = arm_read_register(p, get_bits(ins, 3,0));
            uint32_t index = shift(p, ins, valRm);
            if (condition(p, ins)) {
                if (u == 1) {
                    return valRn + index;
                } else {
                    return valRn - index;
                }
            }
        } else if (get_bit(ins,21) == 1){
            // register pre-indexed
            uint32_t valRm = arm_read_register(p, get_bits(ins, 3,0));
            uint32_t index = shift(p, ins, valRm);
            uint32_t address;
            if (u == 1){
                 address = valRn + index;
            } else {
                address = valRn - index;
            }

            if (condition(p, ins) == 1) {
                arm_write_register(p, rn, address);
                return address;
            } else return -1;

        } else if (get_bit(ins,21) == 0){
            // register offset
            uint32_t valRm = arm_read_register(p, get_bits(ins, 3,0));
            uint32_t index = shift(p, ins, valRm);

            if (u == 1) {
                return valRn + index;
            } else {
                return valRn - index;
            }
        }
        return -1;
    }
}

int arm_load_store(arm_core p, uint32_t ins) {
    int is_load = get_bit(ins, 20) == 1;
    int is_H = get_bits(ins, 27, 25) == 0 && get_bits(ins, 7, 4) == 0xB;
    int is_B = get_bit(ins, 22) == 1;

    uint32_t reg_dest = get_bits(ins, 15, 12);

    if (is_load) {
        if (is_H){
            // LDRH
            uint32_t address = getAddressModeHalf(p, ins);
            if (address != -1 ){
                uint16_t value;
                arm_read_half(p, address, &value);
                arm_write_register(p, reg_dest, value);
            }
        } else if (is_B) {
            // LDRB
            uint32_t address = getAddressModeBW(p, ins);
            if (address != -1) {
                uint8_t value;
                arm_read_byte(p, address, &value);
                arm_write_register(p, reg_dest, value);
            }
        } else {
            // LDR
            uint32_t address = getAddressModeBW(p, ins);
            if (address != -1) {
                uint32_t value;
                arm_read_word(p, address, &value);
                arm_write_register(p, reg_dest, value);
            }
        }
    } else {
        if (is_H){
            // STRH
            uint32_t address = getAddressModeHalf(p, ins);
            uint16_t value = arm_read_register(p, reg_dest);
            arm_write_half(p,address, value);
        } else if (is_B) {
            // STRB
            uint32_t address = getAddressModeBW(p, ins);
            uint8_t value = arm_read_register(p, reg_dest);
            arm_write_byte(p,address, value);
        } else {
            // STR
            uint32_t address = getAddressModeBW(p, ins);
            uint32_t value = arm_read_register(p, reg_dest);
            arm_write_word(p,address, value);
        }
    }
    return 0;
}

int arm_load_store_multiple(arm_core p, uint32_t ins) {
    return UNDEFINED_INSTRUCTION;
}

int arm_coprocessor_load_store(arm_core p, uint32_t ins) {
    /* Not implemented */
    return UNDEFINED_INSTRUCTION;
}
