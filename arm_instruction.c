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
#include "arm_instruction.h"
#include "arm_exception.h"
#include "arm_data_processing.h"
#include "arm_load_store.h"
#include "arm_branch_other.h"
#include "arm_constants.h"
#include "util.h"

static int arm_execute_instruction(arm_core p) {
    uint32_t val;
    uint8_t champ;
    arm_fetch(p, &val);
    if (((val & 0xF0000000) >> 28) == 0xF){
        return -1;
    } 
    champ = (uint8_t)((val & 0x0E000000) >> 25);
    switch (champ){
        case 0:         //Data processing & load store
            if (get_bit(val, 4) == 1 && get_bit(val, 7) == 1) {
                return arm_load_store(p, val);
            }
            else if (((get_bits(val,24,23) == 2) && (get_bit(val,20) == 0)) && (get_bit(val,4) == 0 || (get_bit(val,7) == 0 && get_bit(val,4) == 1))){
                return arm_miscellaneous(p, val);
            }
            else {
                return arm_data_processing_shift(p, val);      
            }
        case 1:         //Data processing
            return arm_data_processing_immediate_msr(p, val);
        case 2:         //Load/Store
            return arm_load_store(p, val);
        case 3:         //Load/Store
            return arm_load_store(p, val);
        case 4:         //Load/Store
            return arm_load_store_multiple(p, val);
        case 5:         //Branches
            return arm_branch(p, val);
        case 6:         //Coprocessor Load/Store
            return arm_coprocessor_load_store(p, val);
        default:
            return -1;
    }
}

int arm_step(arm_core p) {
    int result;
    result = arm_execute_instruction(p);
    if (result)
        arm_exception(p, result);
    return result;
}