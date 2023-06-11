#include <iostream>
#include "EGP_Converter.h"
#include "EGP_IniRead.h"
#include "EGP_Image2Raw.h"
#include "EGP_Compressors.h"
#include <Magick++.h>

using namespace std;

void EgpConverter_Ini2Img(UEFI_GOP_CONVERT_FILEINPUTINFO *ptFileInput)
{
	FILE* pfOutput;
	UEFI_GOP_PICTURE_HEADER tHeader;
	const UINT8* pImgData = nullptr;
	const UINT8* pImgDataPrevious = nullptr; /* overlay */
	UINT8* pOriginalImgData = nullptr;
	UINT32 nCurrentOffset;
	UINT32 nSize;
	UINT32 nCompSize;
	fopen_s(&pfOutput, (char*)ptFileInput->pszFn, "wb");
	if (ptFileInput->bIsAnimation == true)
		tHeader.dwMagic = UEFI_GOP_PICTURE_HEADER_MAGIC_ANIMATION;
	else
		tHeader.dwMagic = UEFI_GOP_PICTURE_HEADER_MAGIC_IMAGE;
	tHeader.nWidth = ptFileInput->nWidth;
	tHeader.nHeight = ptFileInput->nHeight;
	tHeader.tVersion = { 1, 0 };
	tHeader.tProperties.nBpp = (UINT8)ptFileInput->tBpp;
	tHeader.tProperties.bIsTransparent = ptFileInput->bIsTransparent;
	tHeader.tProperties.bIsAnimated = ptFileInput->bIsAnimation;
	tHeader.tProperties.bIsRotated = ptFileInput->bIsRotated;
	tHeader.tProperties.bUseColorForTransparency = ptFileInput->bIsUsingColorForTransparency;
	tHeader.nNumFrames = (UINT16)ptFileInput->vtImageInfo.size();
	fwrite(&tHeader, sizeof(tHeader), 1, pfOutput);
	fseek(pfOutput, sizeof(UINT32)*tHeader.nNumFrames, SEEK_CUR); /* set offset by frames * 4 */
	for (UINT16 i = 0; i < tHeader.nNumFrames; i++) {
		UEFI_GOP_PICTURE_IMAGE tImageInfo;
		nCurrentOffset = ftell(pfOutput);
		fseek(pfOutput, sizeof(UEFI_GOP_PICTURE_HEADER) + (i * 4), SEEK_SET);
		fwrite(&nCurrentOffset, sizeof(nCurrentOffset), 1, pfOutput);
		fseek(pfOutput, nCurrentOffset, SEEK_SET);
		nSize = ptFileInput->nWidth * ptFileInput->nHeight * (EgpConverter_GetBpp(ptFileInput->tBpp) / 8);
		tImageInfo.nSpeed = ptFileInput->vtImageInfo[i].nSpeed;
		tImageInfo.tCompression = (UINT8)ptFileInput->vtImageInfo[i].nCompression;
		tImageInfo.nPaletteCount = 0;
		pImgDataPrevious = new UINT8[nSize];
		if (i == 0)
		{
			memset((void*)pImgDataPrevious, 0, nSize);
		}
		else {
			memcpy_s((void*)pImgDataPrevious, nSize, pOriginalImgData, nSize);
		}	
		pOriginalImgData = EgpConverter_Img2Raw(ptFileInput->vtImageInfo[i].pszFn, ptFileInput->tBpp);
		if ((UEFI_GOP_PICTURE_COMPRESSION)tImageInfo.tCompression == UEFI_GOP_PICTURE_COMPRESSION::UEFI_GOP_PICTURE_COMPRESSION_UNCOMPRESSED)
		{
			pImgData = pOriginalImgData;
			tImageInfo.nBuffSize = nSize;
		} else {
			pImgData = EgpConverter_Compress(pImgDataPrevious, pOriginalImgData, nSize, &nCompSize, (UEFI_GOP_PICTURE_COMPRESSION)tImageInfo.tCompression, EgpConverter_GetBpp(ptFileInput->tBpp));
			tImageInfo.nBuffSize = nCompSize;
		}
		fwrite(&tImageInfo, sizeof(UEFI_GOP_PICTURE_IMAGE), 1, pfOutput);
		fwrite(pImgData, tImageInfo.nBuffSize, 1, pfOutput);
	}
	fclose(pfOutput);
}

UINT32 main(UINT32 argc, char *argv[]) {
	UEFI_GOP_CONVERT_FILEINPUTINFO tFileInput;
	Magick::InitializeMagick(*argv);
	if (argc == 1)
	{
		printf_s("Failure, no INI file present");
		return 1;
	}
	EgpConverter_ReadIni(argv[1], &tFileInput);
	EgpConverter_Ini2Img(&tFileInput);
	return 0;
}