/*****************************************************************************
 *
 *                                  bitpack.c
 *
 *     Authors:    Eric Zhao, Leo Kim
 *     Date:       October 26, 2022
 *
 *     Summary:
 *     This is the private implementation of our bitpack module. This module
 *     allows the user to manipulate 64-bit codeword with a variety of
 *     different functions that add and extract data from requested fields in
 *     the codeword.
 * 
 *
 ****************************************************************************/
#include <bitpack.h>
#include <stdio.h>
#include <assert.h>
#include "except.h"

Except_T Bitpack_Overflow = { "Overflow packing bits" };

/*
 * private helper functions, functions details are included in the function
 * contracts respectively
 */
uint64_t        new_extraction_mask(unsigned width, unsigned lsb);
uint64_t        clear_field(uint64_t word, unsigned width, unsigned lsb);
static uint64_t u_rshift(uint64_t word, unsigned shift);
static int64_t  s_rshift(int64_t word, unsigned shift);
static uint64_t lshift(uint64_t word, unsigned shift);
                          
/* bits size of the type uint64_t and int64_t */
#define WORD_LENG 64

/* FUNCTION:  Bitpack_fitsu
 * Purpose:   Tells whether the argument n can be represented in width bits
 * Arg:       n: a uint64_t type integer
 *            width: the number of bits 
 * Returns:   Boolean indicating whether the argument n can be represented 
 *            in width bits
 * Effect:    N/A
 * Error:     Runtime error if width is larger than 64 bits
 */
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
        assert(width <= WORD_LENG);

        if (width == 0) {
                return false;
        }
        if (width == WORD_LENG) {
                return true;
        }
        
        /* compute the bounds in decimal values */
        uint64_t limit = 1;
        limit = limit << width;
        
        return limit > n;
}

/* FUNCTION:  Bitpack_fitss
 * Purpose:   Tells whether the argument n can be represented in width bits
 * Arg:       n: a int64_t type integer
 *            width: the number of bits 
 * Returns:   Boolean indicating whether the argument n can be represented 
 *            in width bits
 * Effect:    N/A
 * Error:     Runtime error if width is larger than 64 bits
 */
bool Bitpack_fitss(int64_t n, unsigned width)
{
        assert(width <= WORD_LENG);

        if (width == 0) {
                return false;
        }
        if (width == WORD_LENG) {
                return true;
        }
        
        /* compute the bounds in decimal values */
        int64_t pos_limit = 1;
        pos_limit = (pos_limit << (width - 1)) - 1;
        int64_t neg_limit = ~pos_limit;
        
        return (pos_limit >= n && n >= neg_limit);
}

/* FUNCTION:  Bitpack_getu
 * Purpose:   extracts a field from a word given the width of the field and 
 *            the location of the field’s least significant bit
 * Arg:       word: a uint64_t type integer
 *            width: the bit length of the desired fields
 *            lsb: the lsb-th bit from which the desired bits starts
 * Returns:   a uint64_t type integer that is requested by the caller
 * Effect:    Note that values extracted can be represented as signed or 
 *            unsigned, but by programming convention we use only unsigned
 *            types to represent words
 * Error:     Runtime error for width + lsb to be larger than 64 
 */
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        assert(width + lsb <= WORD_LENG);
        
        /* make a word that only has width number of 1s from the lsb */
        uint64_t mask = new_extraction_mask(width, lsb);

        /* use & opertaion to extract the desired bit combinations */
        uint64_t new = word & mask;

        /* shift the bits back and return it */
        new = u_rshift(new, lsb);
        return new;
}

/* FUNCTION:  Bitpack_gets
 * Purpose:   extracts a field from a word given the width of the field and 
 *            the location of the field’s least significant bit
 * Arg:       word: a uint64_t type integer
 *            width: the bit length of the desired fields
 *            lsb: the lsb-th bit from which the desired bits starts
 * Returns:   a int64_t type integer that is requested by the caller
 * Effect:    Note that values extracted can be represented as be signed or 
 *            unsigned, but by programming convention we use only unsigned
 *            types to represent words
 * Error:     Runtime error for width + lsb to be larger than 64 
 */
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
        int64_t new = Bitpack_getu(word, width, lsb);
        static uint64_t WORDLEN = WORD_LENG;

        /* shift the bits to propagate the first bit in the desired field */
        new = lshift(new, WORDLEN - width);
        new = s_rshift(new, WORDLEN - width);

        return new;
}


