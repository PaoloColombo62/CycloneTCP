/**
 * @file camellia.c
 * @brief Camellia encryption algorithm
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
 * Camellia is an encryption algorithm designed to encipher and decipher
 * blocks of 128 bits under control of a 128/192/256-bit secret key
 *
 * @author Oryx Embedded (www.oryx-embedded.com)
 * @version 1.3.8
 **/

//Switch to the appropriate trace level
#define TRACE_LEVEL CRYPTO_TRACE_LEVEL

//Dependencies
#include <string.h>
#include "crypto.h"
#include "camellia.h"
#include "debug.h"

//Check crypto library configuration
#if (CAMELLIA_SUPPORT == ENABLED)

//Camellia round function
#define CAMELLIA_ROUND(left1, left2, right1, right2, k1, k2) \
{ \
   temp1 = left1 ^ k1; \
   temp2 = left2 ^ k2; \
   CAMELLIA_S(temp1, temp2); \
   CAMELLIA_P(temp1, temp2); \
   temp1 ^= right2; \
   temp2 ^= right1; \
   right1 = left1; \
   right2 = left2; \
   left1 = temp2; \
   left2 = temp1; \
}

//F-function
#define CAMELLIA_F(xl, xr, kl, kr) \
{ \
   xl = xl ^ kl; \
   xl = xr ^ kr; \
   CAMELLIA_S(xl, xr); \
   CAMELLIA_P(xl, xr); \
}

//FL-function
#define CAMELLIA_FL(xl, xr, kl, kr) \
{ \
   temp1 = (xl & kl); \
   xr ^= ROL32(temp1, 1); \
   xl ^= (xr | kr); \
}

//Inverse FL-function
#define CAMELLIA_INV_FL(yl, yr, kl, kr) \
{ \
   yl ^= (yr | kr); \
   temp1 = (yl & kl); \
   yr ^= ROL32(temp1, 1); \
}

//S-function
#define CAMELLIA_S(zl, zr) \
{ \
   zl = (sbox1[(zl >> 24) & 0xFF] << 24) | (sbox2[(zl >> 16) & 0xFF] << 16) | \
      (sbox3[(zl >> 8) & 0xFF] << 8) | sbox4[zl & 0xFF]; \
   zr = (sbox2[(zr >> 24) & 0xFF] << 24) | (sbox3[(zr >> 16) & 0xFF] << 16) | \
      (sbox4[(zr >> 8) & 0xFF] << 8) | sbox1[zr & 0xFF]; \
}

//P-function
#define CAMELLIA_P(zl, zr) \
{ \
   zl ^= ROL32(zr, 8); \
   zr ^= ROL32(zl, 16); \
   zl ^= ROR32(zr, 8); \
   zr ^= ROR32(zl, 8); \
}

//Key schedule related constants
#define KL 0
#define KR 4
#define KA 8
#define KB 12
#define L  0
#define R  64

//Key schedule for 128-bit key
static const CamelliaSubkey ks1[] =
{
   {0,  KL, 0,  L},  //kw1
   {2,  KL, 0,  R},  //kw2
   {4,  KA, 0,  L},  //k1
   {6,  KA, 0,  R},  //k2
   {8,  KL, 15, L},  //k3
   {10, KL, 15, R},  //k4
   {12, KA, 15, L},  //k5
   {14, KA, 15, R},  //k6
   {16, KA, 30, L},  //ke1
   {18, KA, 30, R},  //ke2
   {20, KL, 45, L},  //k7
   {22, KL, 45, R},  //k8
   {24, KA, 45, L},  //k9
   {26, KL, 60, R},  //k10
   {28, KA, 60, L},  //k11
   {30, KA, 60, R},  //k12
   {32, KL, 77, L},  //ke3
   {34, KL, 77, R},  //ke4
   {36, KL, 94, L},  //k13
   {38, KL, 94, R},  //k14
   {40, KA, 94, L},  //k15
   {42, KA, 94, R},  //k16
   {44, KL, 111, L}, //k17
   {46, KL, 111, R}, //k18
   {48, KA, 111, L}, //kw3
   {50, KA, 111, R}, //kw4
};

