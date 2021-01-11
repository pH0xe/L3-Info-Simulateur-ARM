.global main
.text
main:
    ldr r0,=donnees
    ldrh r4,[r0]
    ldrh r3,[r0, #2]
    mov r3,r3,ror #16
    orr r3,r4,r3
    ldrh r2,[r0, #4]
    ldrh r1,[r0, #6]
    mov r1,r1,ror #16
    orr r1,r2,r1
    stm r0, {r1,r3}
end:
    swi 0x123456
.data
donnees:
    .word 0x12345678
    .word 0x9ABCDEF0
