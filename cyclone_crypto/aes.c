/**
 * @file aes.c
 * @brief AES (Advanced Encryption Standard)
 *
 * @section License
 *
 * Copyright (C) 2010-2013 Oryx Embedded. All rights reserved.
 *
 * This file is part of CycloneCrypto Open.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @section Description
 *
 * AES is an encryption standard based on Rijndael algorithm, a symmetric block
 * cipher that can process data blocks of 128 bits, using cipher keys with
 * lengths of 128, 192, and 256 bits. Refer to FIPS 197 for more details
 *
 * @author Oryx Embedded (www.oryx-embedded.com)
 * @version 1.3.8
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL CRYPTO_TRACE_LEVEL

//Dependencies
#include <string.h>
#include "crypto.h"
#include "aes.h"

//Check crypto library configuration
#if (AES_SUPPORT == ENABLED)

//Rotate macro used by key expansion
#define rotWord(w) ROR32(w, 8)

//Substitution table used by encryption algorithm (S-box)
static const uint8_t sbox[256] =
{
   0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
   0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
   0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
   0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
   0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
   0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
   0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
   0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
   0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
   0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
   0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
   0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
   0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
   0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
   0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
   0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};

//Substitution table used by decryption algorithm (inverse S-box)
static const uint8_t isbox[256] =
{
   0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
   0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
   0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
   0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
   0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
   0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
   0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
   0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
   0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
   0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
   0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
   0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
   0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
   0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
   0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
   0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
};

//Multiplication by {02} in the finite field GF(256)
static const uint8_t mul2[256] =
{
   0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
   0x20, 0x22, 0x24, 0x26, 0x28, 0x2A, 0x2C, 0x2E, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3A, 0x3C, 0x3E,
   0x40, 0x42, 0x44, 0x46, 0x48, 0x4A, 0x4C, 0x4E, 0x50, 0x52, 0x54, 0x56, 0x58, 0x5A, 0x5C, 0x5E,
   0x60, 0x62, 0x64, 0x66, 0x68, 0x6A, 0x6C, 0x6E, 0x70, 0x72, 0x74, 0x76, 0x78, 0x7A, 0x7C, 0x7E,
   0x80, 0x82, 0x84, 0x86, 0x88, 0x8A, 0x8C, 0x8E, 0x90, 0x92, 0x94, 0x96, 0x98, 0x9A, 0x9C, 0x9E,
   0xA0, 0xA2, 0xA4, 0xA6, 0xA8, 0xAA, 0xAC, 0xAE, 0xB0, 0xB2, 0xB4, 0xB6, 0xB8, 0xBA, 0xBC, 0xBE,
   0xC0, 0xC2, 0xC4, 0xC6, 0xC8, 0xCA, 0xCC, 0xCE, 0xD0, 0xD2, 0xD4, 0xD6, 0xD8, 0xDA, 0xDC, 0xDE,
   0xE0, 0xE2, 0xE4, 0xE6, 0xE8, 0xEA, 0xEC, 0xEE, 0xF0, 0xF2, 0xF4, 0xF6, 0xF8, 0xFA, 0xFC, 0xFE,
   0x1B, 0x19, 0x1F, 0x1D, 0x13, 0x11, 0x17, 0x15, 0x0B, 0x09, 0x0F, 0x0D, 0x03, 0x01, 0x07, 0x05,
   0x3B, 0x39, 0x3F, 0x3D, 0x33, 0x31, 0x37, 0x35, 0x2B, 0x29, 0x2F, 0x2D, 0x23, 0x21, 0x27, 0x25,
   0x5B, 0x59, 0x5F, 0x5D, 0x53, 0x51, 0x57, 0x55, 0x4B, 0x49, 0x4F, 0x4D, 0x43, 0x41, 0x47, 0x45,
   0x7B, 0x79, 0x7F, 0x7D, 0x73, 0x71, 0x77, 0x75, 0x6B, 0x69, 0x6F, 0x6D, 0x63, 0x61, 0x67, 0x65,
   0x9B, 0x99, 0x9F, 0x9D, 0x93, 0x91, 0x97, 0x95, 0x8B, 0x89, 0x8F, 0x8D, 0x83, 0x81, 0x87, 0x85,
   0xBB, 0xB9, 0xBF, 0xBD, 0xB3, 0xB1, 0xB7, 0xB5, 0xAB, 0xA9, 0xAF, 0xAD, 0xA3, 0xA1, 0xA7, 0xA5,
   0xDB, 0xD9, 0xDF, 0xDD, 0xD3, 0xD1, 0xD7, 0xD5, 0xCB, 0xC9, 0xCF, 0xCD, 0xC3, 0xC1, 0xC7, 0xC5,
   0xFB, 0xF9, 0xFF, 0xFD, 0xF3, 0xF1, 0xF7, 0xF5, 0xEB, 0xE9, 0xEF, 0xED, 0xE3, 0xE1, 0xE7, 0xE5
};

//Round constant word array
static const uint32_t rcon[11] =
{
   0x00000000,
   0x00000001,
   0x00000002,
   0x00000004,
   0x00000008,
   0x00000010,
   0x00000020,
   0x00000040,
   0x00000080,
   0x0000001B,
   0x00000036
};

//Common interface for encryption algorithms
const CipherAlgo aesCipherAlgo =
{
   "AES",
   sizeof(AesContext),
   CIPHER_ALGO_TYPE_BLOCK,
   AES_BLOCK_SIZE,
   (CipherAlgoInit) aesInit,
   NULL,
   NULL,
   (CipherAlgoEncryptBlock) aesEncryptBlock,
   (CipherAlgoDecryptBlock) aesDecryptBlock
};


/**
 * @brief SubWord transformation
 * @param[in] state Pointer to the AES state array
 **/