/* FUNCTION:  Bitpack_newu
 * Purpose:   Return a new codeword with the requested field containing the 
 *            given value
 * Arg:       word: the codeword
 *            width: the width of the field being added
 *            lsb: the lsb of the field being added
 *            value: the value being added
 * Returns:   The codeword with the new added value
 * Effect:    N/A
 * Error:     Runtime error for width + lsb to be larger than WORD_LEN 
 */
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, 
                      uint64_t value)
{
        assert(width + lsb <= WORD_LENG);

        if (!Bitpack_fitsu(value, width)) {
                RAISE(Bitpack_Overflow);
        }

        word = clear_field(word, width, lsb);

        /* shift the value until it corresponds to the desired field */
        value = lshift(value, lsb);
        
        return word | value;               
}

/* FUNCTION:  Bitpack_news
 * Purpose:   Return a new codeword with the requested field containing the 
 *            given value
 * Arg:       word: the codeword
 *            width: the width of the field being added
 *            lsb: the lsb of the field being added
 *            value: the value being added
 * Returns:   The codeword with the new added value
 * Effect:    N/A
 * Error:     Runtime error for width + lsb to be larger than 64 
 */
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb, 
                      int64_t value)
{
        assert(width + lsb <= WORD_LENG);

        if (!Bitpack_fitss(value, width)) {
                RAISE(Bitpack_Overflow);
        }

        word = clear_field(word, width, lsb);
        
        /* shift to dump any data past the desired width */
        value = lshift(value, WORD_LENG - width);
        value = u_rshift(value, WORD_LENG - width);

        /* shift the value until it corresponds to the desired field */
        value = lshift(value, lsb);
        
        return word | value;               
}

/* FUNCTION:  u_rshift
 * Purpose:   Does right shift operation on a unsigned (non-negative) integer
 * Arg:       word: a uint64_t type integer
 *            shift: the shift amount
 * Returns:   a uint64_t type integer after being shifted
 * Effect:    N/A
 * Error:     Runtime error if the shift amount is larger than 64
 */
static uint64_t u_rshift(uint64_t word, unsigned shift)
{
        assert(shift <= WORD_LENG);

        if (shift == WORD_LENG) {
                return (uint64_t)0;
        }
        
        return word >> shift;
}

/* FUNCTION:  lshift
 * Purpose:   Does left shift operation on both a unsigned or a signed integer
 * Arg:       word: a uint64_t type integer
 *            shift: the shift amount
 * Returns:   a uint64_t type integer after being shifted
 * Effect:    Note that the function is compatible with a int64_t word too
 * Error:     Runtime error if the shift amount is larger than 64
 */
static uint64_t lshift(uint64_t word, unsigned shift)
{
        assert(shift <= WORD_LENG);
        
        if (shift == WORD_LENG) {
                return (uint64_t)0;
        }
        
        return word << shift;
}

/* FUNCTION:  s_rshift
 * Purpose:   Does right shift operation on a signed (both neg and pos) integer
 * Arg:       word: a int64_t type integer
 *            shift: the shift amount
 * Returns:   a int64_t type integer after being shifted
 * Effect:    N/A
 * Error:     Runtime error if the shift amount is larger than 64
 */
static int64_t s_rshift(int64_t word, unsigned shift)
{
        assert(shift <= WORD_LENG);
        
        if (shift == WORD_LENG) {
                if (word < 0) {
                        return (int64_t)-1;
                } else {
                        return (int64_t)0;
                }
        }
        return word >> shift;
}

/* FUNCTION:  new_extraction_mask
 * Purpose:   Create a mask with 1s in the desired field and 0s everywhere
 *            else
 * Arg:       width: the width of the desired field
 *            lsb: the lsb of the desired field
 * Returns:   The 64-bit mask
 * Effect:    N/A
 * Error:     Fails if the given signed value does not fit in the requested
 *            field
 */
uint64_t new_extraction_mask(unsigned width, unsigned lsb)
{
        uint64_t mask = 0;

        /* shift the mask to get rid of undesired 1s */
        mask = ~mask;
        mask = u_rshift(mask, WORD_LENG - width);
        mask = lshift(mask, lsb);
        
        return mask;
}


/* FUNCTION:  clear_field
 * Purpose:   Replace the requested field in a codeword with 0s
 * Arg:       word: the codeword
 *            width: the width of the field being cleared
 *            lsb: the lsb of the field being cleared
 * Returns:   The codeword with the newly cleared field
 * Effect:    N/A
 * Error:     N/A
 */
uint64_t clear_field(uint64_t word, unsigned width, unsigned lsb)
{
        /* a mask that has 0s in the desired fields and 1s elsewhere */
        uint64_t mask = ~new_extraction_mask(width, lsb);
        
        return word & mask;
        
}