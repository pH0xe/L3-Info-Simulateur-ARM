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

void ZNCV_update(arm_core p, int* oVerflow, int* Carry, uint32_t value, int oV, int Ca){
	uint32_t cpsr_value = arm_read_cpsr(p);
	if (value == 0){
		cpsr_value = set_bit(cpsr_value,Z);
	}
	else cpsr_value = clr_bit(cpsr_value,Z);
	if (get_bit(value,31)){
		cpsr_value = set_bit(cpsr_value,N);
	}
	else cpsr_value = clr_bit(cpsr_value,N);
	if (Carry && Ca){
		cpsr_value = set_bit(cpsr_value,C);
	}
	else if(Ca){
		cpsr_value = clr_bit(cpsr_value,C);
	}
	if(oVerflow && oV){
		cpsr_value = set_bit(cpsr_value,V);
	}
	else if(oV){
		cpsr_value = clr_bit(cpsr_value,V);
	}
	arm_write_cpsr(p,cpsr_value);
}

uint32_t data_processing_immediate_operand(arm_core p, uint32_t ins,uint32_t (*operateur)(uint32_t, uint32_t), int* oVerflow, int* Carry, int* CarryFrom){
    uint32_t immediate = get_bits(ins,7,0);
    uint8_t rotate = get_bits(ins,11,8);
    uint8_t rn = get_bits(ins,19,16);
    uint32_t valRn = arm_read_register(p,rn);
    uint32_t shifter_operand = ror(immediate,(rotate*2));
	uint32_t value = operateur(valRn, shifter_operand);

	if (rotate == 0) *Carry = get_bit(arm_read_cpsr(p),C);
	else *Carry = get_bit(shifter_operand,31);

	if ((get_bit(shifter_operand,31) == get_bit(valRn, 31)) && (get_bit(value,31) != get_bit(shifter_operand,31))) *oVerflow = 1;
	else *oVerflow = 0;

	if (shifter_operand > value || valRn > value) *CarryFrom = 1;
	else *CarryFrom = 0;

	return value;
}

uint32_t data_processing_operand(arm_core p, uint32_t ins, uint32_t (*operateur)(uint32_t, uint32_t), int* oVerflow, int* Carry, int* CarryFrom){
	uint32_t value, shifter_operand, valRs, valRm, valRn;
	uint8_t rn, rm, rs, shift_value, champ, valRsb70;
	champ = get_bits(ins,6,4);
	rn = get_bits(ins,19,16);
	rm = get_bits(ins,3,0);
	valRm = arm_read_register(p,rm);

	switch(champ){
		case 0:		//LSL imm
			shift_value = get_bits(ins,11,7);
			if (shift_value == 0) *Carry = get_bit(arm_read_cpsr(p),C);
			else *Carry = get_bit(valRm, 32 - shift_value);
			shifter_operand = (valRm << shift_value);
			break;
		case 1:		//LSL reg
			rs = get_bits(ins,11,8);
			valRs = arm_read_register(p,rs);
			valRsb70 = get_bits(valRs, 7, 0);
			if (valRsb70 == 0) *Carry = get_bit(arm_read_cpsr(p),C);
			else if (valRsb70 <= 32) *Carry = get_bit(valRm, 32 - valRsb70);
			else *Carry = 0;
			shifter_operand = (valRm << valRs);
			break;
		case 2:		//LSR imm
			shift_value = get_bits(ins,11,7);
			if (shift_value == 0) *Carry = get_bit(valRm,31);
			else *Carry = get_bit(valRm, shift_value - 1);
			shifter_operand = (valRm >> shift_value);
			break;
		case 3:		//LSR reg
			rs = get_bits(ins,11,8);
			valRs = arm_read_register(p,rs);
			valRsb70 = get_bits(valRs, 7, 0);
			if (valRsb70 == 0) *Carry = get_bit(arm_read_cpsr(p),C);
			else if (valRsb70 <= 32) *Carry = get_bit(valRm, valRsb70 - 1);
			else *Carry = 0; 
			shifter_operand = (valRm >> valRs);
			break;
		case 4:		//ASR imm
			shift_value = get_bits(ins,11,7);
			if (shift_value == 0) *Carry = get_bit(valRm,31);
			else *Carry = get_bit(valRm, shift_value - 1);
			shifter_operand = asr(valRm, shift_value);
			break;
		case 5:		//ASR reg
			rs = get_bits(ins,11,8);
			valRs = arm_read_register(p,rs);
			valRsb70 = get_bits(valRs, 7, 0);
			if (valRsb70 == 0) *Carry = get_bit(arm_read_cpsr(p),C);
			else if (valRsb70 <= 32) *Carry = get_bit(valRm, valRsb70 - 1);
			else *Carry = 0; 
			shifter_operand = asr(valRm, valRs);
			break;
		case 6:		//ROR imm
			shift_value = get_bits(ins,11,7);
			if (shift_value != 0) *Carry = get_bit(valRm, shift_value - 1);
			// else opérande RRX non codée  
			shifter_operand = ror(valRm, shift_value);
			break;
		case 7:		//ROR reg
			rs = get_bits(ins,11,8);
			valRs = arm_read_register(p,rs);
			if (get_bits(valRs, 7, 0) == 0) *Carry = get_bit(arm_read_cpsr(p),C);
			else if (get_bits(valRs, 4, 0) == 0) *Carry = get_bit(valRm, 31);
			else *Carry = get_bit(valRm, get_bits(valRs, 4, 0) - 1); 
			shifter_operand = ror(valRm, valRs);
			break;
		default:	//ERROR
			return UNDEFINED_INSTRUCTION;
	}

	valRn = arm_read_register(p,rn);
    value = operateur(valRn, shifter_operand);

    if ((get_bit(shifter_operand,31) == get_bit(valRn, 31)) && (get_bit(value,31) != get_bit(shifter_operand,31))) *oVerflow = 1;
	else *oVerflow = 0;

	if (shifter_operand > value || valRn > value) *CarryFrom = 1;
	else *CarryFrom = 0;

	return value;
} 


