#pragma once
#include "tstring.h"
class TCharConvert
{
public:
    static std::string ToMultiByte(const char* szValue, const int length);
    static std::string ToMultiByte(const wchar_t* szValue, const int length);
    static std::_tstring ToTChar(const char* szValue, const int length);
    static std::_tstring ToTChar(const wchar_t* szValue, const int length);
    static std::wstring ToWideChar(const char* szValue, const int length);
    static std::wstring ToWideChar(const wchar_t* szValue, const int length);
};
