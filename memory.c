/*****************************************************************************
 *
 *                                  memory.c
 *
 *     Authors:    Eric Zhao, Leo Kim
 *     Date:       November 21, 2022
 *
 *     Summary:
 *     This is the private implementation of our memory module. This module 
 *     allows the user to load a program into segment 0 of the memory, allocate
 *     and deallocate memory segments, extract values from specific indices of 
 *     memory, get the next instruction from the loaded program, and load a new
 *     program into segment 0. We implement the segmented memory as a Hanson 
 *     sequence of pointers to Hanson UArrays that store each segment. The 
 *     segment ID of each segment is the index of that segment's address in our
 *     sequence. When we remove a segment, we add its index to a stack that 
 *     stores deallocated segment IDs that can be reallocated in the future.
 *     This module is exported to our operations module.
 * 
 *
 ****************************************************************************/

#include "memory.h"
#include <seq.h>
#include <uarray.h>
#include <stack.h>
#include <stdlib.h>
#include <assert.h>

/* defines the byte size of a word */
#define word_size 4

/* struct definition for our Memory struct which holds:
 *      main_mem: a sequence that stores a pointer to each memory segment
 *      unmap_mem: a stack that stores indices of unmapped segments
 *      program_ptr: a pointer to the next instruction of our program
 */
struct Memory_T {
        Seq_T main_mem;
        Stack_T unmap_mem;
        uint32_t *program_ptr;
};


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
Memory_T Memory_new()
{
        Memory_T mem = malloc(sizeof(*mem));
        assert(mem != NULL);

        /* initialize memory data structures */
        mem->main_mem = Seq_new(0);
        mem->unmap_mem = Stack_new();
        mem->program_ptr = NULL;

        return mem;
}


/* FUNCTION:    Memory_free
 * Purpose:     free the memory associated with the data structures in our
 *              memory struct
 * Arg:         mem: A pointer to a Memory_T (which itself is a struct pointer)
 * Returns:     N/A
 * Effect:      free the memory associated with an instance of our memory 
 *              struct
 * Exported to: Operations module. Used when freeing an operations struct 
 *		file
 * Error:       Checked Runtime Error if a NULL pointer or a pointer to a NULL
 *              pointer is passed in
 */
void Memory_free(Memory_T *mem)
{
        /* checks for null argument */
        assert(mem != NULL && *mem != NULL);

        /* free the uarrays */
        int seq_size = Seq_length((*mem)->main_mem);
        for (int i = 0; i < seq_size; i++) {
                UArray_T seg_remove = Seq_remhi((*mem)->main_mem);
                UArray_free(&seg_remove);
        }
        
        /* free the sequence and the stack */
        Seq_free(&((*mem)->main_mem));
        Stack_free(&((*mem)->unmap_mem));
        free(*mem);
        *mem = NULL;
}


/* FUNCTION:    new_segment
 * Purpose:     map a new segment where all the words are 0s.
 * Arg:         size: the number of words in the segment
 	        mem: struct that contains the components of the memory 
                     management unit
 * Returns:     the segment ID of the newly mapped segment 
 * Effect:      Checks if the unmap_mem stack is empty
 *              Adds the segment to the main_mem sequence
 * Exported to: Operation module: this function is used in the map segment
 *              command
 * Error:       Checked Runtime error if the size of the sequence is 232
 *              Checked Runtime if mem is NULL
 */
uint32_t new_segment(uint32_t size, Memory_T mem)
{
        assert(mem != NULL);

        uint32_t seg_id;

        if (Stack_empty(mem->unmap_mem)) { /* if the stack is empty */

                /* checks if we have run out of memory */ /* check with TA */
                uint32_t segments_stored = Seq_length(mem->main_mem);
                assert(segments_stored != (uint32_t)(~0));
        
                /* each element is initialize to 0 */
                UArray_T new_segment = UArray_new(size, word_size);
                Seq_addhi(mem->main_mem, new_segment);
                seg_id = segments_stored; 
                
        } else { /* if the stack is not empty */

                /* get the top id on the stack and replace the segment at that 
                   id with a new segmenet */
                seg_id = (uint64_t)Stack_pop(mem->unmap_mem);
                UArray_T new_segment = UArray_new(size, word_size);
                UArray_T remove_seg = Seq_put(mem->main_mem, seg_id, 
                                              new_segment);
                
                /* recycles the old segment */
                UArray_free(&remove_seg);
        }

        return seg_id;
}


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
void remove_segment(uint32_t seg_id, Memory_T mem)
{
        assert(mem != NULL);

        /* checks if the ID is valid */
        assert(seg_id <= (uint32_t)Seq_length(mem->main_mem));

        /* casting allows the stack to interpret the ID as a void pointer */
        Stack_push(mem->unmap_mem, (void *)(uint64_t)seg_id);
}


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
void load_program(uint32_t seg_id, uint32_t offset, Memory_T mem)
{
        assert(mem != NULL);
        
        if (seg_id != 0) {
                /* replace segment 0 with the requested segment */
                UArray_T new_prog = Seq_get(mem->main_mem, seg_id);
                int new_prog_len = UArray_length(new_prog);
                UArray_T new_prog_copy = UArray_copy(new_prog, new_prog_len);

                UArray_T remove_seg = Seq_put(mem->main_mem, 0, new_prog_copy);

                /* recycles the old segment and remove the replaced segment */
                UArray_free(&remove_seg); 
        }
        
        /* update the program pointer */
        mem->program_ptr = word_at(0, offset, mem);
}


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
uint32_t *word_at(uint32_t seg_id, uint32_t word_index, Memory_T mem)
{
        assert(mem != NULL);
        
        /* get the segment that stores the desired word */
        UArray_T segment = Seq_get(mem->main_mem, seg_id);

        /* return the pointer to the element in the UArray */
        return UArray_at(segment, word_index);
}


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
void initialize_program_ptr(Memory_T mem)
{
        assert(mem != NULL);
        
        mem->program_ptr = word_at(0, 0, mem);
}

 
/* FUNCTION:    get_next_instruction
 * Purpose:     returns the next instruction relative to the current program 
 *              counter
 * Arg:         mem: struct that contains the components of the memory 
 *              management unit
 * Returns:     an uint32_t instruction word
 * Effect:      moves the program counter to the next word in segment 0
 *      	If the next word does not exist, returns 0
 * Exported to:	Operation module: used to get the next instruction for 
 *              operations
 * Error:       Checked Runtime if mem is NULL
 */
uint32_t get_next_instruction(Memory_T mem)
{
        assert(mem != NULL);
        
        /* dereference the program pointer and get the current instruction */
        uint32_t instruction = *(mem->program_ptr);
        
        /* use pointer arithmetic to increment the program pointer */
        (mem->program_ptr)++;

        return instruction;
}