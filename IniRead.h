/*
** ===========================================================================
** IniRead.h
** Read INI file to a value
** ---------------------------------------------------------------------------
** Date			User			Changes
** 06/09/2023	raulmrio28-git	Initial version
** ===========================================================================
*/

#ifndef _INI_READ_H_
#define _INI_READ_H_

/*
**----------------------------------------------------------------------------
**  Includes
**----------------------------------------------------------------------------
*/

#ifdef __cplusplus
extern "C" {
#endif
/* fix for no_init_all */
#if (_MSC_VER >= 1915)
#define no_init_all deprecated
#endif
#include <Windows.h>

/*
**----------------------------------------------------------------------------
**  Definitions
**----------------------------------------------------------------------------
*/
#define INI_READ_MAX_STR_LENGTH 512

/*
**----------------------------------------------------------------------------
**  Type Definitions
**----------------------------------------------------------------------------
*/

/*
**---------------------------------------------------------------------------
**  Variable Declarations
**---------------------------------------------------------------------------
*/

/*
**---------------------------------------------------------------------------
**  Function(external use only) Declarations
**---------------------------------------------------------------------------
*/

bool InitIniReader(const char* szFileName);
bool CheckIfSectionExists(const char* szSection);
UINT32 ReadInteger(const char* szSection, const char* szKey, UINT32 iDefaultValue);
float ReadFloat(const char* szSection, const char* szKey, float fltDefaultValue);
bool ReadBoolean(const char* szSection, const char* szKey, bool bolDefaultValue);
bool ReadBooleanYesNo(const char* szSection, const char* szKey, bool bolDefaultValue);
char* ReadString(const char* szSection, const char* szKey, const char* szDefaultValue);
#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* _INI_READ_H_ */