//Key schedule for 192 and 256-bit keys
static const CamelliaSubkey ks2[] =
{
   {0,  KL, 0,  L},  //kw1
   {2,  KL, 0,  R},  //k2
   {4,  KB, 0,  L},  //k1
   {6,  KB, 0,  R},  //k2
   {8,  KR, 15, L},  //k3
   {10, KR, 15, R},  //k4
   {12, KA, 15, L},  //k5
   {14, KA, 15, R},  //k6
   {16, KR, 30, L},  //ke1
   {18, KR, 30, R},  //ke2
   {20, KB, 30, L},  //k7
   {22, KB, 30, R},  //k8
   {24, KL, 45, L},  //k9
   {26, KL, 45, R},  //k10
   {28, KA, 45, L},  //k11
   {30, KA, 45, R},  //k12
   {32, KL, 60, L},  //ke3
   {34, KL, 60, R},  //ke4
   {36, KR, 60, L},  //k13
   {38, KR, 60, R},  //k14
   {40, KB, 60, L},  //k15
   {42, KB, 60, R},  //k16
   {44, KL, 77, L},  //k17
   {46, KL, 77, R},  //k18
   {48, KA, 77, L},  //ke5
   {50, KA, 77, R},  //ke6
   {52, KR, 94, L},  //k19
   {54, KR, 94, R},  //k20
   {56, KA, 94, L},  //k21
   {58, KA, 94, R},  //k22
   {60, KL, 111, L}, //k23
   {62, KL, 111, R}, //k24
   {64, KB, 111, L}, //kw3
   {66, KB, 111, R}, //kw4
};

//Key schedule constants
static const uint32_t sigma[12] =
{
   0xA09E667F, 0x3BCC908B,
   0xB67AE858, 0x4CAA73B2,
   0xC6EF372F, 0xE94F82BE,
   0x54FF53A5, 0xF1D36F1C,
   0x10E527FA, 0xDE682D1D,
   0xB05688C2, 0xB3E6C1FD
};

//Substitution table 1
static const uint8_t sbox1[256] =
{
   0x70, 0x82, 0x2C, 0xEC, 0xB3, 0x27, 0xC0, 0xE5, 0xE4, 0x85, 0x57, 0x35, 0xEA, 0x0C, 0xAE, 0x41,
   0x23, 0xEF, 0x6B, 0x93, 0x45, 0x19, 0xA5, 0x21, 0xED, 0x0E, 0x4F, 0x4E, 0x1D, 0x65, 0x92, 0xBD,
   0x86, 0xB8, 0xAF, 0x8F, 0x7C, 0xEB, 0x1F, 0xCE, 0x3E, 0x30, 0xDC, 0x5F, 0x5E, 0xC5, 0x0B, 0x1A,
   0xA6, 0xE1, 0x39, 0xCA, 0xD5, 0x47, 0x5D, 0x3D, 0xD9, 0x01, 0x5A, 0xD6, 0x51, 0x56, 0x6C, 0x4D,
   0x8B, 0x0D, 0x9A, 0x66, 0xFB, 0xCC, 0xB0, 0x2D, 0x74, 0x12, 0x2B, 0x20, 0xF0, 0xB1, 0x84, 0x99,
   0xDF, 0x4C, 0xCB, 0xC2, 0x34, 0x7E, 0x76, 0x05, 0x6D, 0xB7, 0xA9, 0x31, 0xD1, 0x17, 0x04, 0xD7,
   0x14, 0x58, 0x3A, 0x61, 0xDE, 0x1B, 0x11, 0x1C, 0x32, 0x0F, 0x9C, 0x16, 0x53, 0x18, 0xF2, 0x22,
   0xFE, 0x44, 0xCF, 0xB2, 0xC3, 0xB5, 0x7A, 0x91, 0x24, 0x08, 0xE8, 0xA8, 0x60, 0xFC, 0x69, 0x50,
   0xAA, 0xD0, 0xA0, 0x7D, 0xA1, 0x89, 0x62, 0x97, 0x54, 0x5B, 0x1E, 0x95, 0xE0, 0xFF, 0x64, 0xD2,
   0x10, 0xC4, 0x00, 0x48, 0xA3, 0xF7, 0x75, 0xDB, 0x8A, 0x03, 0xE6, 0xDA, 0x09, 0x3F, 0xDD, 0x94,
   0x87, 0x5C, 0x83, 0x02, 0xCD, 0x4A, 0x90, 0x33, 0x73, 0x67, 0xF6, 0xF3, 0x9D, 0x7F, 0xBF, 0xE2,
   0x52, 0x9B, 0xD8, 0x26, 0xC8, 0x37, 0xC6, 0x3B, 0x81, 0x96, 0x6F, 0x4B, 0x13, 0xBE, 0x63, 0x2E,
   0xE9, 0x79, 0xA7, 0x8C, 0x9F, 0x6E, 0xBC, 0x8E, 0x29, 0xF5, 0xF9, 0xB6, 0x2F, 0xFD, 0xB4, 0x59,
   0x78, 0x98, 0x06, 0x6A, 0xE7, 0x46, 0x71, 0xBA, 0xD4, 0x25, 0xAB, 0x42, 0x88, 0xA2, 0x8D, 0xFA,
   0x72, 0x07, 0xB9, 0x55, 0xF8, 0xEE, 0xAC, 0x0A, 0x36, 0x49, 0x2A, 0x68, 0x3C, 0x38, 0xF1, 0xA4,
   0x40, 0x28, 0xD3, 0x7B, 0xBB, 0xC9, 0x43, 0xC1, 0x15, 0xE3, 0xAD, 0xF4, 0x77, 0xC7, 0x80, 0x9E
};

