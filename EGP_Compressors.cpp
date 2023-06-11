//LZSS copyright notice:
/**************************************************************
 LZSS.C -- A Data Compression Program
***************************************************************
	4/6/1989 Haruhiko Okumura
	Use, distribute, and modify this program freely.
	Please send me your improved versions.
		PC-VAN      SCIENCE
		NIFTY-Serve PAF01022
		CompuServe  74050,1022
**************************************************************/
//ZLIB copyright notice:
/* Copyright notice:

 (C) 1995-2022 Jean-loup Gailly and Mark Adler

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Jean-loup Gailly        Mark Adler
  jloup@gzip.org          madler@alumni.caltech.edu */
#include <iostream>
#include "EGP_Converter.h"
#include "EGP_IniRead.h"
#include "EGP_Compressors.h"
#include "Compress.h"
#include "zlib/zlib.h"

//===========================Defines.=============================
#define N		 4096	/* size of ring buffer */
#define F		   18	/* upper limit for match_length */
#define THRESHOLD	2   /* encode string UINT32o position and length
						   if match_length is greater than this */
#define NIL			N	/* index for root of binary search trees */

UINT32 textsize = 0;	/* text size counter */
UINT32 codesize = 0;	/* code size counter */
UINT32 printcount = 0;	/* counter for reporting progress every 1K UINT8s */
UINT8 text_buf[N + F - 1];	/* ring buffer of size N,
	with extra F-1 UINT8s to facilitate string comparison */
UINT32 match_position;
UINT32 match_length;
/* of longest match.  These are
			set by the InsertNode() procedure. */
UINT32 lson[N + 1];
UINT32 rson[N + 257];
UINT32 dad[N + 1];  /* left & right children &
		parents -- These constitute binary search trees. */

// Read nBytes bytesfrom UINT8 bffer and return as an unsigned 32-bit UINT32eger
UINT32 ReadNBytes(const UINT8 *pBuffer, UINT8 nBytes)
{
	UINT32 nOut = 0;         // Output value
	UINT8 nCurrByte = 0;        // Current byte being read

	while (nCurrByte < nBytes && nCurrByte < sizeof(UINT32))
	{
		nOut |= pBuffer[nCurrByte] << (8 * nCurrByte); // Shift and OR in the next byte
		nCurrByte++;
	}

	return nOut;            // Return the output value
}

// Write an unsigned 32-bit value to UINT8 buffer using nBytes bytes
void WriteNBytes(UINT8 *pBuffer,UINT32 nValue, UINT8 nBytes) {
	for (UINT32 i = 0; i < nBytes; i++) {        // For each byte
		pBuffer[i] = nValue & 0xFF;           // Write the least significant byte
		nValue >>= 8;                         // Shift value right by 8 bits
	}
}

// RLE compress pInBuffer UINT32o pOutBuffer using nBpp bits per pixel 
void RleCompress(const UINT8 *pInBuffer,UINT32 nInSize, UINT8 *pOutBuffer,UINT32 *nOutSize, UINT8 nBpp) {
	UINT32 nInPos = 0;         // Position in input buffer
	UINT32 nOutPos = 0;        // Position in output buffer
	UINT16 nCount;            // Count of repeated pixel
	UINT32 nValue;            // Value of current pixel(s)

	while (nInPos < nInSize) { // While input position < input size
		nValue = ReadNBytes(pInBuffer + nInPos, nBpp / 8);   // Read the next pixel(s)
		nInPos += nBpp / 8;                               // Increment the input position by nBpp/8

		// Read the next run of pixels
		nCount = 1;                                      // Start count at 1
		while (nInPos < nInSize &&                        // While input position < input size
			ReadNBytes(pInBuffer + nInPos, nBpp / 8) == nValue && // next pixel = curr pixel
			nCount < 0xFFFF) {                         // count less than max unsigned 16-bit value (0xffff)
			nInPos += nBpp / 8;                           // Increment input position
			nCount++;                                  // Increment count
		}

		if (nCount > 1 || (nValue & 0xff) == 0x00 && nCount == 1)
		{ // If repeat count is over 1 or value's first byte is 0x00 (RLE flag) and appears once, write count
			pOutBuffer[nOutPos++] = 0x00;               // Write RLE flag (0x00)
			WriteNBytes(pOutBuffer + nOutPos, nCount, 2); // Write unsigned 16-bit count
			nOutPos += 2; //Self explainatory
		}

		WriteNBytes(pOutBuffer + nOutPos, nValue, nBpp / 8); // Write the pixel(s)
		nOutPos += nBpp / 8;                            // Increment the output position
	}

	*nOutSize = nOutPos;                              // Set output size to final position
}

