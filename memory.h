/*****************************************************************************
 *
 *                                  memory.h
 *
 *     Authors:    Eric Zhao, Leo Kim
 *     Date:       November 21, 2022
 *
 *     Summary:
 *     This is the public interface of our memory module. This module 
 *     allows the user to load a program into segment 0 of the memory, allocate
 *     and deallocate memory segments, extract values from specific indices of 
 *     memory, get the next instruction from the loaded program, and load a new
 *     program into segment 0. This module is exported to our operations 
 *     module.
 * 
 *
 ****************************************************************************/

#ifndef UM_MEMORY_INCLUDED
#define UM_MEMORY_INCLUDED

#include <stdint.h>

typedef struct Memory_T *Memory_T;

/* FUNCTION:    Memory_new
 * Purpose:     Initialize a Hanson sequence to store the main memory, a Hanson
 *              stack to store the segmenets that have been previously mapped
 * Arg:         N/A
 * Returns:     An instance of the struct Memory_T that contains our data
 *              structures to represent the memory segments
 * Effect:      The program pointer is initially set to null
 * Exported to: Operation module. Used when initializing the memory module
 * Error:       Checked runtime error for unsuccessful memory allocation
 */
Memory_T Memory_new();


/* FUNCTION:    Memory_free
 * Purpose:     free the memory associated with the data structures in our
 *              memory struct
 * Arg:         mem: A pointer to a Memory_T (which itself is a struct pointer)
 * Returns:     N/A
 * Effect:      free the memory associated with an instance of our memory 
 *		struct
 * Exported to: Operations module. Used when freeing an operations struct 
 *		file
 * Error:       Checked Runtime Error if a NULL pointer or a pointer to a NULL
 *              pointer is passed in
 */
void Memory_free(Memory_T *mem);


/* FUNCTION:    new_segment
 * Purpose:     map a new segment where all the words are 0s.
 * Arg:         size: the number of words in the segment
 	        mem: struct that contains the components of the memory 
                     management unit
 * Returns:     the segment ID of the newly mapped segment 
 * Effect:      Checks if the unmap_mem stack is empty
 *              Adds the segment to the main_mem sequence
 * Exported to: Operation module: this function is used in the map segment 
 *		command
 * Error:       Checked Runtime error if the size of the sequence is 232
 *              Checked Runtime if mem is NULL
 */
uint32_t new_segment(uint32_t size, Memory_T mem);


/* FUNCTION:    remove_segment
 * Purpose:     unmap a given segment
 * Arg:         seg_id: the index that identifies a specific memory
 *              segment
 *              mem: struct that contains the components of the memory
 *              management unit
 * Returns:     N/A
 * Effect:      Pushes the ID of the unmapped segment onto the unmap_mem 
 *              stack
 * Exported to: Operation module: this function is used in the unmap 
 *              segment command
 * Error:       Checked runtime if ID is invalid
 *              Checked Runtime if mem is NULL
 */
void remove_segment(uint32_t seg_id, Memory_T mem);


/* FUNCTION:    load_program
 * Purpose:     copy a given segment into segment 0
 * Arg:         seg_id: the index that identifies a specific memory segment
 *              mem: struct that contains the components of the memory
 *              management unit
 * Returns:     N/A
 * Effect:      Replaces the segment at seg_id with an empty segment and frees
 *              the segment previously stored in segment 0
 * Exported to: Operation module: Used in the load program command
 * Error:       Checked Runtime if mem is NULL
 */
void load_program(uint32_t seg_id, uint32_t offset, Memory_T mem);


/* FUNCTION:    *word_at
 * Purpose:     returns a pointer to a word given a segment id and the index of 
 *              word in that segment
 * Arg:         seg_id: segment ID of the given segment
 *              word_index: a word index that indicates a specific 
 *              word in that segment
 *		mem: struct that contains the components of the memory 
 *                   management unit
 * Returns:     Pointer to the requested uint32_t word
 * Effect:      N/A
 * Exported to:	Operation module: used in segmented load and segmented store 
 *              commands
 * Error:       Checked Runtime if mem is NULL
 */
uint32_t *word_at(uint32_t seg_id, uint32_t word_index, Memory_T mem);


/* FUNCTION:    get_next_instruction
 * Purpose:     returns the next instruction relative to the current program 
 *		counter
 * Arg:         mem: struct that contains the components of the memory 
 *		management unit
 * Returns:     an uint32_t instruction word
 * Effect:      moves the program counter to the next word in segment 0
 		If the next word does not exist, returns 0
 * Exported to:	Operation module: used to get the next instruction for 
 *		operations
 * Error:       Checked Runtime if mem is NULL
 */
uint32_t get_next_instruction(Memory_T mem);


/* FUNCTION:    initialize_program_ptr
 * Purpose:     set the program pointer to the first word in segment 0
 * Arg:         mem: struct that contains the components of the memory 
 *                   management unit
 * Returns:     N/A
 * Effect:      sets the program pointer to point to the first instruction in
 *              segment 0
 * Exported to:	Operation module: used in loading in the initial program from
 *              a file
 * Error:       Checked Runtime if mem is NULL
 */
void initialize_program_ptr(Memory_T mem);

#endif
