/** @file

Copyright (c) 2004 - 2008, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Compress.h

Abstract:

  Header file for compression routine.
  Providing both EFI and Tiano Compress algorithms.
  
**/

#ifndef _COMPRESS_H_
#define _COMPRESS_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <Windows.h>
#define WIN32_LEAN_AND_MEAN
#include <string.h>
#include <stdlib.h>

//#include "CommonLib.h"
//#include <Common/UefiBaseTypes.h>
#define EFI_STATUS UINT32
//
// Modifiers to abstract standard types to aid in debug of problems
//
#define CONST     const
#define STATIC    static
#define VOID      void

//
// Modifiers for Data Types used to self document code.
// This concept is borrowed for UEFI specification.
//
#ifndef IN
//
// Some other environments use this construct, so #ifndef to prevent
// multiple definition.
//
#define IN
#define OUT
#define OPTIONAL
#endif

//
// Constants. They may exist in other build structures, so #ifndef them.
//
#ifndef TRUE
//
// BugBug: UEFI specification claims 1 and 0. We are concerned about the
//  compiler portability so we did it this way.
//
#define TRUE  ((BOOLEAN)(1==1))
#endif

#ifndef FALSE
#define FALSE ((BOOLEAN)(0==1))
#endif

#ifndef NULL
#define NULL  ((VOID *) 0)
#endif

#define MAX_BIT  ((UINT64)((1ULL << (sizeof (INT64) * 8 - 1))))
#define PACKED
#define ENCODE_ERROR(a)              ((EFI_STATUS)(MAX_BIT | (a)))

#define ENCODE_WARNING(a)            ((EFI_STATUS)(a))
#define EFI_ERROR(a)              (((INT64)(EFI_STATUS)(a)) < 0)
#define EFI_SUCCESS               0
#define EFI_LOAD_ERROR            ENCODE_ERROR (1)
#define EFI_INVALID_PARAMETER     ENCODE_ERROR (2)
#define EFI_UNSUPPORTED           ENCODE_ERROR (3)
#define EFI_BAD_BUFFER_SIZE       ENCODE_ERROR (4)
#define EFI_BUFFER_TOO_SMALL      ENCODE_ERROR (5)
#define EFI_NOT_READY             ENCODE_ERROR (6)
#define EFI_DEVICE_ERROR          ENCODE_ERROR (7)
#define EFI_WRITE_PROTECTED       ENCODE_ERROR (8)
#define EFI_OUT_OF_RESOURCES      ENCODE_ERROR (9)
#define EFI_VOLUME_CORRUPTED      ENCODE_ERROR (10)
#define EFI_VOLUME_FULL           ENCODE_ERROR (11)
#define EFI_NO_MEDIA              ENCODE_ERROR (12)
#define EFI_MEDIA_CHANGED         ENCODE_ERROR (13)
#define EFI_NOT_FOUND             ENCODE_ERROR (14)
#define EFI_ACCESS_DENIED         ENCODE_ERROR (15)
#define EFI_NO_RESPONSE           ENCODE_ERROR (16)
#define EFI_NO_MAPPING            ENCODE_ERROR (17)
#define EFI_TIMEOUT               ENCODE_ERROR (18)
#define EFI_NOT_STARTED           ENCODE_ERROR (19)
#define EFI_ALREADY_STARTED       ENCODE_ERROR (20)
#define EFI_ABORTED               ENCODE_ERROR (21)
#define EFI_ICMP_ERROR            ENCODE_ERROR (22)
#define EFI_TFTP_ERROR            ENCODE_ERROR (23)
#define EFI_PROTOCOL_ERROR        ENCODE_ERROR (24)
#define EFI_INCOMPATIBLE_VERSION  ENCODE_ERROR (25)
#define EFI_SECURITY_VIOLATION    ENCODE_ERROR (26)
#define EFI_CRC_ERROR             ENCODE_ERROR (27)
#define EFI_END_OF_MEDIA          ENCODE_ERROR (28)
#define EFI_END_OF_FILE           ENCODE_ERROR (31)

#define EFI_WARN_UNKNOWN_GLYPH    ENCODE_WARNING (1)
#define EFI_WARN_DELETE_FAILURE   ENCODE_WARNING (2)
#define EFI_WARN_WRITE_FAILURE    ENCODE_WARNING (3)
#define EFI_WARN_BUFFER_TOO_SMALL ENCODE_WARNING (4)

/*++

Routine Description:

  Efi compression routine.

--*/
EFI_STATUS
EfiCompress (
  IN      UINT8   *SrcBuffer,
  IN      UINT32  SrcSize,
  IN      UINT8   *DstBuffer,
  IN OUT  UINT32  *DstSize
  )
;
#ifdef __cplusplus
}  /* extern "C" */
#endif
#endif
