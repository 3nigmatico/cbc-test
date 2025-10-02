#ifndef __e__ran__h__
#define __e__ran__h__

#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

#define INITWORDS "FUCKYOU"
#define INITLENGTH 7
#define HASHLEN 32

uint64_t ranps[20] = { 244150639, 232500001, 235454983, 3543700967,
    6751700497, 6751703393, 7380101171, 7380104987,
    4199804417, 4199801041, 180301027, 160251017,
    6847050869, 169400683, 185001329, 539800477,
    352254437, 808550593, 3297300481, 7397651399 };

uint8_t S[32][32];
uint64_t X = 0;
uint64_t M = 0;
char init = 0;

void ran_update(uint8_t *phash, uint64_t hash_len, uint8_t *psalt, size_t saltlen)
{
    M = X = 0;
    // Populate the estate matrix
    for(int i = 0; i < 32; i++)
    {
        for(int j = 0; j < 32; j++)
        {
            if(!psalt)
                S[i][j] = phash[(i+j)%hash_len] ^ INITWORDS[j%INITLENGTH];
            else
                S[i][j] = phash[(i+j)%hash_len] ^ psalt[(i+j)%saltlen] ^ INITWORDS[j%INITLENGTH];
        }
    }

    // Sum the first row
    for(int j = 0; j < 32; j++)
        M += S[0][j];

    for(int n = 0; n < 4; n++)
    {
        // Scramble things a little according to the sum
        for(int i = 0; i < 32; i++)
        {
            if((M+i) % 2 == 0)
            {
                matrix_row_shift_right(S[i], 32, i);
                matrix_colum_reverse(S, 31-i, 32);
            }
            if((M+i) % 3 == 0)
            {
                matrix_row_shift_left(S[i], 32, i);
                matrix_colum_reverse(S, 31-i, 32);
            }
            if((M+i) % 5 == 0)
            {
                matrix_row_shift_right(S[i], 32, 1);
                matrix_colum_reverse(S, 31-i, 32);
            }
            if((M+i) % 7 == 0)
            {
                matrix_row_shift_left(S[i], 32, 1);
                matrix_colum_reverse(S, 31-i, 32);
            }
            if((M*i) % 10 == 0)
            {
                matrix_colum_shift_left(S, i, 32, ((i*2)%30)+1);
                matrix_row_reverse(S[31-i], 32);
            }
            if((M*i) % 21 == 0)
            {
                matrix_colum_shift_right(S, i, 32, ((i*2)%30)+1);
                matrix_row_reverse(S[31-i], 32);
            }
        }

        // Adds the 8 lsb bits of the accumulator to each cell
        for(int i = 0; i < 32; i++)
            for(int j = 0; j < 32; j++)
                S[i][j] += M&0xFF;
    }

    // Initialize the accumulator
    for(int i = 0; i < 32; i++)
    {
        M = (M ^ (M<<1))|(S[i][i]);
    }
    X += 232500001 * matrix_trace(S,32);
    return;
}

void ran_initialize(uint8_t *phash, uint64_t hash_len)
{
    if(init)
        return;

    ran_update(phash, hash_len, 0L, 0);

    init = 1;

    return;
}

uint64_t ran_rand_next()
{
    if(!init)
        return 0;

    M += 7397651399 * (~X);
    X += X + ranps[X%20] * M;
    return M;
}

uint32_t ran_rand32_next()
{
    if(!init)
        return 0;

    M += 7397651399 * (~X);
    X += X + ranps[X%20] * M;
    return M&0x0FFFFFFFF;
}

uint16_t ran_rand16_next()
{
    if(!init)
        return 0;

    M += 7397651399 * (~X);
    X += X + ranps[X%20] * M;
    return M&0x0FFFF;
}

uint8_t ran_rand8_next()
{
    if(!init)
        return 0;

    M += 7397651399 * (~X);
    X += X + ranps[X%20] * M;
    return M%0xFF;
}

#endif
