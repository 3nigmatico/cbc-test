#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <argon2.h>
#include <sys/random.h>
#include "ran.h"

#ifndef DIV_CEIL
#define DIV_CEIL(p,q) ((p/q)+((p%q > 0)?1:0))
#endif

#define passphrase "PASSWORD" // TODO: This is only for testing purposes. CHANGE THIS.
#define passlen 8
#define HASHLEN 32
#define SALTLEN 16
#define BUFLEN 262144

char ofilename[2048] = {0x0};
uint8_t prev_block[BUFLEN] = {0x0};
uint8_t buf[BUFLEN] = {0x0};

uint8_t hash[HASHLEN];
uint8_t SALT[SALTLEN];

/* Randomizes the data of a buffer */
int init_buffer(uint8_t* buffer, size_t len)
{
    return (getrandom(buffer, len, GRND_RANDOM) == len);
}

/* Hash the password using Argon2 (id to protect against timing attacks)*/
void hash_password(const char* password)
{
    uint8_t *pwd = (uint8_t*)strdup(password);
    size_t pwdlen = strlen((char*)pwd);
    argon2id_hash_raw(32, (1<<16), 16, pwd, pwdlen, SALT, SALTLEN, hash, HASHLEN);
    free(pwd);
    return;
}

//TODO: FIXME: Maybe set a flag instead when not found?
uint64_t findblockoffset(uint64_t *blockorder, uint64_t listsz, uint64_t blocknumber)
{
    for(uint64_t i = 0; i < listsz; i++)
    {
        if(blockorder[i] == blocknumber)
            return i;
    }
    return 0;
}

int8_t apply_mask(uint8_t *bufin, uint8_t mode, size_t i, uint8_t *prev_clear)
{
    if(!bufin)
        return -1;
    if(mode != 0 && !prev_clear)
        return -2;
    if(i >= BUFLEN)
        return -3;
    if(mode == 0)
    {
        bufin[i] = bufin[i] ^ ((ran_rand8_next() % 254)+1) ^ SALT[i%SALTLEN] ^ hash[i%HASHLEN];
    }else{
        bufin[i] = bufin[i] ^ prev_clear[i] ^ ((ran_rand8_next() % 254)+1) ^ SALT[i%SALTLEN] ^ hash[i%HASHLEN];
    }
    return 0;
}

