#pragma once
#include <Windows.h>
#include <tchar.h>
#include "PrivateProfile.h"

class PrivateProfileManager
{
public:
    static PrivateProfile Load(const TCHAR* szFileName);
private:
    static void GetKeyAndValue(std::_tstring* strText, std::_tstring* strKey, std::_tstring* strValue);
    static PrivateProfile::Section GetSection(const TCHAR* szFileName, const TCHAR* szSectionName);
};