// RLE decompress pInBuffer UINT32o pOutBuffer using nBpp bits per pixel
void RleDecompress(const UINT8 *pInBuffer,UINT32 nInSize, UINT8 *pOutBuffer,UINT32 *nOutSize, UINT8 nBpp) {
	UINT32 nInPos = 0;         // Position in input buffer
	UINT32 nOutPos = 0;        // Position in output buffer
	UINT16 nCount;            // Count of repeated bytes
	UINT32 nValue;            // Value of current byte(s)

	while (nInPos < nInSize) { // While input position < input size

		// Check if the next value is the RLE flag (0x00)
		if (pInBuffer[nInPos] == 0x00) {  //Value is RLE flag, time to read count
			nCount = (UINT16)ReadNBytes(pInBuffer + nInPos + 1, 2); // Read the run count
			nInPos += 3;  //3 = size of flag + size of count

			nValue = ReadNBytes(pInBuffer + nInPos, nBpp / 8); // Read the pixel to repeat
			nInPos += nBpp / 8;

			// Write the repeated pixel nCount times
			while (nCount--)
			{
				WriteNBytes(pOutBuffer + nOutPos, nValue, nBpp / 8);
				nOutPos += nBpp / 8;
			}
		}
		else
		{  //Nope, we encounter a pixel
			// Move back a byte
			nInPos--;

			// Read and write the pixel(s)
			nValue = ReadNBytes(pInBuffer + nInPos, nBpp / 8);
			WriteNBytes(pOutBuffer + nOutPos, nValue, nBpp / 8);
			nInPos += nBpp / 8;
			nOutPos += nBpp / 8;
		}
	}

	*nOutSize = nOutPos;                          // Set output size to final position
}

// RLE compress pInBuffer UINT32o pOutBuffer using nBpp bits per pixel, with 10-bit counts
void Rle16Compress(const UINT8 *pInBuffer,UINT32 nInSize, UINT8 *pOutBuffer,UINT32 *nOutSize, UINT8 nBpp) {
	UINT32 nInPos = 0;         // Position in input buffer
	UINT32 nOutPos = 0;        // Position in output buffer
	UINT16 nCount;            // Count of repeated bytes
	UINT16 nCountBs;           // Count with flag bit set
	UINT32 nValue;            // Value of current byte(s)

	while (nInPos < nInSize) { // While input position < input size
		nValue = ReadNBytes(pInBuffer + nInPos, nBpp / 8);   // Read the next pixel(s)
		nInPos += nBpp / 8;                               // Increment the input position by nBpp/8

		// Read the next run of pixels
		nCount = 1;                                      // Start count at 1
		while (nInPos < nInSize &&                        // While input position < input size
			ReadNBytes(pInBuffer + nInPos, nBpp / 8) == nValue && // next pixel = curr pixel
			nCount < 1023) {                           // count less than max unsigned 10-bit value (0x3ff)
			nInPos += nBpp / 8;                           // Increment input position
			nCount++;                                  // Increment count
		}

		nCountBs = (UINT16)(nCount << 6) | 0x20;                // Set count write value as nCount left bitshifted by 6 bits with flag bit (0x20)
		if (nCount > 1 || (nValue & 0x20) && nCount == 1)  // If repeat count is over 1 or value has flag bit (0x20) set and appears once, write count
		{
			WriteNBytes(pOutBuffer + nOutPos, nCountBs, 2);  // Write bitshifted with flag count
			nOutPos += 2;
		}

		WriteNBytes(pOutBuffer + nOutPos, nValue, nBpp / 8); // Write the pixel(s)
		nOutPos += nBpp / 8;                            // Increment the output position
	}

	*nOutSize = nOutPos;                              // Set output size to final position  
}

