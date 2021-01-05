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
#include "arm_data_processing.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "arm_branch_other.h"
#include "util.h"
#include "debug.h"




uint32_t logical_and(uint32_t val_rn, uint32_t shifter_operand){ return val_rn & shifter_operand;}

uint32_t logical_eor(uint32_t val_rn, uint32_t shifter_operand){ return val_rn ^ shifter_operand;}

uint32_t sub(uint32_t val_rn, uint32_t shifter_operand){ return val_rn + (~shifter_operand+1);}

uint32_t rsb(uint32_t val_rn, uint32_t shifter_operand){ return shifter_operand + (~val_rn+1);}

uint32_t add(uint32_t val_rn, uint32_t shifter_operand){ return val_rn + shifter_operand;}

uint32_t logical_or(uint32_t val_rn, uint32_t shifter_operand){ return val_rn | shifter_operand;}

uint32_t mov(uint32_t val_rn, uint32_t shifter_operand){ return shifter_operand;}

uint32_t bic(uint32_t val_rn, uint32_t shifter_operand){ return val_rn & ~shifter_operand;}

int ZNCV_update(arm_core p, int* oVerflow, int* Carry, uint32_t value, bool V, bool C){
	uint32_t cpsr_value = arm_read_cpsr(p);
	if (value == 0){
		cpsr_value = set_bit(cpsr_value,Z);
	}
	else cpsr_value = clr_bit(cpsr_value,Z);
	if (get_bit(value,31)){
		cpsr_value = set_bit(cpsr_value,N);
	}
	else cpsr_value = clr_bit(cpsr_value,N);
	if (Carry && C){
		cpsr_value = set_bit(cpsr_value,C);
	}
	else if(C){
		cpsr_value = clr_bit(cpsr_value,C);
	}
	if(oVerflow && V){
		cpsr_value = set_bit(cpsr_value,V);
	}
	else if(V){
		cpsr_value = clr_bit(cpsr_value,V);
	}
	arm_write_cpsr(p,cpsr_value);
}

uint32_t data_processing_operand(arm_core p, uint32_t ins, uint32_t (*operateur)(uint32_t, uint32_t), int* oVerflow, int* Carry){
	uint32_t value, shifter_operand;
	uint8_t rn, rm, rs, shift_value, champ;
	champ = get_bits(ins,6,4);
	rn = get_bits(ins,19,16);
	rm = get_bits(ins,3,0);
	//printf("r0: %x\n", arm_read_register(p,0));
	switch(champ){
		case 0:		//LSL imm
			shift_value = get_bits(ins,11,7);
			shifter_operand = (arm_read_register(p,rm) << shift_value);
			value = operateur(arm_read_register(p,rn), shifter_operand); 
			
			break;
		case 1:		//LSL reg
			rs = get_bits(ins,11,8);
			shifter_operand = (arm_read_register(p,rm) << arm_read_register(p,rs));
			value = operateur(arm_read_register(p,rn),shifter_operand);
			
			break;
		case 2:		//LSR imm
			shift_value = get_bits(ins,11,7);
			shifter_operand = (arm_read_register(p,rm) >> shift_value); 
			value = operateur(arm_read_register(p,rn), shifter_operand); 
			
			break;
		case 3:		//LSR reg
			rs = get_bits(ins,11,8);
			shifter_operand = (arm_read_register(p,rm) >> arm_read_register(p,rs));
			value = operateur(arm_read_register(p,rn), shifter_operand);
			
			break;
		case 4:		//ASR imm
			shift_value = get_bits(ins,11,7);
			shifter_operand = asr(arm_read_register(p,rm), shift_value);
			value = operateur(arm_read_register(p,rn), shifter_operand);
			
			break;
		case 5:		//ASR reg
			rs = get_bits(ins,11,8);
			shifter_operand = asr(arm_read_register(p,rm), arm_read_register(p,rs));
			value = operateur(arm_read_register(p,rn), shifter_operand);
			
			break;
		case 6:		//ROR imm
			shift_value = get_bits(ins,11,7);
			shifter_operand = ror(arm_read_register(p,rm), shift_value);
			value = operateur(arm_read_register(p,rn), shifter_operand);
			
			break;
		case 7:		//ROR reg
			rs = get_bits(ins,11,8);
			shifter_operand = ror(arm_read_register(p,rm), arm_read_register(p,rs));
			value = operateur(arm_read_register(p,rn), shifter_operand);
			
			break;
		default:	//ERROR
			return UNDEFINED_INSTRUCTION;
			break;
	}
	if ((get_bit(shifter_operand,31) == get_bit(arm_read_register(p,rn), 31)) && (get_bit(value,31) != get_bit(shifter_operand,31))){
		*oVerflow = 1;
	}
	else *oVerflow = 0;
	if(shifter_operand > value || arm_read_register(p,rn) > value){
		*Carry = 1;
	}
	else *Carry=0;
	//printf("r0: %x\n", arm_read_register(p,0));
	return value;
} 