static uint32_t subWord(uint32_t w)
{
   uint8_t *p = (uint8_t *) &w;

   //Substitute each byte using the S-box table
   p[0] = sbox[p[0]];
   p[1] = sbox[p[1]];
   p[2] = sbox[p[2]];
   p[3] = sbox[p[3]];

   //Return the resulting word
   return w;
}


/**
 * @brief AddRoundKey transformation
 * @param[in] state Pointer to the AES state array
 **/

static void addRoundKey(AesState *state, uint32_t *k)
{
   state->w[0] ^= k[0];
   state->w[1] ^= k[1];
   state->w[2] ^= k[2];
   state->w[3] ^= k[3];
}

/**
 * @brief SubBytes transformation
 * @param[in] state Pointer to the AES state array
 **/

static void subBytes(AesState *state)
{
   uint_t i;

   //Substitute each byte of the state using the S-box table
   for(i = 0; i < 16; i++)
      state->b[i] = sbox[state->b[i]];
}


/**
 * @brief InvSubBytes transformation
 * @param[in] state Pointer to the AES state array
 **/

static void invSubBytes(AesState *state)
{
   uint_t i;

   //Substitute each byte using the inverse S-box table
   for(i = 0; i < 16; i++)
      state->b[i] = isbox[state->b[i]];
}


/**
 * @brief ShiftRows transformation
 * @param[in] state Pointer to the AES state array
 **/

static void shiftRows(AesState *state)
{
   uint8_t temp;

   //The second row is shifted left by 1 byte
   temp = state->b[1];
   state->b[1] = state->b[5];
   state->b[5] = state->b[9];
   state->b[9] = state->b[13];
   state->b[13] = temp;

   //The third row is shifted left by 2 bytes
   temp = state->b[2];
   state->b[2] = state->b[10];
   state->b[10] = temp;
   temp = state->b[6];
   state->b[6] = state->b[14];
   state->b[14] = temp;

   //The last row is shifted left by 3 bytes
   temp = state->b[3];
   state->b[3] = state->b[15];
   state->b[15] = state->b[11];
   state->b[11] = state->b[7];
   state->b[7] = temp;
}


/**
 * @brief InvShiftRows transformation
 * @param[in] state Pointer to the AES state array
 **/

static void invShiftRows(AesState *state)
{
   uint8_t temp;

   //The second row is shifted right by 1 byte
   temp = state->b[1];
   state->b[1] = state->b[13];
   state->b[13] = state->b[9];
   state->b[9] = state->b[5];
   state->b[5] = temp;

   //The third row is shifted right by 2 bytes
   temp = state->b[2];
   state->b[2] = state->b[10];
   state->b[10] = temp;
   temp = state->b[6];
   state->b[6] = state->b[14];
   state->b[14] = temp;

   //The last row is shifted right by 3 bytes
   temp = state->b[3];
   state->b[3] = state->b[7];
   state->b[7] = state->b[11];
   state->b[11] = state->b[15];
   state->b[15] = temp;
}


/**
 * @brief MixColumns transformation
 * @param[in] state Pointer to the AES state array
 **/

static void mixColumns(AesState *state)
{
   uint_t i;
   uint8_t b0;
   uint8_t b1;
   uint8_t b2;
   uint8_t b3;
   uint8_t p;

   //Loop through the columns of the state array
   for(i = 0; i < 16; i += 4)
   {
      //Save current column
      b0 = state->b[i + 0];
      b1 = state->b[i + 1];
      b2 = state->b[i + 2];
      b3 = state->b[i + 3];
      //Intermediate variable
      p = b0 ^ b1 ^ b2 ^ b3;
      //Apply transformation
      state->b[i + 0] = p ^ b0 ^ mul2[b0 ^ b1];
      state->b[i + 1] = p ^ b1 ^ mul2[b1 ^ b2];
      state->b[i + 2] = p ^ b2 ^ mul2[b2 ^ b3];
      state->b[i + 3] = p ^ b3 ^ mul2[b3 ^ b0];
   }
}


/**
 * @brief InvMixColumns transformation
 * @param[in] state Pointer to the AES state array
 **/

