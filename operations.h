/*****************************************************************************
 *
 *                                  operations.h
 *
 *     Assignment: UM
 *     Authors:    Eric Zhao, Leo Kim
 *     Date:       November 21, 2022
 *
 *     Summary:
 *     This is the private implementation of our operations module. This module
 *     does the core of the operations in our UM program. Given an instruction,
 *     this module uses private helper functions to execute the requested
 *     operations on the requested registers. This module is exported to our UM
 *     main program.
 * 
 *
 ****************************************************************************/

#ifndef UM_OPERATIONS_INCLUDED
#define UM_OPERATIONS_INCLUDED

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct Operations_T *Operations_T;

/* FUNCTION:    Operations_new
 * Purpose:     Constructor for the operation struct that contains the memory
 *              segments
 * Arg:         N/A
 * Returns:     Pointer to a operation struct
 * Exported to: N/A
 * Effect:      N/A
 * Error:       Check Runtime error if the memory allocation fails
 */
Operations_T Operations_new();

/* FUNCTION:    Operations_free
 * Purpose:     Free the memory associated with an operations struct
 * Arg:         op: a pointer to an operations struct
 * Returns:     N/A
 * Exported to: Our main program module: used in running the command loop
 * Effect:      Frees the memory associated with an operations struct
 * Error:       If a NULL pointer or a pointer to a NULL pointer is passed in
 */
void Operations_free(Operations_T *op);

/* FUNCTION:    read_in_program
 * Purpose:     Reads the file, packs the content into different words, and
 *              put them into segment 0
 * Arg:         *input: pointer to a file that has been opened
 *              num_words: the number of words in the file
 *              op: po
 * Returns:     N/A
 * Exported to: Our main program module: used in running the command loop
 * Effect:      
 * Error:       Runtime error if the file pointer is NULL
 *              Runtime error if the operation struct is NULL
 */
void read_in_program(FILE *input, uint32_t num_words, Operations_T op);

/* FUNCTION:    next_instruction
 * Purpose:     get the next instruction in the program provided by the user
 * Arg:         op: an instance of the operations struct storing our UM’s data
 *              structures
 * Returns:     A uint32_t representing the next instruction in the program
 * Exported to: Our main program module: used in running the command loop
 * Effect:      N/A
 * Error:       Checked runtime if op is NULL
 */
uint32_t next_instruction(Operations_T op);

/* FUNCTION:  do_instruction
 * Purpose:          execute the instruction given
 * Arg:                 instruction: the next instruction to be executed
    op: an instance of the operations struct storing our UM’s data structures
 * Returns:           False if the instruction is “Halt”, true otherwise
 * Exported to:    Our main program module: used in running the command loop
 * Effect:             Executes the requested instruction (using private helper
 *                 functions)
 * Error:               N/A
 */
bool do_instruction(uint32_t instruction, Operations_T op);

#endif