/*****************************************************************************
 *
 *                                 um_main.c
 *
 *     Assignment: UM
 *     Authors:    Eric Zhao, Leo Kim
 *     Date:       November 21, 2022
 *
 *     Summary: This is the main function for our UM program. This program reads
 *     in a binary file of UM instructions and executes them using functions
 *     from our operations module.
 * 
 *
 ****************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>
#include "operations.h"

int main (int argc, char *argv[]) 
{
        /* if a filename is provided open it for reading in and if not read 
           in from stdin */
        FILE *input;
        char *file_name = argv[1];

        if (argc == 2) {
                input = fopen(file_name, "r");
                if (input == NULL) {
                        fprintf(stderr, 
                               "Provided file cannot be opened for reading\n");
                        exit(EXIT_FAILURE);
                }
        } else {
                fprintf(stderr, "Incorrect number of arguments provided\n");
                exit(EXIT_FAILURE);
        }

        /* declare an operations struct */
        Operations_T operations = Operations_new();

        /* get the number of words from the file metadata */
        struct stat meta_data;
        stat(file_name, &meta_data);
        unsigned num_words = meta_data.st_size / 4;
        
        /* read in the program from the provided file */
        read_in_program(input, num_words, operations);
        fclose(input);
        
        /* loop that reads an insruction from segment 0 and then runs it. Runs
           until it reaches a HALT instruction */
        uint32_t instruction;
        do {
                instruction = next_instruction(operations);
        } while (do_instruction(instruction, operations));

        /* free memory */
        Operations_free(&operations);

        return EXIT_SUCCESS;
}