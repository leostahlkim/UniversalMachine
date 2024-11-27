/*****************************************************************************
 *
 *                                  operations.c
 *
 *     Assignment: UM
 *     Authors:    Eric Zhao, Leo Kim
 *     Date:       November 21, 2022
 *
 *     Summary:
 *     This is the private implementation of our operations module. This module
 *     does the core of the operations in our UM program. Given an instruction,
 *     this module uses private helper functions to execute the requested
 *     operations on the requested registers. Our operations module stores an
 *     instance of our UM memory struct and an array of registers. This module 
 *     uses functions from our instruction packing and memory modules. 
 *     This module is exported to our UM main program.
 * 
 *
 ****************************************************************************/

#include "operations.h"
#include "memory.h"
#include "instruction_packing.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define num_registers 8

/* 
 * This struct will be exported to our main program module as a struct pointer.
 * memory: pointer to a struct that stores our data structures representing
 * our memory management.
 * registers: an array of uint32_t of registers in the UM machine
 */
struct Operations_T {
	Memory_T memory;
        uint32_t registers[num_registers];
};

/* 
 * typedef all the operation codes into an enum
 */
typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;


/* private helper functions, details can be viewed below */
void load_value(uint32_t instruction, Operations_T op);
void output    (uint32_t instruction, Operations_T op);
void input     (uint32_t instruction, Operations_T op);
void add       (uint32_t instruction, Operations_T op);
void multiply  (uint32_t instruction, Operations_T op);
void divide    (uint32_t instruction, Operations_T op);
void nand      (uint32_t instruction, Operations_T op);
void cond_move (uint32_t instruction, Operations_T op);
void map_seg   (uint32_t instruction, Operations_T op);
void unmap_seg (uint32_t instruction, Operations_T op);
void seg_store (uint32_t instruction, Operations_T op);
void seg_load  (uint32_t instruction, Operations_T op);
void load_prog (uint32_t instruction, Operations_T op);

/* FUNCTION:    Operations_new
 * Purpose:     Constructor for the operation struct that contains the memory
 *              segments
 * Arg:         N/A
 * Returns:     Pointer to a operation struct
 * Exported to: N/A
 * Effect:      N/A
 * Error:       Check Runtime error if the memory allocation fails
 */
Operations_T Operations_new()
{
        Operations_T op = malloc(sizeof(*op));
        // fprintf(stderr, "size of op: %lu\n", sizeof(*op));
        assert(op != NULL);
        
        for (uint32_t i = 0; i < num_registers; i++) {
                op->registers[i] = 0;
        }

        op->memory = Memory_new();

        return op;
}


/* FUNCTION:    Operations_free
 * Purpose:     Free the memory associated with an operations struct
 * Arg:         op: a pointer to an operations struct
 * Returns:     N/A
 * Exported to: Our main program module: used in running the command loop
 * Effect:      Frees the memory associated with an operations struct
 * Error:       If a NULL pointer or a pointer to a NULL pointer is passed in
 */
void Operations_free(Operations_T *op)
{
        assert(op != NULL);
        assert(*op != NULL);
        
        Memory_free(&((*op)->memory));
        free(*op);

        *op = NULL;
}


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
void read_in_program(FILE *input, uint32_t num_words, Operations_T op) 
{
        assert(input != NULL);
        assert(op != NULL);

        /* creates a new segment in the memory at segment 0 */
        new_segment(num_words, op->memory);

        /* for every word in the input file */
        for (uint32_t i = 0; i < num_words; i++) {
                
                /* read in and pack each byte of the instruction */
                unsigned char a = fgetc(input);
                unsigned char b = fgetc(input);
                unsigned char c = fgetc(input);
                unsigned char d = fgetc(input);
                uint32_t instruction = pack_instruction(a, b, c, d);

                /* add the instruction to segment 0 of memory */
                *word_at(0, i, op->memory) = instruction;
        }

        initialize_program_ptr(op->memory);
}


/* FUNCTION:    next_instruction
 * Purpose:     get the next instruction in the program provided by the user
 * Arg:         op: an instance of the operations struct storing our UM’s data
 *              structures
 * Returns:     A uint32_t representing the next instruction in the program
 * Exported to: Our main program module: used in running the command loop
 * Effect:      N/A
 * Error:       Checked runtime if op is NULL
 */
