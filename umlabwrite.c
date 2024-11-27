#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "fmt.h"
#include "seq.h"

extern void Um_write_sequence(FILE *output, Seq_T instructions);

extern void build_halt_test         (Seq_T stream);
extern void build_verbose_halt_test (Seq_T stream);
extern void build_add_test          (Seq_T stream);
extern void print_digit_test        (Seq_T stream);
extern void build_loadval_test      (Seq_T stream);
extern void build_input_test        (Seq_T stream);
extern void build_output_test       (Seq_T stream);
extern void build_input_eof_test    (Seq_T stream);
extern void print_multiple_add_test (Seq_T stream);
extern void print_input_add_test    (Seq_T stream);
extern void mod_mul_test            (Seq_T stream);
extern void build_multi_test        (Seq_T stream);
extern void print_multiple_mul_test (Seq_T stream);
extern void print_mul_0_test        (Seq_T stream);
extern void build_div_test          (Seq_T stream);
extern void print_multiple_div_test (Seq_T stream);
extern void print_divide_by_1_test  (Seq_T stream);
extern void divide_by_nonfactor     (Seq_T stream);
extern void build_cond_move_true    (Seq_T stream);
extern void build_cond_move_false   (Seq_T stream);
extern void build_nand_neg1_neg1    (Seq_T stream);
extern void build_nand_0_0          (Seq_T stream);
extern void build_map_seg_0words    (Seq_T stream);
extern void build_map_seg_large     (Seq_T stream);
extern void build_map_seg_many      (Seq_T stream);
extern void build_unmap_seg         (Seq_T stream);
extern void build_unmap_seg_many    (Seq_T stream);
extern void build_unmap_seg_alt     (Seq_T stream);
extern void build_seg_store         (Seq_T stream);
extern void build_seg_load          (Seq_T stream);
extern void build_seg_storeload     (Seq_T stream);
extern void build_prog_load         (Seq_T stream);
extern void run_500k_times          (Seq_T stream);



/* The array `tests` contains all unit tests for the lab. */

static struct test_info {
        const char *name;
        const char *test_input;          /* NULL means no input needed */
        const char *expected_output;
        /* writes instructions into sequence */
        void (*build_test)(Seq_T stream);
} tests[] = {
        { "halt",         NULL, "", build_halt_test },
        { "halt-verbose", NULL, "", build_verbose_halt_test },

        { "load_output", NULL, "test\n", build_output_test },

        { "input_output", "what\n", "what\n", build_input_test },
        { "input_eof", "2", "1", build_input_eof_test },
        
        { "add", NULL, "", build_add_test },
        { "print-six", NULL, "6", print_digit_test },
        { "multiple_add", NULL, "8", print_multiple_add_test },
        { "input_add", "*&", "P", print_input_add_test },

        { "multiply", NULL, "0", build_multi_test },
        { "multiply_many", NULL, "0", print_multiple_mul_test },
        { "multiply_by_0", "a", "2", print_mul_0_test },
        { "mod_mul", NULL, "8", mod_mul_test },

        { "divide", NULL, "2", build_div_test },
        { "divide_many", NULL, "2", print_multiple_div_test },
        { "divide_by_1", NULL, "4", print_divide_by_1_test },
        { "divide_by_nonfactor", "(", ">", divide_by_nonfactor },

        { "conditional_move_true", NULL, "2", build_cond_move_true },
        { "conditional_move_false", NULL, "0", build_cond_move_false },

        { "nand_0", NULL, "2", build_nand_neg1_neg1 },
        { "nand_1", NULL, "1", build_nand_0_0 },

        { "mapseg_size0", NULL, "2", build_map_seg_0words },
        { "mapseg_large", NULL, "2", build_map_seg_large },
        { "mapseg_many", NULL, "d", build_map_seg_many },

        { "unmapseg_size1", NULL, "2", build_unmap_seg },
        { "unmapseg_many", NULL, "2", build_unmap_seg_many },
        { "unmapseg_alt", NULL, "e", build_unmap_seg_alt },

        { "seg_store", NULL, "", build_seg_store },
        { "seg_load", NULL, "2", build_seg_load },
        { "seg_storeload", NULL, "2", build_seg_storeload },
        { "load_prog", NULL, "2", build_prog_load },

        { "run_500k", NULL, "", run_500k_times },
};

  
#define NTESTS (sizeof(tests)/sizeof(tests[0]))

/*
 * open file 'path' for writing, then free the pathname;
 * if anything fails, checked runtime error
 */
static FILE *open_and_free_pathname(char *path);

/*
 * if contents is NULL or empty, remove the given 'path',
 * otherwise write 'contents' into 'path'.  Either way, free 'path'.
 */
static void write_or_remove_file(char *path, const char *contents);

static void write_test_files(struct test_info *test);


int main (int argc, char *argv[])
{
        bool failed = false;
        if (argc == 1)
                for (unsigned i = 0; i < NTESTS; i++) {
                        printf("***** Writing test '%s'.\n", tests[i].name);
                        write_test_files(&tests[i]);
                }
        else
                for (int j = 1; j < argc; j++) {
                        bool tested = false;
                        for (unsigned i = 0; i < NTESTS; i++)
                                if (!strcmp(tests[i].name, argv[j])) {
                                        tested = true;
                                        write_test_files(&tests[i]);
                                }
                        if (!tested) {
                                failed = true;
                                fprintf(stderr,
                                        "***** No test named %s *****\n",
                                        argv[j]);
                        }
                }
        return failed; /* failed nonzero == exit nonzero == failure */
}


static void write_test_files(struct test_info *test)
{
        FILE *binary = open_and_free_pathname(Fmt_string("%s.um", test->name));
        Seq_T instructions = Seq_new(0);
        test->build_test(instructions);
        Um_write_sequence(binary, instructions);
        Seq_free(&instructions);
        fclose(binary);

        write_or_remove_file(Fmt_string("%s.0", test->name),
                             test->test_input);
        write_or_remove_file(Fmt_string("%s.1", test->name),
                             test->expected_output);
}


static void write_or_remove_file(char *path, const char *contents)
{
        if (contents == NULL || *contents == '\0') {
                remove(path);
        } else {
                FILE *input = fopen(path, "wb");
                assert(input != NULL);

                fputs(contents, input);
                fclose(input);
        }
        free(path);
}


static FILE *open_and_free_pathname(char *path)
{
        FILE *fp = fopen(path, "wb");
        assert(fp != NULL);

        free(path);
        return fp;
}
