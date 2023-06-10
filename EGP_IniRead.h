#include "IniRead.h"
#include <vector>
#include <GraphicsLib_Private.h>

typedef struct {
	std::string pszFn;
	UEFI_GOP_PICTURE_COMPRESSION nCompression;
	UINT16 nSpeed;
} UEFI_GOP_CONVERT_IMAGEINFO;

typedef struct {
	UINT8* pszFn;
	UINT32 nWidth;
	UINT32 nHeight;
	BOOLEAN bIsAnimation;
	UEFI_GOP_PICTURE_BPP tBpp;
	BOOLEAN bIsTransparent;
	BOOLEAN bIsRotated;
	BOOLEAN bIsUsingColorForTransparency;
	std::vector<UEFI_GOP_CONVERT_IMAGEINFO> vtImageInfo;
} UEFI_GOP_CONVERT_FILEINPUTINFO;

void EgpConverter_ReadIni(char *pszIniFilename, UEFI_GOP_CONVERT_FILEINPUTINFO *ptFileInfo);