// RLE decompress pInBuffer UINT32o pOutBuffer using nBpp bits per pixel, with 10-bit counts
void Rle16Decompress(const UINT8 *pInBuffer,UINT32 nInSize, UINT8 *pOutBuffer,UINT32 *nOutSize, UINT8 nBpp) {
	UINT32 nInPos = 0;         // Position in input buffer
	UINT32 nOutPos = 0;        // Position in output buffer
	UINT16 nCount;            // Count of repeated bytes
	UINT16 nCountBs;           // Count without flag bit set
	UINT32 nValue;            // Value of current byte(s)

	while (nInPos < nInSize) { // While input position < input size
		nCountBs = (UINT16)ReadNBytes(pInBuffer + nInPos, 2);  // Read count value
		nInPos += 2;  // Increment it by size of 16-bit value
		if (nCountBs & 0x20) {  // Value has has flag bit (0x20) set, It has to be a count
			nValue = ReadNBytes(pInBuffer + nInPos, nBpp / 8);  // Read the pixel to repeat
			nInPos += nBpp / 8;
			nCount = nCountBs >> 6;  // The repeat count is left shifted 6 bits

			 // Write the repeated pixel nCount times
			while (nCount--) {
				WriteNBytes(pOutBuffer + nOutPos, nValue, nBpp / 8);  // Writes the repeated pixel
				nOutPos += nBpp / 8;  // Increment output position by nBpp/8
			}
		}
		else {  //Nope, we encounter a pixel
			nInPos -= 2;  // Go back 2 bytes
			nValue = ReadNBytes(pInBuffer + nInPos, nBpp / 8);  // Read the pixel to write
			WriteNBytes(pOutBuffer + nOutPos, nValue, nBpp / 8);  // Writes the pixel
			nInPos += nBpp / 8;    // Increment output position by nBpp/8
			nOutPos += nBpp / 8;   // Same here
		}
	}

	*nOutSize = nOutPos;  // Set the output size
}

void PackBitsCompress(const UINT8 *pInBuffer,UINT32 nInSize, UINT8 *pOutBuffer,UINT32 *nOutSize, UINT8 nBpp) {
	UINT32 nInPos = 0;         // Position in input buffer
	UINT32 nOutPos = 0;        // Position in output buffer
	UINT8 nRepCount = 1;      // Count of repeated bytes, Defaults to 1 
	UINT8 nCopCount = 0;      // Count of non-repeated bytes, Defaults to 0
	UINT8* pWhereCopCount = nullptr; // PoUINT32er to location to write nCopCount

	UINT32 nValue;            // Value of current byte(s) stored asUINT32.

	while (nInPos < nInSize) { // While input position < input size
		nValue = ReadNBytes(pInBuffer + nInPos, nBpp / 8);   // Read the next pixel
		nInPos += nBpp / 8;                               // Increment the input position by BPP/8

		nRepCount = 1;                                  // Start count of repeats at 1
		// Read the next run of pixels
		while (nInPos < nInSize &&                    // While input position < input size
			ReadNBytes(pInBuffer + nInPos, nBpp / 8) == nValue && // next pixel = current pixel
			nRepCount < 0x7F) {                   // count less than max signed 8-bit value 
			nInPos += nBpp / 8;                         // Increment input position
			nRepCount++;                              // Increment repeat count
		}

		if (nRepCount > 1) {                           // We have repeat count over 1
			WriteNBytes(pOutBuffer + (nOutPos++), nRepCount, 1); //Write count of repeat bytes without non-repeat flag
			nCopCount = 0;                                  // Reset non-repeat count to 0
		}
		else                                        // We have repeat count of 1, this means we should UINT32erprets unrepeated pixel(s) as literals
		{
			if (nCopCount == 0) {                    // If nCopCount = 0
				pWhereCopCount = pOutBuffer + nOutPos;   // Set poisition of nCopCount write
				nOutPos++;                            // Increment output position by 1 byte (size of 8bit value)
			}
			nCopCount++;                             // Increment the non-repeat count
			if (nCopCount == 0x7F) {                // If nCopCount reaches the limit (127)
				WriteNBytes(pWhereCopCount, 0xff, 1); //Write max count of nonrepeat bytes with non-repeat flag
				nCopCount = 0;                       // Reset nCopCount to 0
			}
			else
				WriteNBytes(pWhereCopCount, nCopCount | 0x80, 1); //Write count of nonrepeat bytes with non-repeat flag
		}
		WriteNBytes(pOutBuffer + nOutPos, nValue, nBpp / 8); // Write the pixel
		nOutPos += nBpp / 8;                            // Increment the output position by nBpp/8
	}

	*nOutSize = nOutPos;                              // Set output size to final position
}

