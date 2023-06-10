#include "EGP_IniRead.h"
#include "EGP_Image2Raw.h"
#include <Magick++.h>

UINT32 EgpConverter_GetBpp(UEFI_GOP_PICTURE_BPP tBppType)
{
	if (tBppType == UEFI_GOP_PICTURE_BPP_1P || tBppType == UEFI_GOP_PICTURE_BPP_1BW)
		return 1;
	if (tBppType == UEFI_GOP_PICTURE_BPP_4)
		return 4;
	if (tBppType == UEFI_GOP_PICTURE_BPP_8P || tBppType == UEFI_GOP_PICTURE_BPP_8RGB332)
		return 8;
	if(tBppType == UEFI_GOP_PICTURE_BPP_8RGBA3328 || tBppType == UEFI_GOP_PICTURE_BPP_16RGB444 || tBppType == UEFI_GOP_PICTURE_BPP_16RGBA4444 || tBppType == UEFI_GOP_PICTURE_BPP_16RGB555 || tBppType == UEFI_GOP_PICTURE_BPP_16RGBA5551 || tBppType == UEFI_GOP_PICTURE_BPP_16RGB565 )
		return 16;
	if (tBppType == UEFI_GOP_PICTURE_BPP_16RGBA5658 || tBppType == UEFI_GOP_PICTURE_BPP_18RGB666 || tBppType == UEFI_GOP_PICTURE_BPP_18RGBA6666 || tBppType == UEFI_GOP_PICTURE_BPP_24RGB888)
		return 24;
	if (tBppType == UEFI_GOP_PICTURE_BPP_24RGBA8888)
		return 32;
	else
		return 24;
}

