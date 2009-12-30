/** @file hqrdepack.c
	@brief
	This file contains High Quality Resource (HQR) decompress routines.

	Prequengine: a Little Big Adventure engine

	Copyright (C) 2008 Prequengine team \n
	Copyright (C) 2002-2007 The TwinEngine team \n

	This program is free software; you can redistribute it and/or \n
	modify it under the terms of the GNU General Public License \n
	as published by the Free Software Foundation; either version 2 \n
	of the License, or (at your option) any later version. \n

	This program is distributed in the hope that it will be useful, \n
	but WITHOUT ANY WARRANTY; without even the implied warranty of \n
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the \n
	GNU General Public License for more details. \n

	You should have received a copy of the GNU General Public License \n
	along with this program; if not, write to the Free Software \n
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

	$URL$
	$Id$
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hqrdepack.h"
#include "filereader.h"

FileReader fr;

/** Decompress entry based in Yaz0r and Zink decompression code
	@param dst destination pointer where will be the decompressed entry
	@param src compressed data pointer
	@decompsize real file size after decompression
	@mode compression mode used */
void hqr_decompress_entry(uint8 * dst, uint8 * src, int32 decompsize, int32 mode) {
	uint8 b;
	int32 lenght, d, i;
	uint16 offset;
	uint8 *ptr;

	do {
		b = *(src++);
		for (d = 0; d < 8; d++) {
			if (!(b & (1 << d))) {
				offset = *(uint16*)(src);
				src += 2;
				lenght = (offset & 0x0F) + (mode + 1);
				ptr = dst - (offset >> 4) - 1;
				for (i = 0; i < lenght; i++)
					*(dst++) = *(ptr++);
			} else {
				lenght = 1;
				*(dst++) = *(src++);
			}
			decompsize -= lenght;
			if (decompsize <= 0)
				return;
		}
	} while (decompsize);
}

/** Decompress entry based in the original expand lzss lba code
	@param dst destination pointer where will be the decompressed entry
	@param src compressed data pointer
	@decompsize real file size after decompression
	@mode compression mode used */
void hqr_decompress_lz_entry(uint8 * dst, uint8 * src, int32 decompsize, int32 mode) {
	uint16 offset;
	int32 lenght;
	uint8 *ptr;

	while (decompsize > 0) {
		uint8 bits;
		uint8 type = *(src++);
		for (bits = 1; bits != 0; bits <<= 1) {
			if (!(type&bits)) {
				offset = *(uint16*)(src);
				src += 2;
				lenght = (offset & 0x0F) + (mode + 1);
				ptr = dst - (offset >> 4) - 1;
				if (offset == 0) {
					memset(dst, *ptr, lenght);
				} else {
					if ((ptr + lenght) >= dst) {
						int32 n;
						uint8 *tmp = dst;
						for (n = 0; n < lenght; n++)
							*tmp++ = *ptr++;
					} else {
						memcpy(dst, ptr, lenght);
					}
				}
				dst += lenght;
			} else {
				lenght = 1;
				*(dst++) = *(src++);
			}
			decompsize -= lenght;
			if (decompsize <= 0)
				return;
		}
	}
}

/** Get a HQR entry pointer
	@param ptr pointer to save the entry
	@param filename HQR file name
	@param index entry index to extract
	@return entry real size*/
int32 hqr_get_entry(uint8 * ptr, int8 *filename, int32 index) {
	uint32 headerSize;
	uint32 offsetToData;
	uint32 realSize;
	uint32 compSize;
	uint16 mode;

	if (!filename)
		return 0;

	if (!fropen(&fr, filename))
		printf("HQR: %s can't be found !\n", filename);

	frread(&fr, &headerSize, 4);

	if ((uint32)index >= headerSize / 4) {
		printf("\nHQR WARNING: Invalid entry index!!\n");
		frclose(&fr);
		return 0;
	}

	frseek(&fr, index*4);
	frread(&fr, &offsetToData, 4);

	frseek(&fr, offsetToData);
	frread(&fr, &realSize, 4);
	frread(&fr, &compSize, 4);
	frread(&fr, &mode, 2);

	if (!ptr)
		ptr = (uint8*)malloc(realSize);

	if (!ptr) {
		printf("\nHQR WARNING: Unable to allocate memory!!\n");
		frclose(&fr);
		return 0;
	}

	// uncompressed
	if (mode == 0) {
		frread(&fr, ptr, realSize);
	}
	// compressed: modes (1 & 2)
	else {
		uint8* compDataPtr = 0;
		compDataPtr = (uint8*)malloc(compSize);
		frread(&fr, compDataPtr, compSize);
		hqr_decompress_entry(ptr, compDataPtr, realSize, mode);
		free(compDataPtr);
	}

	frclose(&fr);

	return realSize;
}

/** Get a HQR entry pointer
	@param filename HQR file name
	@param index entry index to extract
	@return entry real size */
int hqr_entry_size(int8 *filename, int32 index) {
	uint32 headerSize;
	uint32 offsetToData;
	uint32 realSize;

	if (!filename)
		return 0;

	if (!fropen(&fr, filename)) {
		printf("HQR: %s can't be found !\n", filename);
		exit(1);
	}

	frread(&fr, &headerSize, 4);

	if ((uint32)index >= headerSize / 4) {
		printf("\nHQR WARNING: Invalid entry index!!\n");
		frclose(&fr);
		return 0;
	}

	frseek(&fr, index*4);
	frread(&fr, &offsetToData, 4);

	frseek(&fr, offsetToData);
	frread(&fr, &realSize, 4);

	frclose(&fr);

	return realSize;
}

/** Get a HQR total number of entries
	@param filename HQR file name
	@return total number of entries */
int hqr_num_entries(int8 *filename) {
	uint32 headerSize;

	if (!filename)
		return 0;

	if (!fropen(&fr, filename)) {
		printf("HQR: %s can't be found !\n", filename);
		exit(1);
	}

	frread(&fr, &headerSize, 4);

	return headerSize / 4;
}

/** Get a HQR entry pointer with memory allocation
	@param ptr pointer to save the entry
	@param filename HQR file name
	@param index entry index to extract
	@return entry real size */
int32 hqr_getalloc_entry(uint8 ** ptr, int8 *filename, int32 index) {
	int32 size;
	size = hqr_entry_size(filename, index);

	*ptr = (uint8*)malloc(size * sizeof(uint8));
	if (!*ptr) {
		printf("HQR WARNING: unable to allocate entry memory!!\n");
		return 0;
	}
	hqr_get_entry(*ptr, filename, index);

	return size;
}