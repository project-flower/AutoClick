#pragma once
#include <tchar.h>

namespace Constants
{
    namespace Application
    {
        static const TCHAR* ICONTIP_DISABLED = _T(" (Disabled)");
    }

    namespace FileNames
    {
        static const TCHAR* EXTENSION_PRIVATEPROFILE = _T(".ini");
        static const TCHAR* EXTENSION_SETTINGS = _T(".config");
    }

    namespace Messages
    {
        static const TCHAR* CANNOT_LAUNCH_MULTIINSTANCES = _T("This application cannot launch multiple instances.");
        static const TCHAR* FAILED_LOAD_PRIVATEPROFILE = _T("AutoClick.ini could not be loaded.");
        static const TCHAR* FAILED_SET_TASKTRAYICON = _T("The icon could not be set to the task tray.");
    }

    namespace Settings
    {
        static const TCHAR* BOOL_TRUE = _T("1");
        static const TCHAR* CHECK_CLASS_NAME = _T("CheckClassName");
        static const TCHAR* SECTION_NAME = _T("Settings");
    }

    namespace Windows
    {
        static const TCHAR* SHELL_WM_TASKBARCREATED = _T("TaskbarCreated");
    }
}
