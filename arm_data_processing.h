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
#ifndef __ARM_DATA_PROCESSING_H__
#define __ARM_DATA_PROCESSING_H__
#include <stdint.h>
#include "arm_core.h"

int arm_data_processing_shift(arm_core p, uint32_t ins);
int arm_data_processing_immediate_msr(arm_core p, uint32_t ins);
void ZNCV_update(arm_core p, int* oVerflow, int* Carry, uint32_t value, int oV, int Ca);

uint32_t data_processing_operand(arm_core p, uint32_t ins, uint32_t (*operateur)(uint32_t, uint32_t), int* oVerflow, int* Carry);
uint32_t logical_and(uint32_t val_rn, uint32_t shifter_operand);
uint32_t logical_eor(uint32_t val_rn, uint32_t shifter_operand);
uint32_t sub(uint32_t val_rn, uint32_t shifter_operand);
uint32_t rsb(uint32_t val_rn, uint32_t shifter_operand);
uint32_t add(uint32_t val_rn, uint32_t shifter_operand);
uint32_t logical_or(uint32_t val_rn, uint32_t shifter_operand);
uint32_t mov(uint32_t val_rn, uint32_t shifter_operand);
uint32_t bic(uint32_t val_rn, uint32_t shifter_operand);
#endif