//Substitution table 2
static const uint8_t sbox2[256] =
{
   0xE0, 0x05, 0x58, 0xD9, 0x67, 0x4E, 0x81, 0xCB, 0xC9, 0x0B, 0xAE, 0x6A, 0xD5, 0x18, 0x5D, 0x82,
   0x46, 0xDF, 0xD6, 0x27, 0x8A, 0x32, 0x4B, 0x42, 0xDB, 0x1C, 0x9E, 0x9C, 0x3A, 0xCA, 0x25, 0x7B,
   0x0D, 0x71, 0x5F, 0x1F, 0xF8, 0xD7, 0x3E, 0x9D, 0x7C, 0x60, 0xB9, 0xBE, 0xBC, 0x8B, 0x16, 0x34,
   0x4D, 0xC3, 0x72, 0x95, 0xAB, 0x8E, 0xBA, 0x7A, 0xB3, 0x02, 0xB4, 0xAD, 0xA2, 0xAC, 0xD8, 0x9A,
   0x17, 0x1A, 0x35, 0xCC, 0xF7, 0x99, 0x61, 0x5A, 0xE8, 0x24, 0x56, 0x40, 0xE1, 0x63, 0x09, 0x33,
   0xBF, 0x98, 0x97, 0x85, 0x68, 0xFC, 0xEC, 0x0A, 0xDA, 0x6F, 0x53, 0x62, 0xA3, 0x2E, 0x08, 0xAF,
   0x28, 0xB0, 0x74, 0xC2, 0xBD, 0x36, 0x22, 0x38, 0x64, 0x1E, 0x39, 0x2C, 0xA6, 0x30, 0xE5, 0x44,
   0xFD, 0x88, 0x9F, 0x65, 0x87, 0x6B, 0xF4, 0x23, 0x48, 0x10, 0xD1, 0x51, 0xC0, 0xF9, 0xD2, 0xA0,
   0x55, 0xA1, 0x41, 0xFA, 0x43, 0x13, 0xC4, 0x2F, 0xA8, 0xB6, 0x3C, 0x2B, 0xC1, 0xFF, 0xC8, 0xA5,
   0x20, 0x89, 0x00, 0x90, 0x47, 0xEF, 0xEA, 0xB7, 0x15, 0x06, 0xCD, 0xB5, 0x12, 0x7E, 0xBB, 0x29,
   0x0F, 0xB8, 0x07, 0x04, 0x9B, 0x94, 0x21, 0x66, 0xE6, 0xCE, 0xED, 0xE7, 0x3B, 0xFE, 0x7F, 0xC5,
   0xA4, 0x37, 0xB1, 0x4C, 0x91, 0x6E, 0x8D, 0x76, 0x03, 0x2D, 0xDE, 0x96, 0x26, 0x7D, 0xC6, 0x5C,
   0xD3, 0xF2, 0x4F, 0x19, 0x3F, 0xDC, 0x79, 0x1D, 0x52, 0xEB, 0xF3, 0x6D, 0x5E, 0xFB, 0x69, 0xB2,
   0xF0, 0x31, 0x0C, 0xD4, 0xCF, 0x8C, 0xE2, 0x75, 0xA9, 0x4A, 0x57, 0x84, 0x11, 0x45, 0x1B, 0xF5,
   0xE4, 0x0E, 0x73, 0xAA, 0xF1, 0xDD, 0x59, 0x14, 0x6C, 0x92, 0x54, 0xD0, 0x78, 0x70, 0xE3, 0x49,
   0x80, 0x50, 0xA7, 0xF6, 0x77, 0x93, 0x86, 0x83, 0x2A, 0xC7, 0x5B, 0xE9, 0xEE, 0x8F, 0x01, 0x3D
};

