#include <Windows.h>
#include <GraphicsLib_Private.h>
#pragma pack(1)
typedef struct {
	UINT8 nRed;
	UINT8 nGreen;
	UINT8 nBlue;
	UINT8 nAlpha;
} UEFI_GOP_CONVERT_RGBA;
#pragma pack()

UINT8 *EgpConverter_Img2Raw(const std::string &filename, UEFI_GOP_PICTURE_BPP tBpp);
UINT32 EgpConverter_GetBpp(UEFI_GOP_PICTURE_BPP tBppType);