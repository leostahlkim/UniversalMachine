/*
 * umlab.c
 *
 * Functions to generate UM unit tests. Once complete, this module
 * should be augmented and then linked against umlabwrite.c to produce
 * a unit test writing program.
 *  
 * A unit test is a stream of UM instructions, represented as a Hanson
 * Seq_T of 32-bit words adhering to the UM's instruction format.  
 * 
 * Any additional functions and unit tests written for the lab go
 * here. 
 *  
 */


#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <seq.h>
#include <bitpack.h>


typedef uint32_t Um_instruction;
typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;


/* Functions that return the two instruction types */

Um_instruction three_register(Um_opcode op, int ra, int rb, int rc);
Um_instruction loadval(unsigned ra, unsigned val);


/* Wrapper functions for each of the instructions */

static inline Um_instruction halt(void) 
{
        return three_register(HALT, 0, 0, 0);
}

typedef enum Um_register { r0 = 0, r1, r2, r3, r4, r5, r6, r7 } Um_register;

static inline Um_instruction add(Um_register a, Um_register b, Um_register c) 
{
        return three_register(ADD, a, b, c);
}

static inline Um_instruction multiply(Um_register a, Um_register b, Um_register c)
{
        return three_register(MUL, a, b, c);
}

static inline Um_instruction divide(Um_register a, Um_register b, Um_register c) 
{
        return three_register(DIV, a, b, c);
}

static inline Um_instruction cond_move(Um_register a, Um_register b, Um_register c) 
{
        return three_register(CMOV, a, b, c);
}

Um_instruction output(Um_register c)
{
        return three_register(OUT, 0, 0, c);
}

Um_instruction input(Um_register c)
{
        return three_register(IN, 0, 0, c);
}

Um_instruction nand(Um_register a, Um_register b, Um_register c)
{
        return three_register(NAND, a, b, c);
}

Um_instruction map_seg(Um_register b, Um_register c)
{
        return three_register(ACTIVATE, 0, b, c);
}

Um_instruction unmap_seg(Um_register c)
{
        return three_register(INACTIVATE, 0, 0, c);
}

Um_instruction seg_store(Um_register a, Um_register b, Um_register c)
{
        return three_register(SSTORE, a, b, c);
}

Um_instruction seg_load(Um_register a, Um_register b, Um_register c)
{
        return three_register(SLOAD, a, b, c);
}

Um_instruction load_prog(Um_register b, Um_register c)
{
        return three_register(LOADP, 0, b, c);
}

/* Functions for working with streams */

static inline void append(Seq_T stream, Um_instruction inst)
{
        assert(sizeof(inst) <= sizeof(uintptr_t));
        Seq_addhi(stream, (void *)(uintptr_t)inst);
}

const uint32_t Um_word_width = 32;

void Um_write_sequence(FILE *output, Seq_T stream)
{
        assert(output != NULL && stream != NULL);
        int stream_length = Seq_length(stream);
        for (int i = 0; i < stream_length; i++) {
                Um_instruction inst = (uintptr_t)Seq_remlo(stream);
                for (int lsb = Um_word_width - 8; lsb >= 0; lsb -= 8) {
                        fputc(Bitpack_getu(inst, 8, lsb), output);
                }
        }
      
}


/* Unit tests for the UM */

Um_instruction three_register(Um_opcode op, int ra, int rb, int rc)
{
        Um_instruction instr = 0;
        
        instr = Bitpack_newu(instr, 4, 28, op);
        instr = Bitpack_newu(instr, 3, 0, rc);
        instr = Bitpack_newu(instr, 3, 3, rb);
        instr = Bitpack_newu(instr, 3, 6, ra);

        return instr;
}

Um_instruction loadval(unsigned ra, unsigned val)
{
        Um_instruction instr = 0;

        instr = Bitpack_newu(instr, 4, 28, LV);
        instr = Bitpack_newu(instr, 3, 25, ra);
        instr = Bitpack_newu(instr, 25, 0, val);
        
        return instr;
}

void build_halt_test(Seq_T stream)
{
        append(stream, halt());
}