/* Decoding functions for different classes of instructions */
int arm_data_processing_shift(arm_core p, uint32_t ins) {
	uint8_t rd = get_bits(ins,15,12);
	uint32_t value;
	uint8_t opcode = get_bits(ins,24,21);	//Opcode - bits 21 à 24
	uint8_t s = get_bit(ins,20);			//Bit shift
	int oVerflow,Carry,CarryFrom;
    uint32_t (*operandType)(arm_core, uint32_t, uint32_t (*operateur)(uint32_t, uint32_t), int*, int*, int*);

    if (get_bit(ins, 25) == 0) operandType = data_processing_operand;
    else operandType = data_processing_immediate_operand;

	switch(opcode){
		case 0:		//AND
            value = operandType(p, ins, logical_and, &oVerflow, &Carry, &CarryFrom);
			arm_write_register(p, rd, value);
			if(s){
				if(rd != 15)
					ZNCV_update(p, &oVerflow, &Carry, value, 0, 1);
                else if (arm_current_mode_has_spsr(p))
                    arm_write_cpsr(p, arm_read_spsr(p));
			}
			return 0;
		case 1:		//EOR
			value = operandType(p, ins, logical_eor, &oVerflow, &Carry, &CarryFrom);
			arm_write_register(p, rd, value);
			if(s){
				if(rd != 15)
				    ZNCV_update(p, &oVerflow, &Carry, value, 0, 1);
				else if (arm_current_mode_has_spsr(p))
				    arm_write_cpsr(p, arm_read_spsr(p));
			}
			return 0;
		case 2:		//SUB
			value = operandType(p, ins, sub, &oVerflow, &Carry, &CarryFrom);
			Carry = ~CarryFrom;
			arm_write_register(p, rd, value);
			if(s){
				if(rd != 15)
					ZNCV_update(p, &oVerflow, &Carry, value, 1, 1);
                else if (arm_current_mode_has_spsr(p))
                    arm_write_cpsr(p, arm_read_spsr(p));
			}
			return 0;
		case 3:		//RSB
			value = operandType(p, ins, rsb, &oVerflow, &Carry, &CarryFrom);
			Carry = ~CarryFrom;
			arm_write_register(p, rd, value);
			if(s){
				if(rd != 15)
					ZNCV_update(p, &oVerflow, &Carry, value, 1, 1);
                else if (arm_current_mode_has_spsr(p))
                    arm_write_cpsr(p, arm_read_spsr(p));
			}
			return 0;
		case 4:		//ADD
			value = operandType(p, ins, add, &oVerflow, &Carry, &CarryFrom);
			Carry = CarryFrom;
			arm_write_register(p, rd, value);
			if(s){
				if(rd != 15)
					ZNCV_update(p, &oVerflow, &Carry, value, 1, 1);
                else if (arm_current_mode_has_spsr(p))
                    arm_write_cpsr(p, arm_read_spsr(p));
			}
			return 0;
		case 5:		//ADC
			value = operandType(p, ins, add, &oVerflow, &Carry, &CarryFrom);
			Carry = CarryFrom;
			value = value + get_bit(arm_read_cpsr(p),C);
			arm_write_register(p, rd, value);
			if(s){
				if(rd != 15)
					ZNCV_update(p, &oVerflow, &Carry, value, 1, 1);
                else if (arm_current_mode_has_spsr(p))
                    arm_write_cpsr(p, arm_read_spsr(p));
			}
			return 0;
		case 6:		//SBC
			value = operandType(p, ins, sub, &oVerflow, &Carry, &CarryFrom);
			Carry = ~CarryFrom;
			value = value - ~get_bit(arm_read_cpsr(p),C);
			arm_write_register(p, rd, value);
			if(s){
				if(rd != 15)
					ZNCV_update(p, &oVerflow, &Carry, value, 1, 1);
                else if (arm_current_mode_has_spsr(p))
                    arm_write_cpsr(p, arm_read_spsr(p));
			}
			return 0;

		case 7:		//RSC
			value = operandType(p, ins, rsb, &oVerflow, &Carry, &CarryFrom);
			Carry = ~CarryFrom;
			value = value - ~get_bit(arm_read_cpsr(p),C);
			arm_write_register(p, rd, value);
			if(s){
				if(rd != 15)
				    ZNCV_update(p, &oVerflow, &Carry, value, 1, 1);
                else if (arm_current_mode_has_spsr(p))
                    arm_write_cpsr(p, arm_read_spsr(p));
			}
			return 0;
		case 8:		//TST
			value = operandType(p, ins, logical_and, &oVerflow, &Carry, &CarryFrom);
			ZNCV_update(p, &oVerflow, &Carry, value, 0, 1);
			return 0;
		case 9:		//TEQ
			value = operandType(p, ins, logical_eor, &oVerflow, &Carry, &CarryFrom);
			ZNCV_update(p, &oVerflow, &Carry, value, 0, 1);
			return 0;
		case 10:	//CMP
			value = operandType(p, ins, sub, &oVerflow, &Carry, &CarryFrom);
			Carry = ~CarryFrom;
			ZNCV_update(p, &oVerflow, &Carry, value, 1, 1);
			return 0;
		case 11:	//CMN
			value = operandType(p, ins, add, &oVerflow, &Carry, &CarryFrom);
			Carry = CarryFrom;
			ZNCV_update(p, &oVerflow, &Carry, value, 1, 1);
			return 0;
		case 12:	//ORR
			value = operandType(p, ins, logical_or, &oVerflow, &Carry, &CarryFrom);
			arm_write_register(p, rd, value);
			if(s){
				if(rd != 15){
					ZNCV_update(p, &oVerflow, &Carry, value, 0, 1);
				}
                else if (arm_current_mode_has_spsr(p))
                    arm_write_cpsr(p, arm_read_spsr(p));
			}
			return 0;
		case 13:	//MOV
			value = operandType(p, ins, mov, &oVerflow, &Carry, &CarryFrom);
			arm_write_register(p, rd, value);
			if(s){
				if(rd != 15)
					ZNCV_update(p, &oVerflow, &Carry, value, 0, 1);
                else if (arm_current_mode_has_spsr(p))
                    arm_write_cpsr(p, arm_read_spsr(p));
			}
			return 0;
		case 14:	//BIC
			value = operandType(p, ins, bic, &oVerflow, &Carry, &CarryFrom);
			arm_write_register(p, rd, value);
			if(s){
				if(rd != 15)
					ZNCV_update(p, &oVerflow, &Carry, value, 0, 1);
                else if (arm_current_mode_has_spsr(p))
                    arm_write_cpsr(p, arm_read_spsr(p));
			}
			return 0;
		case 15:	//MVN
			value = operandType(p, ins, mov, &oVerflow, &Carry, &CarryFrom);
			arm_write_register(p, rd, ~value);
			if(s){
				if(rd != 15)
					ZNCV_update(p, &oVerflow, &Carry, value, 0, 1);
                else if (arm_current_mode_has_spsr(p))
                    arm_write_cpsr(p, arm_read_spsr(p));
			}
			return 0;
		default:	//ERROR
			return UNDEFINED_INSTRUCTION;
	}
}
