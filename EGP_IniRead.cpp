#include <iostream>
#include <string>
#include "EGP_Converter.h"
#include "EGP_IniRead.h"

UEFI_GOP_PICTURE_COMPRESSION GetCompressionType(const char* compression) {
	if (strcmp(compression, "NoCompress") == 0 || strcmp(compression, "Uncompressed") == 0)
		return UEFI_GOP_PICTURE_COMPRESSION::UEFI_GOP_PICTURE_COMPRESSION_UNCOMPRESSED;
	else if (strcmp(compression, "RLE") == 0 || strcmp(compression, "RunLengthEncoding") == 0)
		return UEFI_GOP_PICTURE_COMPRESSION::UEFI_GOP_PICTURE_COMPRESSION_RLE;
	else if (strcmp(compression, "LZSS") == 0)
		return UEFI_GOP_PICTURE_COMPRESSION::UEFI_GOP_PICTURE_COMPRESSION_LZSS;
	else if (strcmp(compression, "PackBits8") == 0)
		return UEFI_GOP_PICTURE_COMPRESSION::UEFI_GOP_PICTURE_COMPRESSION_PACKBITS8;
	else if (strcmp(compression, "PackBits16") == 0)
		return UEFI_GOP_PICTURE_COMPRESSION::UEFI_GOP_PICTURE_COMPRESSION_PACKBITS16;
	else if (strcmp(compression, "EfiCompress") == 0)
		return UEFI_GOP_PICTURE_COMPRESSION::UEFI_GOP_PICTURE_COMPRESSION_EFICOMPRESS;
	else if (strcmp(compression, "RleXor") == 0)
		return UEFI_GOP_PICTURE_COMPRESSION::UEFI_GOP_PICTURE_COMPRESSION_RLE_AND_XOR;
	else if (strcmp(compression, "Overlay") == 0)
		return UEFI_GOP_PICTURE_COMPRESSION::UEFI_GOP_PICTURE_COMPRESSION_OVERLAY;
	else if (strcmp(compression, "PrevReuse") == 0 || strcmp(compression, "PreviousReuse") == 0)
		return UEFI_GOP_PICTURE_COMPRESSION::UEFI_GOP_PICTURE_COMPRESSION_PREVREUSE;
	else if (strcmp(compression, "Zlib") == 0)
		return UEFI_GOP_PICTURE_COMPRESSION::UEFI_GOP_PICTURE_COMPRESSION_ZLIB;
}

UEFI_GOP_PICTURE_BPP GetBppType(const char* rgbFormat) {
	if (strcmp(rgbFormat, "1") == 0)
		return UEFI_GOP_PICTURE_BPP::UEFI_GOP_PICTURE_BPP_1P;
	if (strcmp(rgbFormat, "1BW") == 0 || strcmp(rgbFormat, "Monochrome") == 0)
		return UEFI_GOP_PICTURE_BPP::UEFI_GOP_PICTURE_BPP_1BW;
	if (strcmp(rgbFormat, "4") == 0)
		return UEFI_GOP_PICTURE_BPP::UEFI_GOP_PICTURE_BPP_4;
	if (strcmp(rgbFormat, "8") == 0)
		return UEFI_GOP_PICTURE_BPP::UEFI_GOP_PICTURE_BPP_8P;
	if (strcmp(rgbFormat, "RGB332") == 0)
		return UEFI_GOP_PICTURE_BPP::UEFI_GOP_PICTURE_BPP_8RGB332;
	if (strcmp(rgbFormat, "RGBA3328") == 0)
		return UEFI_GOP_PICTURE_BPP::UEFI_GOP_PICTURE_BPP_8RGBA3328;
	else if (strcmp(rgbFormat, "RGB444") == 0)
		return UEFI_GOP_PICTURE_BPP::UEFI_GOP_PICTURE_BPP_16RGB444;
	else if (strcmp(rgbFormat, "RGBA4444") == 0)
		return UEFI_GOP_PICTURE_BPP::UEFI_GOP_PICTURE_BPP_16RGBA4444;
	else if (strcmp(rgbFormat, "RGB555") == 0)
		return UEFI_GOP_PICTURE_BPP::UEFI_GOP_PICTURE_BPP_16RGB555;
	else if (strcmp(rgbFormat, "RGBA5551") == 0)
		return UEFI_GOP_PICTURE_BPP::UEFI_GOP_PICTURE_BPP_16RGBA5551;
	else if (strcmp(rgbFormat, "RGB565") == 0)
		return UEFI_GOP_PICTURE_BPP::UEFI_GOP_PICTURE_BPP_16RGB565;
	else if (strcmp(rgbFormat, "RGBA5658") == 0)
		return UEFI_GOP_PICTURE_BPP::UEFI_GOP_PICTURE_BPP_16RGBA5658;
	else if (strcmp(rgbFormat, "RGB666") == 0)
		return UEFI_GOP_PICTURE_BPP::UEFI_GOP_PICTURE_BPP_18RGB666;
	else if (strcmp(rgbFormat, "RGBA6666") == 0)
		return UEFI_GOP_PICTURE_BPP::UEFI_GOP_PICTURE_BPP_18RGBA6666;
	else if (strcmp(rgbFormat, "RGB888") == 0)
		return UEFI_GOP_PICTURE_BPP::UEFI_GOP_PICTURE_BPP_24RGB888;
	else if (strcmp(rgbFormat, "RGBA8888") == 0)
		return UEFI_GOP_PICTURE_BPP::UEFI_GOP_PICTURE_BPP_24RGBA8888;
}