void build_verbose_halt_test(Seq_T stream)
{
        append(stream, halt());
        append(stream, loadval(r1, 'B'));
        append(stream, output(r1));
        append(stream, loadval(r1, 'a'));
        append(stream, output(r1));
        append(stream, loadval(r1, 'd'));
        append(stream, output(r1));
        append(stream, loadval(r1, '!'));
        append(stream, output(r1));
        append(stream, loadval(r1, '\n'));
        append(stream, output(r1));
}

void build_output_test(Seq_T stream)
{
        append(stream, loadval(r0, 't'));
        append(stream, output(r0));
        
        append(stream, loadval(r3, 'e'));
        append(stream, output(r3));
        
        append(stream, loadval(r4, 's'));
        append(stream, output(r4));
        
        append(stream, loadval(r5, 't'));
        append(stream, output(r5));
        
        append(stream, loadval(r7, '\n'));
        append(stream, output(r7));

        append(stream, halt());
}

void build_input_test(Seq_T stream)
{
        append(stream, input(r5));
        append(stream, output(r5));
        
        append(stream, input(r3));
        append(stream, output(r3));

        append(stream, input(r7));
        append(stream, output(r7));

        append(stream, input(r7));
        append(stream, output(r7));

        append(stream, loadval(r7, '\n'));
        append(stream, output(r7));

        append(stream, halt());
}

void build_input_eof_test(Seq_T stream)
{
        append(stream, input(r1));
        append(stream, input(r2));
        
        append(stream, add(r2, r2, r1));

        append(stream, output(r2));

        append(stream, halt());
}


void build_add_test(Seq_T stream)
{
        append(stream, add(r1, r2, r3));
        append(stream, halt());
}

void print_digit_test(Seq_T stream)
{
        append(stream, loadval(r1, 48));
        append(stream, loadval(r2, 6));
        append(stream, add(r3, r1, r2));
        append(stream, output(r3));
        append(stream, halt());
}

void print_multiple_add_test(Seq_T stream)
{
        append(stream, loadval(r1, 48));
        append(stream, loadval(r2, 6));
        append(stream, add(r3, r1, r2));
        append(stream, loadval(r4, 2));
        append(stream, add(r3, r3, r4));
        append(stream, output(r3));
        append(stream, halt());
}

void print_input_add_test(Seq_T stream)
{
        append(stream, input(r1));
        append(stream, input(r2));
        append(stream, add(r3, r1, r2));
        append(stream, output(r3));

        append(stream, halt());
}

void build_multi_test(Seq_T stream)
{
        append(stream, loadval(r3, 12));
        append(stream, loadval(r2, 4));
        append(stream, multiply(r1, r2, r3));
        append(stream, output(r1));
        append(stream, halt());
}

void print_multiple_mul_test(Seq_T stream)
{
        append(stream, loadval(r1, 6));
        append(stream, loadval(r2, 2));
        append(stream, multiply(r3, r1, r2));
        append(stream, loadval(r4, 4));
        append(stream, multiply(r3, r3, r4));
        append(stream, output(r3));
        append(stream, halt());
}

void print_mul_0_test(Seq_T stream)
{
        append(stream, loadval(r2, 0));
        append(stream, input(r3));

        append(stream, multiply(r1, r2, r3));

        append(stream, loadval(r2, 50));
        append(stream, add(r1, r1, r2));
        append(stream, output(r1));

        append(stream, halt());
}

void mod_mul_test(Seq_T stream)
{
        append(stream, loadval(r1, (358332)));
        append(stream, loadval(r2, (11986)));

        append(stream, multiply(r3, r1, r2));
        append(stream, output(r3));

        append(stream, halt());
}


void build_div_test(Seq_T stream)
{
        append(stream, loadval(r3, 4));
        append(stream, loadval(r2, 200));
        append(stream, divide(r1, r2, r3));
        append(stream, output(r1));
        append(stream, halt());
}

void print_multiple_div_test(Seq_T stream)
{
        append(stream, loadval(r1, 20000));
        append(stream, loadval(r2, 100));
        append(stream, divide(r3, r1, r2));
        append(stream, loadval(r4, 4));
        append(stream, divide(r3, r3, r4));
        append(stream, output(r3));
        append(stream, halt());
}

