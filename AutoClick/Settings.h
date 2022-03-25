#pragma once
#include <tchar.h>
class Settings
{
public:
    bool bCheckClassName;

    Settings();
    static Settings Load(const TCHAR* szFileName);
};