void PackBitsDecompress(const UINT8 *pInBuffer,UINT32 nInSize, UINT8 *pOutBuffer,UINT32 *nOutSize, UINT8 nBpp) {
	UINT32 nInPos = 0;         // Position in input buffer
	UINT32 nOutPos = 0;        // Position in output buffer

	UINT8 nCount;            // Count of repeated or non-repeated bytes

	while (nInPos < nInSize) { // While input position < input size
		nCount = (UINT8)ReadNBytes(pInBuffer + nInPos, 1);   // Read the next byte count
		nInPos++;                               // Increment the input position by a byte

		if (nCount & 0x80) {                      // If high bit is set, we have a non-repeat count
			nCount &= 0x7F;                      // Mask out high bit
			while (nCount > 0) {                   // While count > 0
				WriteNBytes(pOutBuffer + nOutPos, ReadNBytes(pInBuffer + nInPos, nBpp / 8), nBpp / 8); // Write the byte(s)
				nInPos += nBpp / 8;                 // Increment input position by BPP/8
				nOutPos += nBpp / 8;                // Increment output position by BPP/8
				nCount--;                         // Decrement count
			}
		}
		else {                                   // Else we have a repeat count
			UINT32 nValue = ReadNBytes(pInBuffer + nInPos, nBpp / 8); // Read the next byte(s)
			nInPos += nBpp / 8;                     // Increment input position by BPP/8
			while (nCount > 0) {                   // While count > 0
				WriteNBytes(pOutBuffer + nOutPos, nValue, nBpp / 8); // Write the byte(s)
				nOutPos += nBpp / 8;                // Increment output position by BPP/8
				nCount--;                         // Decrement count
			}
		}
	}

	*nOutSize = nOutPos;                          // Set output size to final position
}

void PackBits16Compress(const UINT8 *pInBuffer,UINT32 nInSize, UINT8 *pOutBuffer,UINT32 *nOutSize, UINT8 nBpp) {
	UINT32 nInPos = 0;         // Position in input buffer
	UINT32 nOutPos = 0;        // Position in output buffer
	UINT16 nRepCount = 1;      // Count of repeated bytes, Defaults to 1 
	UINT16 nCopCount = 0;      // Count of non-repeated bytes, Defaults to 0
	UINT8* pWhereCopCount = nullptr; // PoUINT32er to location to write nCopCount

	UINT32 nValue;            // Value of current byte(s) stored asUINT32.

	while (nInPos < nInSize) { // While input position < input size
		nValue = ReadNBytes(pInBuffer + nInPos, nBpp / 8);   // Read the next pixel
		nInPos += nBpp / 8;                               // Increment the input position by BPP/8

		nRepCount = 1;                                  // Start count of repeats at 1
		// Read the next run of pixels
		while (nInPos < nInSize &&                    // While input position < input size
			ReadNBytes(pInBuffer + nInPos, nBpp / 8) == nValue && // next pixel = current pixel
			nRepCount < 0x7FFF) {                   // count less than max signed 16-bit value 
			nInPos += nBpp / 8;                         // Increment input position
			nRepCount++;                              // Increment repeat count
		}

		if (nRepCount > 1) {                           // We have repeat count over 1
			WriteNBytes(pOutBuffer + nOutPos, nRepCount, 2); //Write count of repeat bytes without non-repeat flag
			nOutPos += 2;
			nCopCount = 0;                                  // Reset non-repeat count to 0
		}
		else                                        // We have repeat count of 1, this means we should UINT32erprets unrepeated pixel(s) as literals
		{
			if (nCopCount == 0) {                    // If nCopCount = 0
				pWhereCopCount = pOutBuffer + nOutPos;   // Set poisition of nCopCount write
				nOutPos += 2;                            // Increment output position by 2 bytes (size of 16bit value)
			}
			nCopCount++;         // Increment the non-repeat count
			if (nCopCount == 0x7Fff) {                // If nCopCount reaches the limit (32767)
				WriteNBytes(pWhereCopCount, 0xffff, 2); //Write max count of nonrepeat bytes with non-repeat flag
				nCopCount = 0;                       // Reset nCopCount to 0
			}
			else
				WriteNBytes(pWhereCopCount, nCopCount | 0x8000, 2); //Write count of nonrepeat bytes with non-repeat flag
		}
		WriteNBytes(pOutBuffer + nOutPos, nValue, nBpp / 8); // Write the pixel
		nOutPos += nBpp / 8;                            // Increment the output position by nBpp/8
	}

	*nOutSize = nOutPos;                              // Set output size to final position
}