uint32_t next_instruction(Operations_T op)
{
        assert(op != NULL);
        
        return get_next_instruction(op->memory);
}


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
bool do_instruction(uint32_t instruction, Operations_T op)
{
        assert(op != NULL);
        
        Um_opcode operation = get_operation(instruction);
        if (operation == HALT) {
                return false;
        } if (operation == LV) {
                load_value(instruction, op);
        } else if (operation == OUT) {
                output(instruction, op);
        } else if (operation == IN) {
                input(instruction, op);
        } else if (operation == ADD) {
                add(instruction, op);
        } else if (operation == MUL) {
                multiply(instruction, op);
        } else if (operation == DIV) {
                divide(instruction, op);
        } else if (operation == NAND) {
                nand(instruction, op);
        } else if (operation == CMOV) {
                cond_move(instruction, op);
        } else if (operation == ACTIVATE) {
                map_seg(instruction, op);
        } else if (operation == INACTIVATE) {
                unmap_seg(instruction, op);
        } else if (operation == SSTORE) {
                seg_store(instruction, op);
        } else if (operation == SLOAD) {
                seg_load(instruction, op);
        } else if (operation == LOADP) {
                load_prog(instruction, op);
        }

        return true;
}


/* FUNCTION:    load_value
 * Purpose:     load value into a given register a
 * Arg:         instruction: the instruction to be executed
 *              op: pointer to the operations struct storing our UM’s data
 *              structures
 * Returns:     N/A
 * Exported to: N/A
 * Effect:      Populates the requested register with the given value
 * Error:       Checked runtime error if op is a NULL pointer
 */
void load_value(uint32_t instruction, Operations_T op)
{
        assert(op != NULL);

        /* get the register we are loading into */
        uint32_t register_num = get_register(instruction, 'a');

        /* load the given value into that register */
        op->registers[register_num] = get_value(instruction);
}


/* FUNCTION:    output
 * Purpose:     output the value in register c to the I/O device
 * Arg:         instruction: the instruction to be executed
 *              op: pointer to the operations struct storing our UM’s data
 *              structures
 * Returns:     N/A
 * Exported to: N/A
 * Effect:      outputs the value in register c
 * Error:       Checked runtime error if op is a NULL pointer or if the value
 *              in register c is greater than 255
 */
void output(uint32_t instruction, Operations_T op)
{
        assert(op != NULL);
        
        /* get the register we are outputting */
        uint32_t register_num = get_register(instruction, 'c');
        
        /* get the value in the register */
        uint32_t value = op->registers[register_num];
        
        /* check for range and output */
        assert(value < 256);
        fputc(value, stdout);
}

/* FUNCTION:    input
 * Purpose:     read in a char from the the user and store it in register c
 * Arg:         instruction: the instruction to be executed
 *              op: pointer to the operations struct storing our UM’s data
 *              structures
 * Returns:     N/A
 * Exported to: N/A
 * Effect:      Populates register c with the given value
 * Error:       Checked runtime error if op is a NULL pointer
 */
void input(uint32_t instruction, Operations_T op)
{
        assert(op != NULL);
        
        /* get the register we are inputting */
        uint32_t register_num = get_register(instruction, 'c');

        int value = fgetc(stdin);
        
        if (value == -1) {
                value = ~0;
        } else {
                /* check that the value falls in the correct range */
                assert(value < 256);
        }
        
        /* put the value in the given register */
        op->registers[register_num] = value;
}


/* FUNCTION:    add
 * Purpose:     add the values in registers b and c and store the result
 *              in register a. Register numbers are packed into the instruction
 * Arg:         instruction: the instruction to be executed
 *              op: pointer to the operations struct storing our UM’s data
 *              structures
 * Returns:     N/A
 * Exported to: N/A
 * Effect:      Populates register a with the sum
 * Error:       Checked runtime error if op is a NULL pointer
 */
void add(uint32_t instruction, Operations_T op)
{
        assert(op != NULL);

        /* get the register indexes */
        uint32_t registera_num = get_register(instruction, 'a');
        uint32_t registerb_num = get_register(instruction, 'b');
        uint32_t registerc_num = get_register(instruction, 'c');
        
        /* get the values being added */
        uint32_t value_b = op->registers[registerb_num];
        uint32_t value_c = op->registers[registerc_num];

        /* store the added values in the requested register */
        op->registers[registera_num] = value_b + value_c;
}