void EgpConverter_ReadIni(const char *pszIniFilename, UEFI_GOP_CONVERT_FILEINPUTINFO *ptFileInfo)
{
	/* Initialize INI reader... */
	InitIniReader(pszIniFilename);

	// Check for [Output] section
	if (CheckIfSectionExists("Output")) {
		ptFileInfo->pszFn = (UINT8*)ReadString("Output", "File", "");
		ptFileInfo->nWidth = ReadInteger("Output", "Width", 0);
		ptFileInfo->nHeight = ReadInteger("Output", "Height", 0);
		ptFileInfo->bIsAnimation = ReadBooleanYesNo("Output", "IsAnimation", 0);
		ptFileInfo->tBpp = GetBppType(ReadString("Output", "RGBFormat", ""));
		ptFileInfo->bIsTransparent = ReadBooleanYesNo("Transparent", "Format", false);
		ptFileInfo->bIsRotated = ReadBooleanYesNo("IsRotated", "Format", false);
		if (ptFileInfo->bIsTransparent == true)
			ptFileInfo->bIsUsingColorForTransparency = ReadBooleanYesNo("TransparentUseColor", "Format", false);
		else
			ptFileInfo->bIsUsingColorForTransparency = false;
		printf_s("Width: %d\nHeight: %d\nAnimated? %s\nRGB format: %d\nTransparent? %s\nRotated? %s\n",
			ptFileInfo->nWidth, ptFileInfo->nHeight, ptFileInfo->bIsAnimation ? "True" : "False",
			ptFileInfo->tBpp, ptFileInfo->bIsTransparent ? "True" : "False", ptFileInfo->bIsRotated ? "True" : "False");
	}

	// Check for [Images] section
	if (CheckIfSectionExists("Images")) {
		UINT32 frameNum = 1;
		std::string section;
		char* image;
		section = "Frame" + std::to_string(frameNum);

		while ((image = ReadString("Images", section.c_str(), "")) && strnlen_s(image, INI_READ_MAX_STR_LENGTH)) {
			printf_s("Frame %d path: %s\n", frameNum, image);
			UEFI_GOP_CONVERT_IMAGEINFO info;
			info.pszFn = std::string(image);

			if (CheckIfSectionExists(section.c_str())) {
				info.nCompression = GetCompressionType(ReadString(section.c_str(), "Compression", ""));
				info.nSpeed = (UINT16)ReadInteger(section.c_str(), "Speed", 0);
			}

			ptFileInfo->vtImageInfo.push_back(info);
			frameNum++;
			section = "Frame" + std::to_string(frameNum);
		}
	}
}