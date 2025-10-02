#include <stdio.h> // print
#include <stdlib.h> // calloc, malloc, free...
#include <stdint.h> // uints
#include <string.h> // memcpy, memmove, str functions

#ifndef E_MATRIX_H_
#define E_MATRIX_H_

/* Definitions here */
void matrix_row_shift_right(uint8_t *row, size_t rowlength, size_t shift);
void matrix_row_shift_left(uint8_t *row, size_t rowlength, size_t shift);
void matrix_row_reverse(uint8_t *row, size_t rowlength);
void matrix_colum_shift_left(uint8_t m[][32], size_t col, size_t collength, size_t shift);
void matrix_colum_shift_right(uint8_t m[][32], size_t col, size_t collength, size_t shift);
void matrix_colum_reverse(uint8_t m[][32], size_t col, size_t collength);
uint64_t matrix_trace(uint8_t m[][32], size_t size);

#endif

/* Circular shift to the right */
void matrix_row_shift_right(uint8_t *row, size_t rowlength, size_t shift)
{
    size_t i = 0, pos = 0;
    // If the shift is 0 or a product of rowlength, there is no shift
    if(shift == 0 || shift%rowlength == 0 || rowlength < 2){return;}
    uint8_t *tmp = (uint8_t*)calloc(1, rowlength);
    // Calculate the right shift, in O(n)
    for(i = 0; i < rowlength; i++)
    {
            // The new position of the i-th element
            pos = (i + shift)%rowlength;
            *(uint8_t*)(tmp+pos) = *(uint8_t*)(row+i);
    }
    memcpy(row, tmp, rowlength);
    free(tmp);
    return;
}

/* Circular shift to the left */
void matrix_row_shift_left(uint8_t *row, size_t rowlength, size_t shift)
{
    size_t i = 0, pos = 0, pos_shift = 0;
    // If the shift is 0 or a product of rowlength, there is no shift
    if(shift == 0 || shift%rowlength == 0 || rowlength < 2){return;}
    uint8_t *tmp = (uint8_t*)calloc(1, rowlength);
    // Calculate the RIGHT shift of the first element
    pos_shift = (rowlength-(shift%rowlength));
    // Shift the row RIGHT pos_shift times in O(n)
    for(i = 0; i < rowlength; i++)
    {
        // The new position of the i-th element
        pos = (i + pos_shift)%rowlength;
        *(uint8_t*)(tmp+pos) = *(uint8_t*)(row+i);
    }
    memcpy(row, tmp, rowlength);
    free(tmp);
    return;
}

/* Invert the row */
void matrix_row_reverse(uint8_t *row, size_t rowlength)
{
    size_t i = 0, pos = 0;
    if(rowlength < 2){return;}
    uint8_t *tmp = (uint8_t*)calloc(1, rowlength);
    for(i = 0; i < rowlength; i++)
    {
        *(uint8_t*)(tmp+i) = *(uint8_t*)(row+(rowlength-1-i));
    }
    memcpy(row, tmp, rowlength);
    free(tmp);
    return;
}

void matrix_colum_shift_left(uint8_t m[][32], size_t col, size_t collength, size_t shift)
{
    uint8_t* col_row = (uint8_t*)calloc(collength, 1);
    for(int i = 0; i < collength; i++)
    {
        col_row[i] = m[i][col];
    }
    matrix_row_shift_left(col_row, collength, shift);
    for(int i = 0; i < collength; i++)
    {
        m[i][col] = col_row[i];
    }
    free(col_row);
    return;
}

void matrix_colum_shift_right(uint8_t m[][32], size_t col, size_t collength, size_t shift)
{
    uint8_t* col_row = (uint8_t*)calloc(collength, 1);
    for(int i = 0; i < collength; i++)
    {
        col_row[i] = m[i][col];
    }
    matrix_row_shift_right(col_row, collength, shift);
    for(int i = 0; i < collength; i++)
    {
        m[i][col] = col_row[i];
    }
    free(col_row);
    return;
}

void matrix_colum_reverse(uint8_t m[][32], size_t col, size_t collength)
{
    uint8_t* col_row = (uint8_t*)calloc(collength, 1);
    for(int i = 0; i < collength; i++)
    {
        col_row[i] = m[i][col];
    }
    matrix_row_reverse(col_row, collength);
    for(int i = 0; i < collength; i++)
    {
        m[i][col] = col_row[i];
    }
    free(col_row);
    return;
}

uint64_t matrix_trace(uint8_t m[][32], size_t size)
{
    uint64_t tr = 0;
    for(size_t i = 0; i < size; i++)
        tr += m[i][i];
    return tr;
}
