[bits 64]
global asm_init
global asm_exit

asm_init:
   xor rax, rax
   ret
asm_exit:
   nop
   nop
   ret