//Substitution table 3
static const uint8_t sbox3[256] =
{
   0x38, 0x41, 0x16, 0x76, 0xD9, 0x93, 0x60, 0xF2, 0x72, 0xC2, 0xAB, 0x9A, 0x75, 0x06, 0x57, 0xA0,
   0x91, 0xF7, 0xB5, 0xC9, 0xA2, 0x8C, 0xD2, 0x90, 0xF6, 0x07, 0xA7, 0x27, 0x8E, 0xB2, 0x49, 0xDE,
   0x43, 0x5C, 0xD7, 0xC7, 0x3E, 0xF5, 0x8F, 0x67, 0x1F, 0x18, 0x6E, 0xAF, 0x2F, 0xE2, 0x85, 0x0D,
   0x53, 0xF0, 0x9C, 0x65, 0xEA, 0xA3, 0xAE, 0x9E, 0xEC, 0x80, 0x2D, 0x6B, 0xA8, 0x2B, 0x36, 0xA6,
   0xC5, 0x86, 0x4D, 0x33, 0xFD, 0x66, 0x58, 0x96, 0x3A, 0x09, 0x95, 0x10, 0x78, 0xD8, 0x42, 0xCC,
   0xEF, 0x26, 0xE5, 0x61, 0x1A, 0x3F, 0x3B, 0x82, 0xB6, 0xDB, 0xD4, 0x98, 0xE8, 0x8B, 0x02, 0xEB,
   0x0A, 0x2C, 0x1D, 0xB0, 0x6F, 0x8D, 0x88, 0x0E, 0x19, 0x87, 0x4E, 0x0B, 0xA9, 0x0C, 0x79, 0x11,
   0x7F, 0x22, 0xE7, 0x59, 0xE1, 0xDA, 0x3D, 0xC8, 0x12, 0x04, 0x74, 0x54, 0x30, 0x7E, 0xB4, 0x28,
   0x55, 0x68, 0x50, 0xBE, 0xD0, 0xC4, 0x31, 0xCB, 0x2A, 0xAD, 0x0F, 0xCA, 0x70, 0xFF, 0x32, 0x69,
   0x08, 0x62, 0x00, 0x24, 0xD1, 0xFB, 0xBA, 0xED, 0x45, 0x81, 0x73, 0x6D, 0x84, 0x9F, 0xEE, 0x4A,
   0xC3, 0x2E, 0xC1, 0x01, 0xE6, 0x25, 0x48, 0x99, 0xB9, 0xB3, 0x7B, 0xF9, 0xCE, 0xBF, 0xDF, 0x71,
   0x29, 0xCD, 0x6C, 0x13, 0x64, 0x9B, 0x63, 0x9D, 0xC0, 0x4B, 0xB7, 0xA5, 0x89, 0x5F, 0xB1, 0x17,
   0xF4, 0xBC, 0xD3, 0x46, 0xCF, 0x37, 0x5E, 0x47, 0x94, 0xFA, 0xFC, 0x5B, 0x97, 0xFE, 0x5A, 0xAC,
   0x3C, 0x4C, 0x03, 0x35, 0xF3, 0x23, 0xB8, 0x5D, 0x6A, 0x92, 0xD5, 0x21, 0x44, 0x51, 0xC6, 0x7D,
   0x39, 0x83, 0xDC, 0xAA, 0x7C, 0x77, 0x56, 0x05, 0x1B, 0xA4, 0x15, 0x34, 0x1E, 0x1C, 0xF8, 0x52,
   0x20, 0x14, 0xE9, 0xBD, 0xDD, 0xE4, 0xA1, 0xE0, 0x8A, 0xF1, 0xD6, 0x7A, 0xBB, 0xE3, 0x40, 0x4F
};

