/*
 *  Define assembler memonics for asm code creation
 *  and memonic extensions for different operand sizes
 */

#ifndef __BCC_ASMSUPP_H_INCLUDED
#define __BCC_ASMSUPP_H_INCLUDED

#define __EMIT__(x) __emit__((char)(x))

#define MOV_INS  mov
#define XOR_INS  xor
#define OR_INS   or
#define AND_INS  and

/* for opcode generation using __emit__ */
/* mov [...],al */
#define OPCODE_mov_mem_b 0x88
/* mov [...],ax */
#define OPCODE_mov_mem_w 0x89
/* xor [...],al */
#define OPCODE_xor_mem_b 0x30
/* xor [...],ax */
#define OPCODE_xor_mem_w 0x31
/* or  [...],al */
#define OPCODE_or_mem_b  0x08
/* or  [...],ax */
#define OPCODE_or_mem_w  0x09
/* and [...],al */
#define OPCODE_and_mem_b 0x20
/* and [...],ax */
#define OPCODE_and_mem_w 0x21


#define OP8b   b
#define OP16b  w
#define OP32b  l

#endif