/* FUNCTION:    multiply
 * Purpose:     mutiply values in registers b and c and store the result to 
 *              register a. Register numbers are packed into the instruction
 * Arg:         instruction: the instruction to be executed
 *              op: pointer to the operations struct storing our UM’s data
 *              structures
 * Returns:     N/A
 * Exported to: N/A
 * Effect:      Populates register a with the sum
 * Error:       Checked runtime error if op is a NULL pointer
 */
void multiply(uint32_t instruction, Operations_T op)
{
        assert(op != NULL);

        /* get the register indexes */
        uint32_t registera_num = get_register(instruction, 'a');
        uint32_t registerb_num = get_register(instruction, 'b');
        uint32_t registerc_num = get_register(instruction, 'c');
        
        /* get the values being multiplied */
        uint32_t value_b = op->registers[registerb_num];
        uint32_t value_c = op->registers[registerc_num];

        /* store the multiplied values in the requested register */
        op->registers[registera_num] = value_b * value_c;
}


/* FUNCTION:    divide
 * Purpose:     divide the value register b by the value in register c, stores
 *              the result in register a. Register numbers are packed into the 
 *              instruction.
 * Arg:         instruction: the instruction to be executed
 *              op: pointer to the operations struct storing our UM’s data
 *              structures
 * Returns:     N/A
 * Exported to: N/A
 * Effect:      Populates the requested register with the result
 * Error:       Checked runtime error if op is a NULL pointer
 */
void divide(uint32_t instruction, Operations_T op)
{
        assert(op != NULL);

        /* get the register indexes */
        uint32_t registera_num = get_register(instruction, 'a');
        uint32_t registerb_num = get_register(instruction, 'b');
        uint32_t registerc_num = get_register(instruction, 'c');
        
        /* get the values being divided */
        uint32_t value_b = op->registers[registerb_num];
        uint32_t value_c = op->registers[registerc_num];

        /* store the result in the requested register */
        op->registers[registera_num] = value_b / value_c;
}


/* FUNCTION:    nand
 * Purpose:     perform bitwise NAND instruction on registers b and c and store 
 *              the result in register a. Register numbers are packed into the 
 *              instruction.
 * Arg:         instruction: the instruction to be executed
 *              op: pointer to the operations struct storing our UM’s data
 *              structures
 * Returns:     N/A
 * Exported to: N/A
 * Effect:      Populates the requested register with the result
 * Error:       Checked runtime error if op is a NULL pointer
 */
void nand(uint32_t instruction, Operations_T op)
{
        assert(op != NULL);

        /* get the register indexes */
        uint32_t registera_num = get_register(instruction, 'a');
        uint32_t registerb_num = get_register(instruction, 'b');
        uint32_t registerc_num = get_register(instruction, 'c');
        
        /* get the values being nanded */
        uint32_t value_b = op->registers[registerb_num];
        uint32_t value_c = op->registers[registerc_num];

        /* store the result in the requested register */
        op->registers[registera_num] = ~(value_b & value_c);
}


/* FUNCTION:    cond_move
 * Purpose:     if the value in register c is not 0, store the value in register
 *              b in register c.
 * Arg:         instruction: the instruction to be executed
 *              op: pointer to the operations struct storing our UM’s data
 *              structures
 * Returns:     N/A
 * Exported to: N/A
 * Effect:      Populates the requested register with the result
 * Error:       Checked runtime error if op is a NULL pointer
 */
void cond_move(uint32_t instruction, Operations_T op)
{
        assert(op != NULL);

        /* get the register indexes */
        uint32_t registera_num = get_register(instruction, 'a');
        uint32_t registerb_num = get_register(instruction, 'b');
        uint32_t registerc_num = get_register(instruction, 'c');
        
        if (op->registers[registerc_num] != 0) {
                /* equate register a with register b if register c is 0 */
                op->registers[registera_num] = op->registers[registerb_num];
        }
}


/* FUNCTION:    map_seg
 * Purpose:     A new segment is created with a number of words equal to the 
 *              value in $r[C]. Each word in the new segment is initialized to 
 *              0. A bit pattern that is not all zeroes and that does not 
 *              identify any currently mapped segment is placed in $r[B]. 
 *              The new segment is mapped as $m[$r[B]].
 * Arg:         instruction: the instruction to be executed
 *              op: pointer to the operations struct storing our UM’s data
 *              structures
 * Returns:     N/A
 * Exported to: N/A
 * Effect:      Populates the requested register with the segment ID of the 
 *              newly allocated segment
 * Error:       Checked runtime error if op is a NULL pointer
 */