void PackBits16Decompress(const UINT8 *pInBuffer,UINT32 nInSize, UINT8 *pOutBuffer,UINT32 *nOutSize, UINT8 nBpp) {
	UINT32 nInPos = 0;         // Position in input buffer
	UINT32 nOutPos = 0;        // Position in output buffer

	UINT16 nCount;            // Count of repeated or non-repeated bytes

	while (nInPos < nInSize) { // While input position < input size
		nCount = (UINT16)ReadNBytes(pInBuffer + nInPos, 2);   // Read the next 2-byte count
		nInPos += 2;                               // Increment the input position by 2 bytes

		if (nCount & 0x8000) {                      // If high bit is set, we have a non-repeat count
			nCount &= 0x7FFF;                      // Mask out high bit
			while (nCount > 0) {                   // While count > 0
				WriteNBytes(pOutBuffer + nOutPos, ReadNBytes(pInBuffer + nInPos, nBpp / 8), nBpp / 8); // Write the byte(s)
				nInPos += nBpp / 8;                 // Increment input position by BPP/8
				nOutPos += nBpp / 8;                // Increment output position by BPP/8
				nCount--;                         // Decrement count
			}
		}
		else {                                   // Else we have a repeat count
			UINT32 nValue = ReadNBytes(pInBuffer + nInPos, nBpp / 8); // Read the next byte(s)
			nInPos += nBpp / 8;                     // Increment input position by BPP/8
			while (nCount > 0) {                   // While count > 0
				WriteNBytes(pOutBuffer + nOutPos, nValue, nBpp / 8); // Write the byte(s)
				nOutPos += nBpp / 8;                // Increment output position by BPP/8
				nCount--;                         // Decrement count
			}
		}
	}

	*nOutSize = nOutPos;                          // Set output size to final position
}

void OverlayDecompress(const UINT8 *pInBuffer,UINT32 nInSize, UINT8 *pOutBuffer, UINT8 nBpp) {
	UINT32 nInPos = 0;         // Position in input buffer
	UINT32 nSize = 0;      // Size of pixels
	UINT32 nOffset = 0;   // Offset
	UINT32 nCurrSize = 0;   // Current size
	UINT32 nEncounters;        //Encounter count
	UINT32 nCurrEncounter = 0;            // Current encounter
	nEncounters = ReadNBytes(pInBuffer, 4); //Read first 4 bytes (unsigned 32-bit value
	nInPos += 4;

	while (nInPos < nInSize && nCurrEncounter < nEncounters) { // While input position < input size
		nCurrEncounter = ReadNBytes(pInBuffer + nInPos, 4); //Read 4 bytes encounter (unsigned 32-bit value)
		nOffset = ReadNBytes(pInBuffer + nInPos + 4, 4); //Read 4 bytes offset (unsigned 32-bit value) and multiply it by nBpp/8
		nSize = ReadNBytes(pInBuffer + nInPos + 8, 4); //No multiplies needed
		printf_s("offset: %d, size: %d\n", nOffset, nSize);
		nInPos += 12;
		while (nCurrSize < nSize)
		{
			printf_s("nCurrSize: %d\n", nCurrSize);
			WriteNBytes(pOutBuffer + (nOffset*(nBpp / 8)) + (nCurrSize*(nBpp / 8)), ReadNBytes(pInBuffer + nInPos, nBpp / 8), nBpp / 8); // Write the byte(s)
			nInPos += nBpp / 8;
			nCurrSize++;
		}
	}
}

