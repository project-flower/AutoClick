#pragma once
#include <string>
namespace std
{
#if defined(_UNICODE)
    typedef wstring _tstring;
#else
    typedef string _tstring;
#endif
}
