.global main
.text
div2:
    bic r0,r1
    mov pc,lr
main:
    mvn r0,#0
    mov r1,#0x80000000
loop:
    bl div2
    mov r1,r1,LSR #1
    cmp r0,r1
    bhi loop
    swi 0x123456
