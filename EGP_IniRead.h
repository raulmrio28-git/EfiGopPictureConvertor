#include "IniRead.h"
#include <vector>

using UEFI_GOP_CONVERT_IMAGEINFO = struct {
	std::string pszFn;
	UEFI_GOP_PICTURE_COMPRESSION nCompression;
	UINT16 nSpeed;
};

using UEFI_GOP_CONVERT_FILEINPUTINFO = struct {
	UINT8* pszFn;
	UINT32 nWidth;
	UINT32 nHeight;
	BOOLEAN bIsAnimation;
	UEFI_GOP_PICTURE_BPP tBpp;
	BOOLEAN bIsTransparent;
	BOOLEAN bIsRotated;
	BOOLEAN bIsUsingColorForTransparency;
	std::vector<UEFI_GOP_CONVERT_IMAGEINFO> vtImageInfo;
};

void EgpConverter_ReadIni(const char *pszIniFilename, UEFI_GOP_CONVERT_FILEINPUTINFO *ptFileInfo);