//Substitution table 4
static const uint8_t sbox4[256] =
{
   0x70, 0x2C, 0xB3, 0xC0, 0xE4, 0x57, 0xEA, 0xAE, 0x23, 0x6B, 0x45, 0xA5, 0xED, 0x4F, 0x1D, 0x92,
   0x86, 0xAF, 0x7C, 0x1F, 0x3E, 0xDC, 0x5E, 0x0B, 0xA6, 0x39, 0xD5, 0x5D, 0xD9, 0x5A, 0x51, 0x6C,
   0x8B, 0x9A, 0xFB, 0xB0, 0x74, 0x2B, 0xF0, 0x84, 0xDF, 0xCB, 0x34, 0x76, 0x6D, 0xA9, 0xD1, 0x04,
   0x14, 0x3A, 0xDE, 0x11, 0x32, 0x9C, 0x53, 0xF2, 0xFE, 0xCF, 0xC3, 0x7A, 0x24, 0xE8, 0x60, 0x69,
   0xAA, 0xA0, 0xA1, 0x62, 0x54, 0x1E, 0xE0, 0x64, 0x10, 0x00, 0xA3, 0x75, 0x8A, 0xE6, 0x09, 0xDD,
   0x87, 0x83, 0xCD, 0x90, 0x73, 0xF6, 0x9D, 0xBF, 0x52, 0xD8, 0xC8, 0xC6, 0x81, 0x6F, 0x13, 0x63,
   0xE9, 0xA7, 0x9F, 0xBC, 0x29, 0xF9, 0x2F, 0xB4, 0x78, 0x06, 0xE7, 0x71, 0xD4, 0xAB, 0x88, 0x8D,
   0x72, 0xB9, 0xF8, 0xAC, 0x36, 0x2A, 0x3C, 0xF1, 0x40, 0xD3, 0xBB, 0x43, 0x15, 0xAD, 0x77, 0x80,
   0x82, 0xEC, 0x27, 0xE5, 0x85, 0x35, 0x0C, 0x41, 0xEF, 0x93, 0x19, 0x21, 0x0E, 0x4E, 0x65, 0xBD,
   0xB8, 0x8F, 0xEB, 0xCE, 0x30, 0x5F, 0xC5, 0x1A, 0xE1, 0xCA, 0x47, 0x3D, 0x01, 0xD6, 0x56, 0x4D,
   0x0D, 0x66, 0xCC, 0x2D, 0x12, 0x20, 0xB1, 0x99, 0x4C, 0xC2, 0x7E, 0x05, 0xB7, 0x31, 0x17, 0xD7,
   0x58, 0x61, 0x1B, 0x1C, 0x0F, 0x16, 0x18, 0x22, 0x44, 0xB2, 0xB5, 0x91, 0x08, 0xA8, 0xFC, 0x50,
   0xD0, 0x7D, 0x89, 0x97, 0x5B, 0x95, 0xFF, 0xD2, 0xC4, 0x48, 0xF7, 0xDB, 0x03, 0xDA, 0x3F, 0x94,
   0x5C, 0x02, 0x4A, 0x33, 0x67, 0xF3, 0x7F, 0xE2, 0x9B, 0x26, 0x37, 0x3B, 0x96, 0x4B, 0xBE, 0x2E,
   0x79, 0x8C, 0x6E, 0x8E, 0xF5, 0xB6, 0xFD, 0x59, 0x98, 0x6A, 0x46, 0xBA, 0x25, 0x42, 0xA2, 0xFA,
   0x07, 0x55, 0xEE, 0x0A, 0x49, 0x68, 0x38, 0xA4, 0x28, 0x7B, 0xC9, 0xC1, 0xE3, 0xF4, 0xC7, 0x9E
};

