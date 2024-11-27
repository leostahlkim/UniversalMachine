/*****************************************************************************
 *
 *                            instruction_packing.c
 *
 *     Authors:    Eric Zhao, Leo Kim
 *     Date:       November 21, 2022
 *
 *     Summary:
 *     This is the private implementation of our instruction packing module. 
 *     This module allows the user to manipulate a 32-bit UM instruction word
 *     with a variety of different functions that add and extract data from
 *     requested fields, such as the register numbers, operation code, and load
 *     values. This module uses functions from our bitpacking module from
 *     Arith. This module is exported to our operations module.
 * 
 *
 ****************************************************************************/

#include "instruction_packing.h"
#include <bitpack.h>

/* bit size of a character, since we are reading data from the file in terms of
   characters */
#define char_bitsize 8

/* the least significant bit and the width for the operation field */
#define op_lsb 28
#define op_width 4

/* the least significant bit and the width for the three registers fields */
#define c_lsb 0
#define b_lsb 3
#define a_lsb 6
#define reg_width 3

/* the least significant bit and the width of all the fields for the load_value
   operation */
#define load_reg_opcode 13
#define load_reg_lsb 25
#define val_lsb 0
#define val_width 25


/* FUNCTION:    pack_instruction
 * Purpose:     pack 4 separate char variables representing different bits of
 *              an instruction into a single uint32_t instruction
 * Arg:         a: the most significant byte of the instruction
 *	        b: the second most significant byte of the instruction
 *              c: the third most significant byte of the instruction
 *	        d: the least significant byte of the instruction
 * Returns:     The 32-bit packed instruction
 * Effect:      N/A
 * Exported to: Operation module. Used when reading in instructions from the 
 *		file
 * Error:       N/A
 */
uint32_t pack_instruction(unsigned char a, unsigned char b, unsigned char c, 
			  unsigned char d)
{
        unsigned lsb = 0;
        
        /* set the least significant bits of the instruction to the 4th char 
           read in in big endian order */
        uint32_t instruction = (uint32_t) d;

        /* initialize the rest of the characters into the instruction and 
           updates the lsb incrementally */
        lsb += char_bitsize;
        instruction = Bitpack_newu(instruction, char_bitsize, lsb, c);
        lsb += char_bitsize;
        instruction = Bitpack_newu(instruction, char_bitsize, lsb, b);
        lsb += char_bitsize;
        instruction = Bitpack_newu(instruction, char_bitsize, lsb, a);

        return instruction;
}


/* FUNCTION:    get_operation
 * Purpose:     unpack an operation enum from a given instruction
 * Arg:         instruction: a 32-bit word representing an instruction
 * Returns:     the enum of the operation 
 * Effect:      N/A
 * Exported to: Operation module: used when processing a word
 * Error:       N/A
 */
uint32_t get_operation(uint32_t instruction)
{
        return Bitpack_getu(instruction, op_width, op_lsb);
}


/* FUNCTION:    get_register
 * Purpose:     unpack a register enum from a given instruction
 * Arg:         instruction: a 32-bit word representing an instruction
 *	        reg: char indicating which register to get
 * Returns:     the enum of the requested register, -1 (0xffff) if invalid
 *              register requested
 * Effect:      N/A
 * Exported to: Operation module: used when processing a word
 * Error:       N/A
 */
uint32_t get_register(uint32_t instruction, char reg)
{       
        if (reg == 'a') {
                /* check if a is being requested in a loadval instruction */
                if (get_operation(instruction) == load_reg_opcode) {
                        return Bitpack_getu(instruction, reg_width,
                                            load_reg_lsb);
                } else {
                        return Bitpack_getu(instruction, reg_width,
                                            a_lsb);
                }
        } else if (reg == 'b') {
		return Bitpack_getu(instruction, reg_width, b_lsb);
	} else if (reg == 'c') {
		return Bitpack_getu(instruction, reg_width, c_lsb);
	} else {
		/* for invalid request of registers */
		return -1;
	}
}


/* FUNCTION:    get_value
 * Purpose:     unpack the value from the load value instruction
 * Arg:         instruction: an uint32_t word representing the instruction
 * Returns:     the value stored in the given instruction
 * Effect:      N/A
 * Exported to: Operation module: used when processing a word
 * Error:       N/A
 */
uint32_t get_value(uint32_t instruction)
{
        return Bitpack_getu(instruction, val_width, val_lsb);
}
