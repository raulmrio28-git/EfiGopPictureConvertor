#ifndef _GRAPHICS_LIB_H_
#define _GRAPHICS_LIB_H_

#include <Windows.h>
#define WIN32_LEAN_AND_MEAN

enum class UEFI_GOP_PICTURE_BPP {
	UEFI_GOP_PICTURE_BPP_1P,
	UEFI_GOP_PICTURE_BPP_1BW,
	UEFI_GOP_PICTURE_BPP_4,
	UEFI_GOP_PICTURE_BPP_8P,
	UEFI_GOP_PICTURE_BPP_8RGB332,
	UEFI_GOP_PICTURE_BPP_8RGBA3328,
	UEFI_GOP_PICTURE_BPP_16RGB444,
	UEFI_GOP_PICTURE_BPP_16RGBA4444,
	UEFI_GOP_PICTURE_BPP_16RGB555,
	UEFI_GOP_PICTURE_BPP_16RGBA5551,
	UEFI_GOP_PICTURE_BPP_16RGB565,
	UEFI_GOP_PICTURE_BPP_16RGBA5658,
	UEFI_GOP_PICTURE_BPP_18RGB666,
	UEFI_GOP_PICTURE_BPP_18RGBA6666,
	UEFI_GOP_PICTURE_BPP_24RGB888,
	UEFI_GOP_PICTURE_BPP_24RGBA8888
};

enum class UEFI_GOP_PICTURE_COMPRESSION {
	UEFI_GOP_PICTURE_COMPRESSION_UNCOMPRESSED,		// No compression
	UEFI_GOP_PICTURE_COMPRESSION_RLE,				// Run - length encoding
	UEFI_GOP_PICTURE_COMPRESSION_LZSS,				// LZ55 compression (used in some formats)
	UEFI_GOP_PICTURE_COMPRESSION_PACKBITS8,			// 8-bit packbits compression
	UEFI_GOP_PICTURE_COMPRESSION_PACKBITS16,		// 16-bit packbits compression
	UEFI_GOP_PICTURE_COMPRESSION_EFICOMPRESS,		// UEFI compression
	UEFI_GOP_PICTURE_COMPRESSION_RLE_AND_XOR,		// Run-length encoding + XOR
	UEFI_GOP_PICTURE_COMPRESSION_OVERLAY,			// Image overlay
	UEFI_GOP_PICTURE_COMPRESSION_PREVREUSE,			// Reuses pixel data from the previously decoded image buffer
	UEFI_GOP_PICTURE_COMPRESSION_ZLIB				// ZLIB compression
};

#pragma pack(1)
using UEFI_GOP_PICTURE_PROPERTIES = struct {
	UINT8 nBpp : 4;       // 4-bit color format (enum)
	UINT8 bIsTransparent : 1;   // 1-bit is transparent boolean
	UINT8 bUseColorForTransparency : 1;     // 1-bit uses RGB color or alpha values for transparency boolean 
	UINT8 bIsAnimated : 1;     // 1-bit is animated boolean
	UINT8 bIsRotated : 1;     // 1-bit is rotated boolean
};
#pragma pack()
constexpr auto UEFI_GOP_PICTURE_HEADER_MAGIC_IMAGE = 0x49504745; /* EGPI in hex */
constexpr auto UEFI_GOP_PICTURE_HEADER_MAGIC_ANIMATION = 0x41504745; /* EGPA in hex */

#pragma pack(1)
using UEFI_GOP_PICTURE_HEADER = struct {
	UINT32 dwMagic;     // "EGPA" for animation, "EGPI" for image
	UINT32 nWidth;         // Width of image/frame
	UINT32 nHeight;        // Height of image/frame
	struct {
		UINT8 nMajor : 4;     // 4-bit major version
		UINT8 nMinor : 4;     // 4-bit minor version
	} tVersion;
	UEFI_GOP_PICTURE_PROPERTIES tProperties;
	UINT16 nNumFrames;    // Number of frames (if animated)
};
#pragma pack()
#pragma pack(1)
using UEFI_GOP_PICTURE_IMAGE = struct {
	UINT32 nBuffSize;		// Size of image data buffer
	UINT16 nSpeed;			   // Speed (0 is not animated)
	UINT8 tCompression;		// Compression type
	UINT8 nPaletteCount;		// Number of colors (0 = 1 color, if <= 8bpp)
								// Transparent color index (8bpp RGB332)
};
#pragma pack()
#endif /* _GRAPHICS_LIB_H_ */
