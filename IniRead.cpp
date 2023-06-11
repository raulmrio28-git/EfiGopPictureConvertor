/*
** ===========================================================================
** IniRead.cpp
** Read INI file to a value
** ---------------------------------------------------------------------------
** Date			User			Changes
** 06/09/2023	raulmrio28-git	Initial version
** ===========================================================================
*/

/*
**----------------------------------------------------------------------------
**  Includes
**----------------------------------------------------------------------------
*/

#include "IniRead.h"
#include <iostream>

/*
**----------------------------------------------------------------------------
**  Definitions
**----------------------------------------------------------------------------
*/

/*
**----------------------------------------------------------------------------
**  Type Definitions
**----------------------------------------------------------------------------
*/

/*
**---------------------------------------------------------------------------
**  Global variables
**---------------------------------------------------------------------------
*/

/*
**---------------------------------------------------------------------------
**  UINT32ernal variables
**---------------------------------------------------------------------------
*/

std::string m_szFileName;

/*
**---------------------------------------------------------------------------
**  Function(UINT32ernal use only) Declarations
**---------------------------------------------------------------------------
*/

bool InitIniReader(const char* szFileName)
{
	if (!szFileName) return false;
	m_szFileName = std::string(szFileName);
	return true;
}
bool CheckIfSectionExists(const char* szSection)
{
	if (GetPrivateProfileSection(szSection, nullptr, 0, m_szFileName.c_str()) == 0)
		return true;
	else
		return false;
}
UINT32 ReadInteger(const char* szSection, const char* szKey, UINT32 iDefaultValue)
{
	UINT32 iResult = GetPrivateProfileInt(szSection, szKey, iDefaultValue, m_szFileName.c_str());
	return iResult;
}
float ReadFloat(const char* szSection, const char* szKey, float fltDefaultValue)
{
	char szResult[255];
	char szDefault[255];
	float fltResult;
	sprintf_s(szDefault, "%f", fltDefaultValue);
	GetPrivateProfileString(szSection, szKey, szDefault, szResult, 255, m_szFileName.c_str());
	fltResult = strtof(szResult, nullptr);
	return fltResult;
}
bool ReadBoolean(const char* szSection, const char* szKey, bool bolDefaultValue)
{
	char szResult[255];
	char szDefault[255];
	bool bolResult;
	sprintf_s(szDefault, "%s", bolDefaultValue ? "True" : "False");
	GetPrivateProfileString(szSection, szKey, szDefault, szResult, 255, m_szFileName.c_str());
	bolResult = (strcmp(szResult, "True") == 0 ||
		strcmp(szResult, "true") == 0) ? true : false;
	return bolResult;
}
bool ReadBooleanYesNo(const char* szSection, const char* szKey, bool bolDefaultValue)
{
	char szResult[255];
	char szDefault[255];
	bool bolResult;
	sprintf_s(szDefault, "%s", bolDefaultValue ? "Yes" : "No");
	GetPrivateProfileString(szSection, szKey, szDefault, szResult, 255, m_szFileName.c_str());
	bolResult = (strcmp(szResult, "Yes") == 0 ||
		strcmp(szResult, "yes") == 0) ? true : false;
	return bolResult;
}
char* ReadString(const char* szSection, const char* szKey, const char* szDefaultValue)
{
	auto* szResult = new char[INI_READ_MAX_STR_LENGTH+1];
	memset(szResult, 0x00, INI_READ_MAX_STR_LENGTH);
	GetPrivateProfileString(szSection, szKey,
		szDefaultValue, szResult, INI_READ_MAX_STR_LENGTH, m_szFileName.c_str());
	return szResult;
}