/* Decoding functions for different classes of instructions */
int arm_data_processing_shift(arm_core p, uint32_t ins) {
	uint8_t rd = get_bits(ins,15,12);
	uint32_t value, cpsr_value;
	uint8_t opcode = get_bits(ins,24,21);	//Opcode - bits 21 à 24
	uint8_t s = get_bit(ins,20);			//Bit shift
	int oVerflow,Carry;

	switch(opcode){
		case 0:		//AND
			value = data_processing_operand(p, ins, logical_and, &oVerflow, &Carry);
			arm_write_register(p, rd, value);
			if(s){
				if(rd != 15){
					ZNCV_update(p, oVerflow, 0, value, 0, 1);
				}
				else{
					if (arm_current_mode_has_spsr(p)){
						arm_write_cpsr(p, arm_read_spsr(p));
					}
				}
			}
			return 0;
			break;
		case 1:		//EOR
			value = data_processing_operand(p, ins, logical_eor, &oVerflow, &Carry);
			arm_write_register(p, rd, value);
			if(s){
				if(rd != 15){
					ZNCV_update(p, oVerflow, 0, value, 0, 1);
				}
				else{
					if (arm_current_mode_has_spsr(p)){
						arm_write_cpsr(p, arm_read_spsr(p));
					}
				}
			}
			return 0;
			break;
		case 2:		//SUB
			value = data_processing_operand(p, ins, sub, &oVerflow, &Carry);
			arm_write_register(p, rd, value);
			if(s){
				if(rd != 15){
					ZNCV_update(p, oVerflow, Carry, value, 1, 1);
				}
				else{
					if (arm_current_mode_has_spsr(p)){
						arm_write_cpsr(p, arm_read_spsr(p));
					}
				}
			}
			return 0;
			break;
		case 3:		//RSB
			value = data_processing_operand(p, ins, rsb, &oVerflow, &Carry);
			arm_write_register(p, rd, value);
			if(s){
				if(rd != 15){
					ZNCV_update(p, oVerflow, Carry, value, 1, 1);
				}
				else{
					if (arm_current_mode_has_spsr(p)){
						arm_write_cpsr(p, arm_read_spsr(p));
					}
				}
			}
			return 0;
			break;
		case 4:		//ADD
			value = data_processing_operand(p, ins, add, &oVerflow, &Carry);
			arm_write_register(p, rd, value);
			if(s){
				if(rd != 15){
					ZNCV_update(p, oVerflow, Carry, value, 1, 1);
				}
				else{
					if (arm_current_mode_has_spsr(p)){
						arm_write_cpsr(p, arm_read_spsr(p));
					}
				}
			}
			return 0;
			break;
		case 5:		//ADC
			value = data_processing_operand(p, ins, add, &oVerflow, &Carry);
			value = value + get_bit(arm_read_cpsr(p),C);
			arm_write_register(p, rd, value);
			if(s){
				if(rd != 15){
					ZNCV_update(p, oVerflow, Carry, value, 1, 1);
				}
				else{
					if (arm_current_mode_has_spsr(p)){
						arm_write_cpsr(p, arm_read_spsr(p));
					}
				}
			}
			return 0;
			break;
		case 6:		//SBC
			value = data_processing_operand(p, ins, sub, &oVerflow, &Carry);
			value = value - ~get_bit(arm_read_cpsr(p),C);
			arm_write_register(p, rd, value);
			if(s){
				if(rd != 15){
					ZNCV_update(p, oVerflow, Carry, value, 1, 1);
				}
				else{
					if (arm_current_mode_has_spsr(p)){
						arm_write_cpsr(p, arm_read_spsr(p));
					}
				}
			}
			return 0;
			break;
		case 7:		//RSC
			value = data_processing_operand(p, ins, rsb, &oVerflow, &Carry);
			value = value - ~get_bit(arm_read_cpsr(p),C);
			arm_write_register(p, rd, value);
			if(s){
				if(rd != 15){
					ZNCV_update(p, oVerflow, Carry, value, 1, 1);
				}
				else{
					if (arm_current_mode_has_spsr(p)){
						arm_write_cpsr(p, arm_read_spsr(p));
					}
				}
			}
			return 0;
			break;
		case 8:		//TST
			value = data_processing_operand(p, ins, logical_and, &oVerflow, &Carry);
			ZNCV_update(p, oVerflow, Carry, value, 0, 1);
			return 0;
			break;
		case 9:		//TEQ
			value = data_processing_operand(p, ins, logical_eor, &oVerflow, &Carry);
			ZNCV_update(p, oVerflow, Carry, value, 0, 1);
			return 0;
			break;
		case 10:	//CMP
			value = data_processing_operand(p, ins, sub, &oVerflow, &Carry);
			Carry == 1 ? 0 : 1;
			ZNCV_update(p, oVerflow, Carry, value, 1, 1);
			return 0;
			break;
		case 11:	//CMN
			value = data_processing_operand(p, ins, add, &oVerflow, &Carry);
			ZNCV_update(p, oVerflow, Carry, value, 1, 1);
			return 0;
			break;
		case 12:	//ORR
			value = data_processing_operand(p, ins, logical_or, &oVerflow, &Carry);
			arm_write_register(p, rd, value);
			if(s){
				if(rd != 15){
					ZNCV_update(p, oVerflow, Carry, value, 0, 1);
				}
				else{
					if (arm_current_mode_has_spsr(p)){
						arm_write_cpsr(p, arm_read_spsr(p));
					}
				}
			}
			return 0;
			break;
		case 13:	//MOV
			value = data_processing_operand(p, ins, mov, &oVerflow, &Carry);
			arm_write_register(p, rd, value);
			if(s){
				if(rd != 15){
					ZNCV_update(p, oVerflow, Carry, value, 0, 1);
				}
				else{
					if (arm_current_mode_has_spsr(p)){
						arm_write_cpsr(p, arm_read_spsr(p));
					}
				}
			}
			return 0;
			break;
		case 14:	//BIC
			value = data_processing_operand(p, ins, bic, &oVerflow, &Carry);
			arm_write_register(p, rd, value);
			if(s){
				if(rd != 15){
					ZNCV_update(p, oVerflow, Carry, value, 0, 1);
				}
				else{
					if (arm_current_mode_has_spsr(p)){
						arm_write_cpsr(p, arm_read_spsr(p));
					}
				}
			}
			return 0;
			break;
		case 15:	//MVN
			value = data_processing_operand(p, ins, mov, &oVerflow, &Carry);
			arm_write_register(p, rd, ~value);
			if(s){
				if(rd != 15){
					ZNCV_update(p, oVerflow, Carry, value, 0, 1);
				}
				else{
					if (arm_current_mode_has_spsr(p)){
						arm_write_cpsr(p, arm_read_spsr(p));
					}
				}
			}
			return 0;
			break;
		default:	//ERROR
			return UNDEFINED_INSTRUCTION;
			break;
	}
    return UNDEFINED_INSTRUCTION;
}

int arm_data_processing_immediate_msr(arm_core p, uint32_t ins) {
    return UNDEFINED_INSTRUCTION;
}