static void invMixColumns(AesState *state)
{
   uint_t i;
   uint8_t b0;
   uint8_t b1;
   uint8_t b2;
   uint8_t b3;
   uint8_t p;
   uint8_t q;

   //Loop through the columns of the state array
   for(i = 0; i < 16; i += 4)
   {
      //Save current column
      b0 = state->b[i + 0];
      b1 = state->b[i + 1];
      b2 = state->b[i + 2];
      b3 = state->b[i + 3];
      //Intermediate variable
      q = b0 ^ b1 ^ b2 ^ b3;
      //Compute {09}{b0^b1^b2^b3}
      q = q ^ mul2[mul2[mul2[q]]];
      //Compute {09}{b0^b1^b2^b3} ^ {04}{b0^b2}
      p = q ^ mul2[mul2[b0 ^ b2]];
      //Compute {09}{b0^b1^b2^b3} ^ {04}{b1^b3}
      q = q ^ mul2[mul2[b1 ^ b3]];
      //Apply transformation
      state->b[i + 0] = p ^ b0 ^ mul2[b0 ^ b1];
      state->b[i + 1] = q ^ b1 ^ mul2[b1 ^ b2];
      state->b[i + 2] = p ^ b2 ^ mul2[b2 ^ b3];
      state->b[i + 3] = q ^ b3 ^ mul2[b3 ^ b0];
   }
}


/**
 * @brief Key expansion
 * @param[in] context Pointer to the AES context to initialize
 * @param[in] key Pointer to the key
 * @param[in] keyLength Length of the key
 * @return Error code
 **/

error_t aesInit(AesContext *context, const uint8_t *key, size_t keyLength)
{
   uint_t i;
   uint32_t temp;
   size_t keyScheduleSize;

   //10 rounds are required for 128-bit key
   if(keyLength == 16)
      context->nr = 10;
   //12 rounds are required for 192-bit key
   else if(keyLength == 24)
      context->nr = 12;
   //14 rounds are required for 256-bit key
   else if(keyLength == 32)
      context->nr = 14;
   //Key length is not supported...
   else
      return ERROR_INVALID_KEY_LENGTH;

   //Copy the original key
   memcpy(context->w, key, keyLength);
   //Determine the number of 32-bit words in the key
   keyLength /= 4;
   //The size of the key schedule depends on the number of rounds
   keyScheduleSize = 4 * (context->nr + 1);

   //Perform a key expansion to generate the key schedule
   for(i = keyLength; i < keyScheduleSize; i++)
   {
      //Save previous word
      temp = context->w[i - 1];
      //Apply transformation
      if((i % keyLength) == 0)
         temp = subWord(rotWord(temp)) ^ rcon[i / keyLength];
      else if(keyLength > 6 && (i % keyLength) == 4)
         temp = subWord(temp);
      //Update the key schedule
      context->w[i] = context->w[i - keyLength] ^ temp;
   }

   //No error to report
   return NO_ERROR;
}


/**
 * @brief Encrypt a 16-byte block using AES algorithm
 * @param[in] context Pointer to the AES context
 * @param[in] input Plaintext block to encrypt
 * @param[out] output Ciphertext block resulting from encryption
 **/

void aesEncryptBlock(AesContext *context, const uint8_t *input, uint8_t *output)
{
   uint_t i;
   AesState state;

   //Copy the plaintext to the state array
   memcpy(&state, input, AES_BLOCK_SIZE);

   //Initial round key addition
   addRoundKey(&state, context->w);

   //Apply round function 10, 12 or 14 times depending on the key length
   for(i = 1; i < context->nr; i++)
   {
      //Apply the four transformations
      subBytes(&state);
      shiftRows(&state);
      mixColumns(&state);
      addRoundKey(&state, context->w + 4 * i);
   }

   //The last round differs slightly from the first rounds
   subBytes(&state);
   shiftRows(&state);
   addRoundKey(&state, context->w + 4 * context->nr);

   //The final state is then copied to the output
   memcpy(output, &state, AES_BLOCK_SIZE);
}


/**
 * @brief Decrypt a 16-byte block using AES algorithm
 * @param[in] context Pointer to the AES context
 * @param[in] input Ciphertext block to decrypt
 * @param[out] output Plaintext block resulting from decryption
 **/

void aesDecryptBlock(AesContext *context, const uint8_t *input, uint8_t *output)
{
   uint_t i;
   AesState state;

   //Copy the ciphertext to the state array
   memcpy(&state, input, AES_BLOCK_SIZE);
   //Initial round key addition
   addRoundKey(&state, context->w + 4 * context->nr);

   //Apply round function 10, 12 or 14 times depending on the key length
   for(i = context->nr - 1; i >= 1; i--)
   {
      //Apply the four transformations
      invShiftRows(&state);
      invSubBytes(&state);
      addRoundKey(&state, context->w + 4 * i);
      invMixColumns(&state);
   }

   //The last round differs slightly from the first rounds
   invShiftRows(&state);
   invSubBytes(&state);
   addRoundKey(&state, context->w);

   //The final state is then copied to the output
   memcpy(output, &state, AES_BLOCK_SIZE);
}

#endif