void OverlayCompress(const UINT8 *pInBuffer1, const UINT8 *pInBuffer2,UINT32 nSize, UINT8 *pOutBuffer,UINT32 *nOutSize, UINT8 nBpp) {
	UINT8* pOutTemp = pOutBuffer+4;
	UINT32 nInPos1 = 0;        // Position in input buffer 1
	UINT32 nInPos2 = 0;        // Position in input buffer 2
	UINT32 nOutPos = 4;        // Position in output buffer. It is set to 4 as first 4 bytes represent total encounters
	UINT32 nEncounter = 0;     // Encounter count
	UINT32 nOffset = 0;        // Offset
	UINT32 nCount = 0;         // Count of overlay pixels
	UINT32 nCurrCount = 0;     // Self expainatory

	while (nInPos1 < nSize && nInPos2 < nSize) { // While within input buffer size
		if (ReadNBytes(pInBuffer1 + nInPos1, nBpp / 8) != ReadNBytes(pInBuffer2 + nInPos2, nBpp / 8)) {
			// Pixels do not match, we have an overlay
			nOffset = nInPos1;                  // Set offset to current position in buffer 1
			nCount = 0;                        // Set overlay pixel count to 1
			nCurrCount = 0;						// BUG: Some overlay pixels are not written, so reset current count to 0
			while (nInPos1 < nSize && nInPos2 < nSize &&   // While within input buffer size
				ReadNBytes(pInBuffer1 + nInPos1, nBpp / 8) != ReadNBytes(pInBuffer2 + nInPos2, nBpp / 8)) {
				// Pixels do not match, increment overlay pixel count
				nInPos1 += nBpp / 8;
				nInPos2 += nBpp / 8;
				nCount++;
			}

			// Write the encounter, offset, and count to the output buffer
			WriteNBytes(pOutTemp, nEncounter, 4);
			nOutPos += 4;
			WriteNBytes(pOutTemp+4, nOffset / (nBpp / 8), 4);
			nOutPos += 4;
			WriteNBytes(pOutTemp+8, nCount, 4);
			nOutPos += 4;
			pOutTemp += 12;
			// Write the overlay pixel data to the output buffer
			while (nCurrCount < nCount) {
				WriteNBytes(pOutTemp, ReadNBytes(pInBuffer2 + nOffset + (nCurrCount*nBpp / 8), nBpp / 8), nBpp / 8);
				nOutPos += nBpp / 8;
				pOutTemp += nBpp / 8;
				nCurrCount++;
			}

			nEncounter++;  // Increment encounter count
			nOffset += nCount * nBpp / 8; // Increment offset by size of overlay block
			nInPos1 = nOffset;          // Set input position 1 to end of overlay block
			nInPos2 = nOffset;          // Set input position 2 to end of overlay block
		}
		else {
			// Pixels match, increment input positions
			nInPos1 += nBpp / 8;
			nInPos2 += nBpp / 8;
		}
	}
	WriteNBytes(pOutBuffer, nEncounter, 4); //write final encounter count

	*nOutSize = nOutPos;  // Set output size
}

void ReuseCompress(const UINT8 *source, UINT32 length, UINT8 *dest, UINT32 *outsize, UINT32 nBpp)
{
	UINT32 ch;
	const UINT8 *instart;
	const UINT8 *inptr;
	UINT8 *outptr = dest;
	const UINT8 *inscan;
	UINT16 *map;
	UINT8 *dat;
	UINT16 *pWhereCopCount = nullptr;
	const UINT8 *bestscan;
	UINT32 beststring;
	UINT32 maxstring;
	UINT32 string;
	UINT16 nCopCount = 0;
	UINT32 maplength = 0;
	UINT32 datlength = 0;

	map = new UINT16[length];
	dat = new UINT8[length];

	// set up
	bestscan = instart = inptr = source;

	// compress
	do {
		ch = ReadNBytes(inptr, nBpp / 8);

		// scan from start for patterns that match current data
		beststring = 0;
		for (inscan = instart; inscan < inptr; inscan += nBpp / 8) {
			if (ReadNBytes(inscan, nBpp / 8) != ch)
				continue;

			maxstring = (UINT32)(inptr - inscan) / (nBpp / 8);
			if (maxstring > 15)
				maxstring = 15;

			string = 1;
			while (ReadNBytes(inscan + (string * (nBpp / 8)), nBpp / 8) == ReadNBytes(inptr + (string * (nBpp / 8)), nBpp / 8) &&
				(inptr - inscan) < 1023 && string < maxstring)
				string++;

			if (string >= beststring) {
				beststring = string;
				bestscan = inscan;
			}
		}

		if (beststring > 1) {
			// repeated value!
			nCopCount = 0;
			map[maplength++] = (UINT16)(1 | ((inscan - bestscan) << 1) | beststring << 12);
			inptr += beststring * (nBpp / 8);
			length -= beststring * (nBpp / 8);
		}
		else {
			// no compression!
			if (nCopCount == 0) {
				pWhereCopCount = &map[maplength++];
			}
			nCopCount++;
			*pWhereCopCount = (UINT16)(nCopCount << 1);
			if (nCopCount == 0x7Fff) {
				nCopCount = 0;
			}
			WriteNBytes(dat + datlength, ReadNBytes(inptr, nBpp/8), nBpp/8);
			datlength += nBpp / 8;
			inptr += nBpp / 8;
			length -= nBpp / 8;
		}
	} while (length);

	WriteNBytes(outptr, maplength<<1, sizeof(UINT32));
	outptr += sizeof(UINT32);
	memcpy_s(outptr, maplength << 1, map, maplength << 1);
	outptr += maplength << 1;
	delete[] map;
	memcpy_s(outptr, datlength, dat, datlength);
	delete[] dat;
	*outsize = 4 + (maplength << 1) + datlength;
}

