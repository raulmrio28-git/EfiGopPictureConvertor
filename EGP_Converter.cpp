#include <iostream>
#include "EGP_IniRead.h"
#include "EGP_Image2Raw.h"
#include <GraphicsLib_Private.h>
#include <Magick++.h>

using namespace std;

void EgpConverter_Ini2Img(UEFI_GOP_CONVERT_FILEINPUTINFO *ptFileInput)
{
	FILE* pfOutput;
	UEFI_GOP_PICTURE_HEADER tHeader;
	UINT8* pImgData;
	UINT32 nCurrentOffset;
	UINT32 nSize;
	fopen_s(&pfOutput, (char*)ptFileInput->pszFn, "wb");
	if (ptFileInput->bIsAnimation == true)
		tHeader.dwMagic = UEFI_GOP_PICTURE_HEADER_MAGIC_ANIMATION;
	else
		tHeader.dwMagic = UEFI_GOP_PICTURE_HEADER_MAGIC_IMAGE;
	tHeader.nWidth = ptFileInput->nWidth;
	tHeader.nHeight = ptFileInput->nHeight;
	tHeader.tVersion = { 1, 0 };
	tHeader.tProperties.nBpp = ptFileInput->tBpp;
	tHeader.tProperties.bIsTransparent = ptFileInput->bIsTransparent;
	tHeader.tProperties.bIsAnimated = ptFileInput->bIsAnimation;
	tHeader.tProperties.bIsRotated = ptFileInput->bIsRotated;
	tHeader.tProperties.bUseColorForTransparency = ptFileInput->bIsUsingColorForTransparency;
	tHeader.nNumFrames = ptFileInput->vtImageInfo.size();
	fwrite(&tHeader, sizeof(tHeader), 1, pfOutput);
	fseek(pfOutput, sizeof(UINT32)*tHeader.nNumFrames, SEEK_CUR); /* set offset by frames * 4 */
	for (UINT16 i = 0; i < tHeader.nNumFrames; i++) {
		UEFI_GOP_PICTURE_IMAGE tImageInfo;
		nCurrentOffset = ftell(pfOutput);
		fseek(pfOutput, sizeof(UEFI_GOP_PICTURE_HEADER) + (i * 4), SEEK_SET);
		fwrite(&nCurrentOffset, sizeof(nCurrentOffset), 1, pfOutput);
		fseek(pfOutput, nCurrentOffset, SEEK_SET);
		pImgData = EgpConverter_Img2Raw(ptFileInput->vtImageInfo[i].pszFn, ptFileInput->tBpp);
		nSize = ptFileInput->nWidth * ptFileInput->nHeight * (EgpConverter_GetBpp(ptFileInput->tBpp)/8);
		tImageInfo.nBuffSize = nSize;
		tImageInfo.nSpeed = ptFileInput->vtImageInfo[i].nSpeed;
		tImageInfo.tCompression = ptFileInput->vtImageInfo[i].nSpeed;
		tImageInfo.nPaletteCount = 0;
		fwrite(&tImageInfo, sizeof(UEFI_GOP_PICTURE_IMAGE), 1, pfOutput);
		fwrite(pImgData, nSize, 1, pfOutput);
		free(pImgData);
	}
	fclose(pfOutput);
}

int main(int argc, char *argv[]) {
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