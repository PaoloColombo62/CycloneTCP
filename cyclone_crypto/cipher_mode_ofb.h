/**
 * @file cipher_mode_ofb.h
 * @brief Output Feedback (OFB) mode
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
 * @author Oryx Embedded (www.oryx-embedded.com)
 * @version 1.3.8
 **/

#ifndef _BLOCK_CIPHER_MODE_OFB_H
#define _BLOCK_CIPHER_MODE_OFB_H

//Dependencies
#include "crypto.h"

//OFB encryption and decryption routines
error_t ofbEncrypt(const CipherAlgo *cipher, void *context, uint_t s,
   uint8_t *iv, const uint8_t *p, uint8_t *c, size_t length);

error_t ofbDecrypt(const CipherAlgo *cipher, void *context, uint_t s,
   uint8_t *iv, const uint8_t *c, uint8_t *p, size_t length);

#endif