/* Mask / Unmask */
int fmask(const char* path, const char* pswd, char mode)
{
    size_t fsize = 0;
    size_t blocks = 0;
    size_t last_block = 0;
    size_t rin, rout = 0;
    FILE *fin = fopen(path, "rb");
    FILE *fout;
    uint64_t *blockorder;
    uint64_t order_num;
    uint64_t block_offset = 0;
    uint8_t debug_value = 0; // TODO: REMOVE THIS
    FILE *debg  = fopen((const char*)"debg.png", "wb+");

    // Temporal file
    snprintf(ofilename, 2047, "%s.tmp", path);
    fout = fopen(ofilename, "wb+");

    // Source file
    fseek(fin,0,SEEK_END);
    fsize = ftell(fin); // Get the file size and rewind
    fseek(fin,0,SEEK_SET);

    // How many blocks in the source?
    blocks = DIV_CEIL((int)(fsize - ((mode==1)?SALTLEN:0)), BUFLEN);
    // Size of the last block? TODO: THIS IS NEEDED
    last_block = fsize%BUFLEN;

    // Allocate space for the list of blocks
    blockorder = (uint64_t*)calloc(sizeof(uint64_t), blocks);

    // 0: Mask, 1: Unmask
    if(mode == 0)
    {
        // Key expansion using Argon2
        hash_password(pswd);
        // PRNG init
        ran_initialize(hash, HASHLEN);
        // Write the salt
        fwrite(SALT, SALTLEN, 1, fout);
        // Fill the output file with 0
        memset(buf,0, BUFLEN);
        fwrite(buf, BUFLEN, blocks, fout);
        fseek(fout, SALTLEN, SEEK_SET);
        // Fill the output file with 0
        memset(buf,0, BUFLEN);
        fwrite(buf, BUFLEN, blocks, fout);
        fseek(fout, 0, SEEK_SET);
    }else{
        // Read the salt
        fread(SALT,SALTLEN,1,fin);
        // Key expansion using Argon2
        hash_password(pswd);
        // PRNG init
        ran_initialize(hash, HASHLEN);
    }

    // Get a random number between 0 and blocks-1
    order_num = ran_rand_next();
    // Make sure it doesn't have a factor of the number of blocks
    while(order_num%blocks == 0)
        order_num += 7;

    for(uint64_t i = 0; i < blocks; i++)
    {
        // Determine the order of the blocks
        blockorder[i] = order_num%blocks;
        order_num = (order_num + (blocks + 235454983));
    }

    if(mode == 0)
    {
       /* Cleartext to cypher */
        for(uint64_t b = 0; b < blocks; b++)
        {
            // Initialize the buffer
            init_buffer(buf, BUFLEN);
            // Read a cleartext block
            rin = fread(buf, 1, BUFLEN, fin);
            printf("Read %u bytes out of %u\n", rin, BUFLEN);
            // Keep a copy of the clear text
            memcpy(prev_block, buf, BUFLEN);
            fwrite(prev_block, BUFLEN, 1, debg);
            // Apply the mask
            for(int i = 0; i < BUFLEN; i++)
            {
                if(b == 0)
                {
                    // If this is the first block, salt using the salt itself
                    //buf[i] = buf[i] ^ ((ran_rand8_next() % 254)+1) ^ SALT[i%SALTLEN] ^ hash[i%HASHLEN];
                    apply_mask(buf, 0, i, 0);
                }
                else
                {
                    // Else salt using the previous cypher block and the salt
                    // TODO: Something's wrong with the way it handles prev_block
                    //buf[i] = (uint8_t)prev_block[i] ^ (uint8_t)buf[i] ^ (uint8_t)((ran_rand8_next() % 254)+1) ^ (uint8_t)SALT[i%SALTLEN] ^ (uint8_t)hash[i%HASHLEN];

                    //buf[i] = buf[i] ^ ((ran_rand8_next() % 254)+1) ^ SALT[i%SALTLEN] ^ hash[i%HASHLEN];
                    //buf[i] = buf[i] ^ prev_block[i];
                    apply_mask(buf, 1, i, prev_block);
                }
            }
            // Write the masked block
            block_offset = (BUFLEN * findblockoffset(blockorder, blocks, b)) + SALTLEN;
            fseek(fout,block_offset,SEEK_SET);
            rout = fwrite(buf, 1, rin, fout);
            // Update the cypher with the plaintext for the next iteration
            ran_update(prev_block, BUFLEN, hash, HASHLEN);
        }

    }else if(mode == 1)
    {
        //Initialize the buffer
        memset(buf, 0, BUFLEN);
        /* Cypher to cleartext */
        for(uint64_t b = 0; b < blocks; b++)
        {
            // Find the block offset
            block_offset = (BUFLEN * findblockoffset(blockorder, blocks, b)) + SALTLEN;
            // Seek
            fseek(fin, block_offset, SEEK_SET);
            // Read the cypher block
            rin = fread(buf, 1, BUFLEN, fin);
            printf("Read %u bytes out of %u\n", rin, BUFLEN);
            // Apply the mask
            for(int i = 0; i < BUFLEN; i++)
            {
                if(b == 0)
                {
                    // If this is the first block, salt using the salt itself
                    //buf[i] = buf[i] ^ ((ran_rand8_next() % 254)+1) ^ SALT[i%SALTLEN] ^ hash[i%HASHLEN];
                    apply_mask(buf, 0, i, 0);
                }
                else
                {
                    // Else salt using the previous cypher block and the salt
                    // TODO: Something's wrong with the way it handles prev_block
                    //buf[i] = prev_block[i] ^buf[i];

                    //buf[i] = (uint8_t)prev_block[i] ^ (uint8_t)buf[i] ^ (uint8_t)((ran_rand8_next() % 254)+1) ^ (uint8_t)SALT[i%SALTLEN] ^ (uint8_t)hash[i%HASHLEN];
                    //buf[i] = buf[i] ^ ((ran_rand8_next() % 254)+1) ^ SALT[i%SALTLEN] ^ hash[i%HASHLEN];
                    //buf[i] = buf[i] ^ prev_block[i];
                    apply_mask(buf, 1, i, prev_block);

                }
            }
            // Keep a copy of the clear text
            memcpy(prev_block, buf, BUFLEN);
            fwrite(prev_block, BUFLEN, 1, debg);
            // Write the unmasked block
            rout = fwrite(buf, 1, rin, fout);
            // Update the cypher with the plaintext for the next iteration
            ran_update(prev_block, BUFLEN, hash, HASHLEN);
        }
    }

    // Delete the original file
    memset(buf, 0L, BUFLEN);
    for(int j = 0; j < 3; j++)
    {
        fseek(fin, SEEK_SET, 0L);
        for(int i = 0; i < fsize; i++)
        {
            fwrite(buf, 1, BUFLEN, fin);
        }
    }

    if(remove(path))
    {
        printf("WARNING: Could not remove %s\n", path);
    }
    if(rename(ofilename, path))
    {
        printf("WARNING: Could not rename %s\n", ofilename);
    }
    // Clear the memory
    fclose(fin);
    fclose(fout);
    free(blockorder);
    memset(buf, 0, BUFLEN);
    memset(prev_block, 0, BUFLEN);
    fclose(debg);
    return 0;
}

int main()
{
    // Initialize salt
    if(init_buffer(SALT, SALTLEN) == 0)
    {
        printf("FATAL ERROR: Not enough entropy in this system.\n");
        return -1;
    }

    // TODO: The parameters are not hardcoded (Duh)
    fmask("test2.png", passphrase, 0);

    return 0;
}