//Common interface for encryption algorithms
const CipherAlgo camelliaCipherAlgo =
{
   "CAMELLIA",
   sizeof(CamelliaContext),
   CIPHER_ALGO_TYPE_BLOCK,
   CAMELLIA_BLOCK_SIZE,
   (CipherAlgoInit) camelliaInit,
   NULL,
   NULL,
   (CipherAlgoEncryptBlock) camelliaEncryptBlock,
   (CipherAlgoDecryptBlock) camelliaDecryptBlock
};


/**
 * @brief Initialize a Camellia context using the supplied key
 * @param[in] context Pointer to the Camellia context to initialize
 * @param[in] key Pointer to the key
 * @param[in] keyLength Length of the key
 * @return Error code
 **/

error_t camelliaInit(CamelliaContext *context, const uint8_t *key, size_t keyLength)
{
   uint_t i;
   uint32_t temp1;
   uint32_t temp2;
   uint32_t *k;
   const CamelliaSubkey *p;

   //18 rounds are required for 128-bit key
   if(keyLength == 16)
      context->nr = 18;
   //24 rounds are required for 192 and 256-bit keys
   else if(keyLength == 24 || keyLength == 32)
      context->nr = 24;
   //Key length is not supported...
   else
      return ERROR_INVALID_KEY_LENGTH;

   //Point to KA, KB, KL and KR
   k = context->k;
   //Clear key contents
   memset(k, 0, 64);
   //Save the supplied secret key
   memcpy(k, key, keyLength);

   //192-bit keys require special processing
   if(keyLength == 24)
   {
      //Form a 256-bit key
      k[KR + 2] = ~k[KR + 0];
      k[KR + 3] = ~k[KR + 1];
   }

   //XOR KL and KR before applying the rounds
   for(i = 0; i < 4; i++)
   {
      k[KL + i] = betoh32(k[KL + i]);
      k[KR + i] = betoh32(k[KR + i]);
      k[KB + i] = k[KL + i] ^ k[KR + i];
   }

   //Generate the 128-bit keys KA and KB
   for(i = 0; i < 6; i++)
   {
      //Apply round function
      CAMELLIA_ROUND(k[KB + 0], k[KB + 1], k[KB + 2], k[KB + 3], sigma[2 * i], sigma[2 * i + 1]);

      //The 2nd round requires special processing
      if(i == 1)
      {
         //The result is XORed with KL
         k[KB + 0] ^= k[KL + 0];
         k[KB + 1] ^= k[KL + 1];
         k[KB + 2] ^= k[KL + 2];
         k[KB + 3] ^= k[KL + 3];
      }
      //The 4th round requires special processing
      else if(i == 3)
      {
         //Save KA after the 4th round
         memcpy(k + KA, k + KB, 16);
         //The result is XORed with KR
         k[KB + 0] ^= k[KR + 0];
         k[KB + 1] ^= k[KR + 1];
         k[KB + 2] ^= k[KR + 2];
         k[KB + 3] ^= k[KR + 3];
      }
   }

   //The key schedule depends on the length of key
   if(keyLength == 16)
   {
      //Key schedule for 128-bit key
      i = arraysize(ks1);
      p = ks1;
   }
   else
   {
      //Key schedule for 192 and 256-bit keys
      i = arraysize(ks2);
      p = ks2;
   }

   //Generate subkeys
   while(i > 0)
   {
      //Calculate the shift count
      uint_t n = (p->shift + p->position) / 32;
      uint_t m = (p->shift + p->position) % 32;
      //Point to KL, KR, KA or KB
      k = context->k + p->key;

      //Generate the current subkey
      if(m == 0)
      {
         context->ks[p->index] = k[n % 4];
         context->ks[p->index + 1] = k[(n + 1) % 4];
      }
      else
      {
         context->ks[p->index] = (k[n % 4] << m) | (k[(n + 1) % 4] >> (32 - m));
         context->ks[p->index + 1] = (k[(n + 1) % 4] << m) | (k[(n + 2) % 4] >> (32 - m));
      }

      //Next subkey
      p++;
      i--;
   }

   //No error to report
   return NO_ERROR;
}


