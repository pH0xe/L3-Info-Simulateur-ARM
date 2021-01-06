#include <stdio.h>
#include <stdlib.h>
#include "registers.h"
#include "util.h"
#include "arm_constants.h"


void afficher_mode(registers r){
    printf("Mode courant: ");
    switch(get_mode(r)){
        case 0x10:
            printf("USR\n");
            break;
        case 0x11:
            printf("FIQ\n");
            break;
        case 0x12:
            printf("IRQ\n");
            break;
        case 0x13:
            printf("SVC\n");
            break;
        case 0x17:
            printf("ABT\n");
            break;
        case 0x1b:
            printf("UND\n");
            break;
        case 0x1f:
            printf("SYS\n");
            break;
        default:
            printf("Mode inexistant\n");
            break;
    }
}

int main() {
    registers r = registers_create();
    if(r == NULL){
        printf("Erreur d'allocation mémoire à la création des registres\n");
        return -1;
    }
    afficher_mode(r);
    printf("Mode courant possède un registre SPSR: ");
    if(current_mode_has_spsr(r)) printf("Oui\n");
    else printf("Non\n");

    printf("Mode courant privilégié: ");
    if(in_a_privileged_mode(r)) printf("Oui\n");
    else printf("Non\n");

    for (int i=0;i<18;i++){
        printf("Le registre %s contient la valeur %x\n",arm_get_register_name(i), read_register(r,i));
    }

    printf("On écrit la valeur 0x00112233445566778899aabbccddeeff dans les registres 0 à 15, un octet par registre en commençant par la gauche\n");
    uint32_t val = 0x00;
    for(int i=0;i<16;i++){
        write_register(r,i,val);
        val += 0x11;
    }
    printf("On écrit la valeur 0x12345678 dans CPSR\n");
    write_cpsr(r,0x12345678);
    for (int i=0;i<18;i++){
        printf("Le registre %s contient la valeur %x\n",arm_get_register_name(i), read_register(r,i));
    }
    
    registers_destroy(r);
}
/*
//changement mode

in_a_privileged_mode
*/




