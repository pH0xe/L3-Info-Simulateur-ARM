.global main
.text
main:

    mvn r0,#0
    adds r0,#1
    mrs r2, cpsr
    adc r0,r0,#0
    ldr r1,=donnees
    strh r2,[r1, #2]
    mov r2,r2,LSR #24
    strb r2,[r1]
    mov r2,r2,LSL #8
    strb r2,[r1, #1]
    
end:
    swi 0x123456
.data
donnees:
    .word 0x12345678
    .word 0x9ABCDEF0

