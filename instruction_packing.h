/*****************************************************************************
 *
 *                                  instruction_packing.h
 *
 *     Authors:    Eric Zhao, Leo Kim
 *     Date:       November 21, 2022
 *
 *     Summary:
 *     This is the public interface of our instruction packing module. 
 *     This module allows the user to manipulate a 32-bit UM instruction word
 *     with a variety of different functions that add and extract data from
 *     requested fields, such as the register numbers, operation code, and load 
 *     values. This module is exported to our operations module.
 * 
 *
 ****************************************************************************/

#ifndef INSTRUCTION_PACKING_INCLUDED
#define INSTRUCTION_PACKING_INCLUDED

#include "stdint.h"

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
                          unsigned char d);


/* FUNCTION:    get_operation
 * Purpose:     unpack an operation enum from a given instruction
 * Arg:         instruction: a 32-bit word representing an instruction
 * Returns:     the enum of the operation 
 * Effect:      N/A
 * Exported to: Operation module: used when processing a word
 * Error:       N/A
 */
uint32_t get_operation(uint32_t instruction);


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
uint32_t get_register(uint32_t instruction, char reg);


/* FUNCTION:    get_value
 * Purpose:     unpack the value from the load value instruction
 * Arg:         instruction: an uint32_t word representing the instruction
 * Returns:     the value stored in the given instruction
 * Effect:      N/A
 * Exported to: Operation module: used when processing a word
 * Error:       N/A
 */
uint32_t get_value(uint32_t instruction);

#endif