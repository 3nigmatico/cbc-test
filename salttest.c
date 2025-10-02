#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <argon2.h>
#include <string.h>
//#include "esalt.h"
#include "ran.h"
#define SALTLEN 16
//#define HASHLEN 32

#define NUMGEN 1000

uint8_t vals[255] = {0};
uint8_t seq[NUMGEN];

uint8_t hash[HASHLEN];
uint8_t SALT[SALTLEN] = {0xF0};

void printmatrix(uint8_t *matrix, int cols, int size)
{
    printf("         ");
    for(int i = 0; i < cols; i++)
        printf("(%02x)", i);
    printf("\n(00)     ");
    for(int i = 0; i < size; i++)
    {
        printf("[%02x]", matrix[i]);
        if((i+1)%cols == 0 && i != 0)
            printf("\n(%02x)     ",i+1);
    }
    printf("\n");
    return;
}


void hash_password(const char* password)
{
    uint8_t *pwd = (uint8_t*)strdup(password);
    size_t pwdlen = strlen((char*)pwd);
    argon2id_hash_raw(32, (1<<16), 16, pwd, pwdlen, SALT, SALTLEN, hash, HASHLEN);
    free(pwd);
    return;
}

int main()
{
    uint8_t c = 0;
    //esalt_init("PASSWORD123");
    hash_password("PASSWORD");
    ran_initialize(hash, HASHLEN);

    for(int i = 0; i < 8; i++)
        printf("%u ", ran_rand_next());
    printf("\n");

    for(int i = 0; i < NUMGEN; i++)
    {
        //c = esalt_nextchar();
        c = ran_rand8_next();
        seq[i] = c;
        ++vals[c];
    }

    printmatrix(vals, 30, 255);
    printmatrix(seq, 30, NUMGEN);

    return 0;
}
