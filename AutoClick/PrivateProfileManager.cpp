#include "PrivateProfileManager.h"

using namespace std;

PrivateProfile PrivateProfileManager::Load(const TCHAR* szFileName)
{
    TCHAR szSections[1024] = { _T('\0') };
    const int nSectionLength = GetPrivateProfileSectionNames(szSections, (sizeof(szSections) / sizeof(TCHAR)), szFileName);
    PrivateProfile result;

    if (nSectionLength < 1) {
        return result;
    }

    for (size_t i = 0; i < nSectionLength;) {
        _tstring strSectionName = &szSections[i];
        PrivateProfile::Section section = GetSection(szFileName, strSectionName.c_str());
        section.Name = strSectionName;
        result.Sections.push_back(section);
        i += (strSectionName.length() + 1);
    }

    return result;
}

void PrivateProfileManager::GetKeyAndValue(_tstring* strText, _tstring* strKey, _tstring* strValue)
{
    const TCHAR* szText = strText->c_str();
    const size_t nLength = strText->length();

    for (int i = 0; i < nLength; ++i) {
        if (szText[i] == _T('=')) {
            if (i > 0) {
                *strKey = strText->substr(0, i);
            }

            if (i < (nLength - 1)) {
                *strValue = strText->substr(i + 1);
            }

            return;
        }
    }
}

PrivateProfile::Section PrivateProfileManager::GetSection(const TCHAR* szFileName, const TCHAR* szSectionName)
{
    PrivateProfile::Section result;
    TCHAR szElements[256] = { _T('\0') };
    const int nElementLength = GetPrivateProfileSection(szSectionName, szElements, (sizeof(szElements) / sizeof(TCHAR)), szFileName);

    for (size_t i = 0; i < nElementLength;) {
        _tstring strElement = &szElements[i];
        _tstring strKey;
        _tstring strValue;
        GetKeyAndValue(&strElement, &strKey, &strValue);
        PrivateProfile::KeyAndValue keyAndValue;
        keyAndValue.Key = strKey;
        keyAndValue.Value = strValue;
        result.Values.push_back(keyAndValue);
        i += (strElement.length() + 1);
    }

    return result;
}
