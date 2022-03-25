#include "TCharConvert.h"
#include <Windows.h>

std::string TCharConvert::ToMultiByte(const char* szValue, const int length)
{
    return std::string(szValue);
}

std::string TCharConvert::ToMultiByte(const wchar_t* szValue, const int length)
{
    const int length_ = (length + 1);
    char* szTemp = new char[length_];
    std::string strResult;

    if (WideCharToMultiByte(CP_ACP, 0, szValue, -1, szTemp, length_, nullptr, nullptr)) {
        strResult = szTemp;
    }

    delete[] szTemp;
    return strResult;
}

std::_tstring TCharConvert::ToTChar(const char* szValue, const int length)
{
#if defined(_UNICODE)
    return ToWideChar(szValue, length);
#else
    return std::string(szValue);
#endif
}

std::_tstring TCharConvert::ToTChar(const wchar_t* szValue, const int length)
{
#if defined(_UNICODE)
    return std::wstring(szValue);
#else
    return ToMultiByte(szValue, length);
#endif
}

std::wstring TCharConvert::ToWideChar(const char* szValue, const int length)
{
    const int length_ = (length + 1);
    wchar_t* szTemp = new wchar_t[length_];
    std::wstring strResult;

    if (MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szValue, -1, szTemp, length_)) {
        strResult = szTemp;
    }

    delete[] szTemp;
    return strResult;
}

std::wstring TCharConvert::ToWideChar(const wchar_t* szValue, const int length)
{
    return szValue;
}