void InitTree(void)  /* initialize trees */
{
	UINT32  i;

	/* For i = 0 to N - 1, rson[i] and lson[i] will be the right and
	   left children of node i.  These nodes need not be initialized.
	   Also, dad[i] is the parent of node i.  These are initialized to
	   NIL (= N), which stands for 'not used.'
	   For i = 0 to 255, rson[N + i + 1] is the root of the tree
	   for strings that begin with character i.  These are initialized
	   to NIL.  Note there are 256 trees. */

	for (i = N + 1; i <= N + 256; i++) rson[i] = NIL;
	for (i = 0; i < N; i++) dad[i] = NIL;
}

void InsertNode(UINT32 r)
/* Inserts string of length F, text_buf[r..r+F-1], UINT32o one of the
   trees (text_buf[r]'th tree) and returns the longest-match position
   and length via the global variables match_position and match_length.
   If match_length = F, then removes the old node in favor of the new
   one, because the old one will be deleted sooner.
   Note r plays double role, as tree node and position in buffer. */
{
	INT32  i, p, cmp;
	const UINT8  *key;

	cmp = 1;  key = &text_buf[r];  p = N + 1 + key[0];
	rson[r] = lson[r] = NIL;  match_length = 0;
	for (; ; ) {
		if (cmp >= 0) {
			if (rson[p] != NIL) p = rson[p];
			else { rson[p] = r;  dad[r] = p;  return; }
		}
		else {
			if (lson[p] != NIL) p = lson[p];
			else { lson[p] = r;  dad[r] = p;  return; }
		}
		for (i = 1; i < F; i++)
			if ((cmp = key[i] - text_buf[p + i]) != 0)  break;
		if (i > match_length) {
			match_position = p;
			if ((match_length = i) >= F)  break;
		}
	}
	dad[r] = dad[p];  lson[r] = lson[p];  rson[r] = rson[p];
	dad[lson[p]] = r;  dad[rson[p]] = r;
	if (rson[dad[p]] == p) rson[dad[p]] = r;
	else                   lson[dad[p]] = r;
	dad[p] = NIL;  /* remove p */
}

void DeleteNode(UINT32 p)  /* deletes node p from tree */
{
	UINT32  q;

	if (dad[p] == NIL) return;  /* not in tree */
	if (rson[p] == NIL) q = lson[p];
	else if (lson[p] == NIL) q = rson[p];
	else {
		q = lson[p];
		if (rson[q] != NIL) {
			do { q = rson[q]; } while (rson[q] != NIL);
			rson[dad[q]] = lson[q];  dad[lson[q]] = dad[q];
			lson[q] = lson[p];  dad[lson[p]] = q;
		}
		rson[q] = rson[p];  dad[rson[p]] = q;
	}
	dad[q] = dad[p];
	if (rson[dad[p]] == p) rson[dad[p]] = q;  else lson[dad[p]] = q;
	dad[p] = NIL;
}
void LzssCompress(const UINT8 *in, UINT8 *out, UINT32 insize, UINT32 &outsize)
{
	UINT32  i;
	UINT8 c;
	UINT32 len = 0;
	UINT32 r;
	UINT32 s;
	UINT32 last_match_length;
	UINT32 code_buf_ptr;
	UINT8  code_buf[17];
	UINT8 mask;
	const UINT8 *inend = in + insize;
	InitTree();
	code_buf[0] = 0;
	code_buf_ptr = mask = 1;
	s = 0;  r = N - F;
	for (i = s; i < r; i++) text_buf[i] = 0xff; 
	while (len < F && in < inend)
	{
		text_buf[r + len] = *in++;  
		len++;
	}
	if ((textsize = len) == 0) return; 
	for (i = 1; i <= F; i++) InsertNode(r - i);
	InsertNode(r); 
	do {
		if (match_length > len) match_length = len;  
		if (match_length <= THRESHOLD) {
			match_length = 1;  /* Not long enough match.  Send one UINT8. */
			code_buf[0] |= mask;  /* 'send one UINT8' flag */
			code_buf[code_buf_ptr++] = text_buf[r];  /* Send uncoded. */
		}
		else {
			code_buf[code_buf_ptr++] = (UINT8)match_position;
			code_buf[code_buf_ptr++] = (UINT8)
				(((match_position >> 4) & 0xf0)
					| (match_length - (THRESHOLD + 1)));
		}
		if ((mask <<= 1) == 0) { 
			memcpy_s(out + outsize, code_buf_ptr, code_buf, code_buf_ptr);
			outsize += code_buf_ptr;
			codesize += code_buf_ptr;
			code_buf[0] = 0;  code_buf_ptr = mask = 1;
		}
		last_match_length = match_length;
		i = 0;
		while(i < last_match_length && in < inend) {
			DeleteNode(s);	
			c = *in++;
			text_buf[s] = c;
			if (s < F - 1) text_buf[s + N] = c;
			s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);
			InsertNode(r);	
			i++;
		}
		if ((textsize += i) > printcount) {
			printcount += 1024;
		}
		while (i++ < last_match_length) {	
			DeleteNode(s);				
			s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);
			if (--len) InsertNode(r);		
		}
	} while (len > 0);
	if (code_buf_ptr > 1) {	
		memcpy_s(out + outsize, code_buf_ptr, code_buf, code_buf_ptr);
		outsize += code_buf_ptr;
		codesize += code_buf_ptr;
	}
}