void print_divide_by_1_test(Seq_T stream)
{
        append(stream, loadval(r1, 52));
        append(stream, loadval(r2, 1));

        append(stream, divide(r3, r1, r2));
        append(stream, output(r3));

        append(stream, halt());
}


void divide_by_nonfactor(Seq_T stream)
{
        append(stream, loadval(r2, 2500));
        append(stream, input(r3));

        append(stream, divide(r1, r2, r3));

        append(stream, output(r1));

        append(stream, halt());
}

void build_cond_move_true(Seq_T stream)
{
        append(stream, loadval(r1, 1213));
        append(stream, loadval(r2, 50));
        append(stream, loadval(r3, 48));

        append(stream, cond_move(r3, r2, r1));

        append(stream, output(r3));

        append(stream, halt());
}

void build_cond_move_false(Seq_T stream)
{
        append(stream, loadval(r1, 0));
        append(stream, loadval(r2, 50));
        append(stream, loadval(r3, 48));

        append(stream, cond_move(r3, r2, r1));

        append(stream, output(r3));

        append(stream, halt());
}

void build_nand_neg1_neg1(Seq_T stream)
{
        append(stream, loadval(r1, 0));
        append(stream, loadval(r2, 0));
        append(stream, loadval(r3, 50));
        append(stream, loadval(r4, 432));


        append(stream, nand(r3, r1, r2));
        append(stream, nand(r4, r1, r2));
        append(stream, nand(r5, r3, r4));

        append(stream, loadval(r2, 50));
        append(stream, add(r5, r5, r2));
        append(stream, output(r5));

        append(stream, halt());
}

void build_nand_0_0(Seq_T stream)
{
        append(stream, loadval(r1, 0));
        append(stream, loadval(r2, 0));
        append(stream, loadval(r3, 23));

        append(stream, nand(r3, r2, r1));

        append(stream, loadval(r2, 50));
        append(stream, add(r3, r3, r2));
        append(stream, output(r3));

        append(stream, halt());
}

void build_map_seg_0words(Seq_T stream)
{
        append(stream, loadval(r1, 0));
        append(stream, loadval(r2, 123));

        append(stream, map_seg(r2, r1));
        
        append(stream, loadval(r3, 49));
        append(stream, add(r3, r3, r2));
        append(stream, output(r3));

        append(stream, halt());
}

void build_map_seg_large(Seq_T stream)
{
        append(stream, loadval(r1, 99999));
        append(stream, loadval(r2, 123));

        append(stream, map_seg(r2, r1));
        
        append(stream, loadval(r3, 49));
        append(stream, add(r3, r3, r2));
        append(stream, output(r3));

        append(stream, halt());
}

void build_map_seg_many(Seq_T stream)
{
        append(stream, loadval(r1, 300));
        append(stream, loadval(r2, 123));


        for (int i = 0; i < 100; i++) {
                append(stream, map_seg(r2, r1));
        }
        
        append(stream, output(r2));

        append(stream, halt());
}

void build_unmap_seg(Seq_T stream)
{
        append(stream, loadval(r1, 3));
        append(stream, loadval(r2, 123));
        append(stream, map_seg(r2, r1));
        append(stream, unmap_seg(r2));

        append(stream, map_seg(r2, r1));

        append(stream, loadval(r3, 49));
        append(stream, add(r3, r3, r2));
        append(stream, output(r3));

        append(stream, halt());
}

void build_unmap_seg_many(Seq_T stream)
{
        append(stream, loadval(r1, 3));
        append(stream, loadval(r2, 123));
        
        for (int i = 0; i < 200; i++) {
                append(stream, map_seg(r2, r1));
                append(stream, unmap_seg(r2));
        }
        
        append(stream, map_seg(r2, r1));

        append(stream, loadval(r3, 49));
        append(stream, add(r3, r3, r2));
        append(stream, output(r3));

        append(stream, halt());
}