/**
 * @brief Encrypt a 16-byte block using Camellia algorithm
 * @param[in] context Pointer to the Camellia context
 * @param[in] input Plaintext block to encrypt
 * @param[out] output Ciphertext block resulting from encryption
 **/

void camelliaEncryptBlock(CamelliaContext *context, const uint8_t *input, uint8_t *output)
{
   uint_t i;
   uint32_t temp1;
   uint32_t temp2;
   uint32_t *ks;

   //The plaintext is separated into two parts (L and R)
   uint32_t left1 = LOAD32BE(input + 0);
   uint32_t left2 = LOAD32BE(input + 4);
   uint32_t right1 = LOAD32BE(input + 8);
   uint32_t right2 = LOAD32BE(input + 12);

   //The key schedule must be applied in ascending order
   ks = context->ks;

   //XOR plaintext with kw1 and kw2
   left1 ^= ks[0];
   left2 ^= ks[1];
   right1 ^= ks[2];
   right2 ^= ks[3];
   //Advance current location in key schedule
   ks += 4;

   //Apply round function 18 or 24 times depending on the key length
   for(i = context->nr; i > 0; i--)
   {
      //Apply round function
      CAMELLIA_ROUND(left1, left2, right1, right2, ks[0], ks[1]);
      //Advance current location in key schedule
      ks += 2;

      //6th, 12th and 18th rounds require special processing
      if(i == 7 || i == 13 || i == 19)
      {
         //Apply FL-function
         CAMELLIA_FL(left1, left2, ks[0], ks[1])
         //Apply inverse FL-function
         CAMELLIA_INV_FL(right1, right2, ks[2], ks[3])
         //Advance current location in key schedule
         ks += 4;
      }
   }

   //XOR operation with kw3 and kw4
   right1 ^= ks[0];
   right2 ^= ks[1];
   left1 ^= ks[2];
   left2 ^= ks[3];

   //The resulting value is the ciphertext
   STORE32BE(right1, output + 0);
   STORE32BE(right2, output + 4);
   STORE32BE(left1, output + 8);
   STORE32BE(left2, output + 12);
}


/**
 * @brief Decrypt a 16-byte block using Camellia algorithm
 * @param[in] context Pointer to the Camellia context
 * @param[in] input Ciphertext block to decrypt
 * @param[out] output Plaintext block resulting from decryption
 **/

void camelliaDecryptBlock(CamelliaContext *context, const uint8_t *input, uint8_t *output)
{
   uint_t i;
   uint32_t temp1;
   uint32_t temp2;
   uint32_t *ks;

   //The ciphertext is separated into two parts (L and R)
   uint32_t right1 = LOAD32BE(input + 0);
   uint32_t right2 = LOAD32BE(input + 4);
   uint32_t left1 = LOAD32BE(input + 8);
   uint32_t left2 = LOAD32BE(input + 12);

   //The key schedule must be applied in reverse order
   ks = (context->nr == 18) ? (context->ks + 48) : (context->ks + 64);

   //XOR ciphertext with kw3 and kw4
   right1 ^= ks[0];
   right2 ^= ks[1];
   left1 ^= ks[2];
   left2 ^= ks[3];

   //Apply round function 18 or 24 times depending on the key length
   for(i = context->nr; i > 0; i--)
   {
      //Update current location in key schedule
      ks -= 2;
      //Apply round function
      CAMELLIA_ROUND(right1, right2, left1, left2, ks[0], ks[1]);

      //6th, 12th and 18th rounds require special processing
      if(i == 7 || i == 13 || i == 19)
      {
         //Update current location in key schedule
         ks -= 4;
         //Apply FL-function
         CAMELLIA_FL(right1, right2, ks[2], ks[3])
         //Apply inverse FL-function
         CAMELLIA_INV_FL(left1, left2, ks[0], ks[1])
      }
   }

   //Update current location in key schedule
   ks -= 4;
   //XOR operation with kw1 and kw2
   left1 ^= ks[0];
   left2 ^= ks[1];
   right1 ^= ks[2];
   right2 ^= ks[3];

   //The resulting value is the plaintext
   STORE32BE(left1, output + 0);
   STORE32BE(left2, output + 4);
   STORE32BE(right1, output + 8);
   STORE32BE(right2, output + 12);
}

#endif