void XorTwoImgs(const UINT8 *pImgPrevious, const UINT8 *pImgCurr, UINT8 *pImgDest, UINT32 nSize)
{
	UINT32 nCurrPtr = 0;
	while (nCurrPtr < nSize)
	{
		pImgDest[nCurrPtr] = pImgPrevious[nCurrPtr] ^ pImgCurr[nCurrPtr];
		nCurrPtr++;
	}
}

UINT8 * EgpConverter_Compress(const UINT8 * pImgDataPrevious, UINT8 * pOriginalImgData,UINT32 nSize,UINT32* pnOutSize, UEFI_GOP_PICTURE_COMPRESSION tCompression, UINT8 nBpp)
{
	if (pImgDataPrevious == nullptr || pOriginalImgData == nullptr || nSize == 0 || pnOutSize == nullptr) return nullptr;
	auto *pCompressedData = new UINT8[nSize*2];
	if (tCompression == UEFI_GOP_PICTURE_COMPRESSION::UEFI_GOP_PICTURE_COMPRESSION_RLE)
	{
		RleCompress(pOriginalImgData, nSize, pCompressedData, pnOutSize, nBpp);
	}
	if (tCompression == UEFI_GOP_PICTURE_COMPRESSION::UEFI_GOP_PICTURE_COMPRESSION_LZSS)
	{
		LzssCompress(pOriginalImgData, pCompressedData, nSize, *pnOutSize);
	}
	if (tCompression == UEFI_GOP_PICTURE_COMPRESSION::UEFI_GOP_PICTURE_COMPRESSION_PACKBITS8)
	{
		PackBitsCompress(pOriginalImgData, nSize, pCompressedData, pnOutSize, nBpp);
	}
	if (tCompression == UEFI_GOP_PICTURE_COMPRESSION::UEFI_GOP_PICTURE_COMPRESSION_PACKBITS16)
	{
		PackBits16Compress(pOriginalImgData, nSize, pCompressedData, pnOutSize, nBpp);
	}
	if (tCompression == UEFI_GOP_PICTURE_COMPRESSION::UEFI_GOP_PICTURE_COMPRESSION_EFICOMPRESS)
	{
		EfiCompress(pOriginalImgData, nSize, pCompressedData, pnOutSize);
	}
	if (tCompression == UEFI_GOP_PICTURE_COMPRESSION::UEFI_GOP_PICTURE_COMPRESSION_RLE_AND_XOR)
	{
		auto* pImgDataXor = new UINT8[nSize];
		XorTwoImgs(pImgDataPrevious, pOriginalImgData, pImgDataXor, nSize);
		RleCompress(pImgDataXor, nSize, pCompressedData, pnOutSize, nBpp);
		delete[] pImgDataXor;
	}
	if (tCompression == UEFI_GOP_PICTURE_COMPRESSION::UEFI_GOP_PICTURE_COMPRESSION_OVERLAY)
	{
		OverlayCompress(pImgDataPrevious, pOriginalImgData, nSize, pCompressedData, pnOutSize, nBpp);
	}
	if (tCompression == UEFI_GOP_PICTURE_COMPRESSION::UEFI_GOP_PICTURE_COMPRESSION_PREVREUSE)
	{
		ReuseCompress(pOriginalImgData, nSize, pCompressedData, pnOutSize, nBpp);
	}
	if (tCompression == UEFI_GOP_PICTURE_COMPRESSION::UEFI_GOP_PICTURE_COMPRESSION_ZLIB)
	{
		if (compress2(pCompressedData, (uLongf *)pnOutSize, pOriginalImgData, nSize, 9) != Z_OK)
			return nullptr;
	}
	return pCompressedData;
}
