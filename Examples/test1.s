.global main
.text
main:
    ldr r2,=donnees
    mov r1,#0
loop:
    ldrb r0,[r2, r1]
    teq r0,#0xF0
    add r1,r1,#1
    bne loop
    swi 0x123456
.data
donnees:
    .word 0x12345678
    .word 0x9ABCDEF0