/* Converts an image file using Magick++ to a UINT8 array with image in raw <tBpp> form */
/* This is Img2Raw + BPP conversion integrated to fix error regarding memory access */
UINT8 *EgpConverter_Img2Raw(const std::string &filename, UEFI_GOP_PICTURE_BPP tBpp)
{
	Magick::Image img;
	UEFI_GOP_CONVERT_RGBA tPixel;
	UINT8* pOut;
	unsigned int nCurrPixel = 0;
	try {
		img.read(filename);
	}
	catch (const Magick::Exception& e) {
		printf("%s\n", e.what());
		return NULL;
	}
	img.type(Magick::TrueColorAlphaType);
	const Magick::Quantum *pixels = img.getConstPixels(0, 0, img.columns(), img.rows());
	pOut = (UINT8*)malloc(img.columns() * img.rows()* EgpConverter_GetBpp(tBpp));
	for (unsigned i = 0; i < img.rows(); i++) {
		for (unsigned j = 0; j < img.columns(); j++) {
			nCurrPixel = img.columns() * i + j;
			tPixel.nRed = *pixels++;
			tPixel.nGreen = *pixels++;
			tPixel.nBlue = *pixels++;
			tPixel.nAlpha = *pixels++;
			switch (tBpp) {
			case UEFI_GOP_PICTURE_BPP_8RGB332:
				pOut[nCurrPixel] = ((tPixel.nBlue >> 6) & 0x3) | ((tPixel.nGreen >> 5) & 0x7) << 2 | ((tPixel.nRed >> 5) & 0x7) << 5;
				break;
			case UEFI_GOP_PICTURE_BPP_8RGBA3328:
				pOut[(nCurrPixel * 2)] = ((tPixel.nBlue >> 6) & 0x3) | ((tPixel.nGreen >> 5) & 0x7) << 2 | ((tPixel.nRed >> 5) & 0x7) << 5;
				pOut[(nCurrPixel * 2) + 1] = tPixel.nAlpha;
				break;
			case UEFI_GOP_PICTURE_BPP_16RGB444:
				pOut[(nCurrPixel * 2)] = ((tPixel.nBlue >> 4) & 0xf) | ((tPixel.nGreen >> 4) & 0xf) << 4;
				pOut[(nCurrPixel * 2) + 1] = ((tPixel.nRed >> 4) & 0xf);
				break;
			case UEFI_GOP_PICTURE_BPP_16RGBA4444 :
				pOut[(nCurrPixel * 2)] = ((tPixel.nBlue >> 4) & 0xf) | ((tPixel.nGreen >> 4) & 0xf) << 4;
				pOut[(nCurrPixel * 2) + 1] = ((tPixel.nRed >> 4) & 0xf) || ((tPixel.nAlpha >>4) & 0xf) << 4;
				break;
			case UEFI_GOP_PICTURE_BPP_16RGB555:
				pOut[(nCurrPixel * 2)] = ((tPixel.nBlue >> 3) & 0x1f) | ((tPixel.nGreen >> 3) & 0x7) << 5;
				pOut[(nCurrPixel * 2) + 1] = (((tPixel.nGreen >> 3) & 0x18) >> 3) | ((tPixel.nRed >> 3) & 0x1f) << 2;
				break;
			case UEFI_GOP_PICTURE_BPP_16RGBA5551:
				pOut[(nCurrPixel * 2)] = ((tPixel.nBlue >> 3) & 0x1f) | ((tPixel.nGreen >> 3) & 0x7) << 5;
				pOut[(nCurrPixel * 2) + 1] = (((tPixel.nGreen >> 3) & 0x18) >> 3) | ((tPixel.nRed >> 3) & 0x1f) << 2 | ((tPixel.nAlpha >> 7)& 0x1) << 7;
				break;
			case UEFI_GOP_PICTURE_BPP_16RGB565:
				pOut[(nCurrPixel * 2)] = ((tPixel.nBlue >> 3) & 0x1f) | ((tPixel.nGreen >> 2) & 0x7) << 5;
				pOut[(nCurrPixel * 2) + 1] = (((tPixel.nGreen >> 2) & 0x38) >> 3) | ((tPixel.nRed >> 3) & 0x1f) << 3;
				break;
			case UEFI_GOP_PICTURE_BPP_18RGB666:
				pOut[(nCurrPixel * 3)] = ((tPixel.nBlue >> 2) & 0x3f) | ((tPixel.nGreen >> 2) & 0x3) << 6;
				pOut[(nCurrPixel * 3)+1] = (((tPixel.nGreen >> 2) & 0x3c) >> 2) | ((tPixel.nRed >> 2) & 0xf) << 4;
				pOut[(nCurrPixel * 3) + 2] = (((tPixel.nRed >> 2) & 0x30) >> 4);
				break;
			case UEFI_GOP_PICTURE_BPP_18RGBA6666:
				pOut[(nCurrPixel * 3)] = ((tPixel.nBlue >> 2) & 0x3f) | ((tPixel.nGreen >> 2) & 0x3) << 6;
				pOut[(nCurrPixel * 3) + 1] = (((tPixel.nGreen >> 2) & 0x3c) >> 2) | ((tPixel.nRed >> 2) & 0xf) << 4;
				pOut[(nCurrPixel * 3) + 2] = (((tPixel.nRed >> 2) & 0x30) >> 4) || ((tPixel.nAlpha >> 2) & 0x3f) << 2;
				break;
			case UEFI_GOP_PICTURE_BPP_24RGB888:
				pOut[(nCurrPixel * 3) + 2] = tPixel.nRed;
				pOut[(nCurrPixel * 3) + 1] = tPixel.nGreen;
				pOut[(nCurrPixel * 3)] = tPixel.nBlue;
				break;
			case UEFI_GOP_PICTURE_BPP_24RGBA8888:
				pOut[(nCurrPixel * 4) + 3] = tPixel.nRed;
				pOut[(nCurrPixel * 4) + 2] = tPixel.nGreen;
				pOut[(nCurrPixel * 4) + 1] = tPixel.nBlue;
				pOut[(nCurrPixel * 4)] = tPixel.nAlpha;
				break;
			}
		}
	}
	return pOut;
}