void build_unmap_seg_alt(Seq_T stream)
{
        append(stream, loadval(r1, 4));
        append(stream, loadval(r2, 123));
        
        for (int i = 0; i < 100; i++) {
                append(stream, map_seg(r2, r1));
                append(stream, map_seg(r2, r1));
                append(stream, unmap_seg(r2));

        }
        
        append(stream, map_seg(r2, r1));

        append(stream, output(r2));

        append(stream, halt());
}


void build_seg_store(Seq_T stream)
{
        append(stream, loadval(r1, 10));
        append(stream, loadval(r3, 5));
                
        append(stream, map_seg(r2, r1));

        append(stream, loadval(r1, 50));
        append(stream, seg_store(r2, r3, r1));

        append(stream, halt());
}


void build_seg_load(Seq_T stream)
{
        append(stream, loadval(r1, 10));
        append(stream, loadval(r3, 5));

                
        append(stream, map_seg(r2, r1));

        append(stream, loadval(r1, 50));

        /* stores $m[r2][r3] := $r[1] */
        append(stream, seg_store(r2, r3, r1));

        /* $r[6] := $m[$r[2]][$r[3]] */
        append(stream, seg_load(r6, r2, r3));
        
        append(stream, output(r6));
        // append(stream, loadval(r6, 50));
        // append(stream, output(r6));


        append(stream, halt());
}

void build_seg_storeload(Seq_T stream)
{
        append(stream, loadval(r1, 0));
        append(stream, loadval(r3, 9));

        append(stream, loadval(r6, 50));
        append(stream, output(r6));

        append(stream, seg_load(r5, r1, r3));

        append(stream, loadval(r4, 7));
        append(stream, seg_store(r1, r4, r5));
        
        append(stream, loadval(r1, 51));
        append(stream, output(r6));

        append(stream, halt());
}

void build_prog_load(Seq_T stream)
{
        /* map a segment of size 10 */
        append(stream, loadval(r1, 10));
        append(stream, map_seg(r2, r1));

        /* index of the loadval command in our program */
        append(stream, loadval(r6, 21));

        /* index where we are adding to the new segment*/
        append(stream, loadval(r5, 0));

        /* load the final 3 lines of our prog into the new segment */
        /* r5 = 0, r6 = 21, r7 = mem[0][21]*/
        append(stream, seg_load(r7, r5, r6));
        /* r2 = index of new seg, r5 = 0, r7 = mem[0][21]*/
        append(stream, seg_store(r2, r5, r7));
        
        /* increment the indices */
        append(stream, loadval(r0, 1));
        append(stream, add(r3, r5, r0));
        append(stream, add(r6, r6, r0));

        append(stream, seg_load(r7, r5, r6));
        append(stream, seg_store(r2, r3, r7));

        append(stream, loadval(r0, 1));
        append(stream, add(r3, r3, r0));
        append(stream, add(r6, r6, r0));

        append(stream, seg_load(r7, r5, r6));
        append(stream, seg_store(r2, r3, r7));

        append(stream, loadval(r3, 0));
        append(stream, load_prog(r2, r3));

        append(stream, seg_store(r2, r5, r7));

        append(stream, loadval(r1, 55));
        append(stream, output(r1));
        
        append(stream, loadval(r1, 50));
        append(stream, output(r1));
        append(stream, halt());
}


void run_500k_times(Seq_T stream)
{
        /* seg 0 id */
        append(stream, loadval(r1, 0));

        append(stream, nand(r5, r1, r1));
        append(stream, loadval(r0, 4));


        /* counter */
        append(stream, loadval(r2, 50000));

        /* increment counter */
        append(stream, add(r2, r2, r5));

        /* test if 500k */
        append(stream, loadval(r7, 24553));
        append(stream, loadval(r7, 234));
        append(stream, loadval(r7, 2332));
        append(stream, loadval(r7, 32));
        append(stream, loadval(r7, 5));
        append(stream, loadval(r7, 3));
        append(stream, loadval(r7, 14));
        append(stream, cond_move(r7, r0, r2));

        /* if not, load prog back to line 4*/
        append(stream, load_prog(r1, r7));

        append(stream, halt());
}