void map_seg(uint32_t instruction, Operations_T op)
{
        assert(op != NULL);

        /* get the register indexes */
        uint32_t registerb_num = get_register(instruction, 'b');
        uint32_t registerc_num = get_register(instruction, 'c');
        
        /* get the number of words from register c */
        uint32_t num_words = op->registers[registerc_num];

        /* store the segment ID in register b */
        op->registers[registerb_num] = new_segment(num_words, op->memory);
}


/* FUNCTION:    unmap_seg
 * Purpose:     The segment $m[$r[C]] is unmapped. Future Map Segment 
 *              instructions may reuse the identifier $r[C].
 * Arg:         instruction: the instruction to be executed
 *              op: pointer to the operations struct storing our UM’s data
 *              structures
 * Returns:     N/A
 * Exported to: N/A
 * Effect:      Populates the requested register with the result
 * Error:       Checked runtime error if op is a NULL pointer
 */
void unmap_seg(uint32_t instruction, Operations_T op)
{
        assert(op != NULL);

        /* get the register index */
        uint32_t registerc_num = get_register(instruction, 'c');

        remove_segment(op->registers[registerc_num], op->memory);
}


/* FUNCTION:    seg_store
 * Purpose:     store the value in register a into the memory at [$r[B]][$r[C]]
 * Arg:         instruction: the instruction to be executed
 *              op: pointer to the operations struct storing our UM’s data
 *              structures
 * Returns:     N/A
 * Exported to: N/A
 * Effect:      Populates the requested register with the result
 * Error:       Checked runtime error if op is a NULL pointer
 */
void seg_store(uint32_t instruction, Operations_T op)
{
        assert(op != NULL);

        /* get the register numbers */
        uint32_t registera_num = get_register(instruction, 'a');
        uint32_t registerb_num = get_register(instruction, 'b');
        uint32_t registerc_num = get_register(instruction, 'c');

        /* get the values from the registers */
        uint32_t value_a = op->registers[registera_num];
        uint32_t value_b = op->registers[registerb_num];
        uint32_t value_c = op->registers[registerc_num];
        
        /* store the value in register c in the requested location */
        *(word_at(value_a, value_b, op->memory)) = value_c;
}


/* FUNCTION:    seg_load
 * Purpose:     load the value in memory at [$r[B]][$r[C]] into register a 
 * Arg:         instruction: the instruction to be executed
 *              op: pointer to the operations struct storing our UM’s data
 *              structures
 * Returns:     N/A
 * Exported to: N/A
 * Effect:      Populates the requested register with the result
 * Error:       Checked runtime error if op is a NULL pointer
 */
void seg_load(uint32_t instruction, Operations_T op)
{
        assert(op != NULL);

        /* get the register numbers */
        uint32_t registera_num = get_register(instruction, 'a');
        uint32_t registerb_num = get_register(instruction, 'b');
        uint32_t registerc_num = get_register(instruction, 'c');

        /* get the values from the registers */
        uint32_t value_b = op->registers[registerb_num];
        uint32_t value_c = op->registers[registerc_num];
        
        op->registers[registera_num] = *(word_at(value_b, value_c, op->memory));
}


/* FUNCTION:    load_prog
 * Purpose:     Segment $m[$r[B]] is duplicated, and the duplicate replaces 
 *              $m[0], which is abandoned. The program counter is set to point 
 *              to $m[0][$r[C]]. If $r[B] = 0, the load-program operation is 
 *              expected to be extremely quick.
 * Arg:         instruction: the instruction to be executed
 *              op: pointer to the operations struct storing our UM’s data
 *              structures
 * Returns:     N/A
 * Exported to: N/A
 * Effect:      Populates the requested register with the result
 * Error:       Checked runtime error if op is a NULL pointer
 */
void load_prog(uint32_t instruction, Operations_T op)
{
        assert(op != NULL);

        /* get the register numbers */
        uint32_t registerb_num = get_register(instruction, 'b');
        uint32_t registerc_num = get_register(instruction, 'c');

        /* get the values from the registers */
        uint32_t value_b = op->registers[registerb_num];
        uint32_t value_c = op->registers[registerc_num];

        load_program(value_b, value_c, op->memory);
}
