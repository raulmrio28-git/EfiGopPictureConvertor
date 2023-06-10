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

bool InitIniReader(char* pszFilename);
bool CheckIfSectionExists(char* szSection);
int ReadInteger(char* pszSection, char* pszKey, int nDefaultValue);
float ReadFloat(char* pszSection, char* pszKey, float fltDefaultValue);
bool ReadBoolean(char* pszSection, char* pszKey, bool bDefaultValue);
bool ReadBooleanYesNo(char* szSection, char* szKey, bool bolDefaultValue);
char* ReadString(char* pszSection, char* pszKey, const char* pszDefaultValue);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* _INI_READ_H_ */
