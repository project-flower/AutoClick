// AutoClick.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "framework.h"
#include "AutoClick.h"
#include "Constants.h"
#include "PrivateProfileManager.h"
#include "Settings.h"
#include "TCharConvert.h"
#include "tstring.h"

#define MAX_LOADSTRING 100
#define WM_AUTOCLICK_NOTIFYICON (WM_APP+100)

using namespace std;

// グローバル変数:
bool bEnabled = true;
bool bOperated = false;
HINSTANCE hInst;                                // 現在のインターフェイス
HANDLE hMutex = nullptr;
HWINEVENTHOOK hWinEventHook = nullptr;
HWND hwndTarget = nullptr;
NOTIFYICONDATA notifyIconData;
const int nRetryLimit = 30;
Settings settings;
TCHAR szGuid[MAX_LOADSTRING];
TCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
TCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名
PrivateProfile targets;
UINT WM_TASKBARCREATED = 0;

// このコード モジュールに含まれる関数の宣言を転送します:
bool                CheckWindow(HWND hwnd);
void                ExitWithMessage(HWND hwnd, LPCTSTR szMessage);
bool                HandleMutex(HWND hWnd);
BOOL                InitInstance(HINSTANCE, int);
ATOM                MyRegisterClass(HINSTANCE hInstance);
_tstring            PathRenameExtension(const TCHAR* szFileName, const TCHAR* szExtension);
void                SetHook(bool bEnable);
void                SetIconTip(PNOTIFYICONDATA lpData);
bool                SetTaskTray(HWND hwnd);
int                 ShowErrorMessage(HWND hwnd, LPCTSTR szMessage);
void                UpdateIconTips(PNOTIFYICONDATA lpData);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

namespace Callbacks
{
    void OnCommand(HWND hwnd, WPARAM wParam, bool& bHandled);
    void OnCreate(HWND hwnd);
    void OnDestroy(HWND hwnd);
    BOOL CALLBACK OnEnumChild(HWND hwnd, LPARAM lParam);
    BOOL CALLBACK OnEnumWindows(HWND hwnd, LPARAM lParam);
    void OnNotifyIcon(HWND hwnd, LPARAM lParam, bool& bHandled);
    void OnPaint(HWND hwnd);
    void OnTaskbarCreated(HWND hwnd);
    VOID CALLBACK OnWinEvent(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD idEventThread, DWORD dwmsEventTime);
}

void ExitWithMessage(HWND hwnd, LPCTSTR szMessage)
{
    ShowErrorMessage(hwnd, szMessage);
    DestroyWindow(hwnd);
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPTSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ここにコードを挿入してください。

    // グローバル文字列を初期化する
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_AUTOCLICK, szWindowClass, MAX_LOADSTRING);
    LoadString(hInstance, IDS_GUID, szGuid, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // アプリケーション初期化の実行:
    if (!InitInstance(hInstance, nCmdShow)) {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_AUTOCLICK));

    MSG msg;

    // メイン メッセージ ループ:
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

bool CheckWindow(HWND hwnd)
{
    TCHAR szWindowText[MAX_LOADSTRING];

    if (GetWindowText(hwnd, szWindowText, MAX_LOADSTRING) != 0) {
        vector<PrivateProfile::Section>* pSections = &targets.Sections;

        for (int i = 0; i < pSections->size(); ++i) {
            PrivateProfile::Section* pSection = &pSections->at(i);

            if (_tcscmp(szWindowText, pSection->Name.c_str())) {
                continue;
            }

            hwndTarget = hwnd;
            EnumChildWindows(hwnd, Callbacks::OnEnumChild, i);
            hwndTarget = nullptr;

            if (bOperated) {
                return true;
            }
        }
    }

    return false;
}

bool HandleMutex(HWND hWnd)
{
    hMutex = CreateMutex(nullptr, TRUE, szGuid);
    DWORD dwError = GetLastError();

    if (dwError == ERROR_ALREADY_EXISTS) {
        return false;
    }

    return true;
}

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // グローバル変数にインスタンス ハンドルを格納する

    HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) {
        return FALSE;
    }

    return TRUE;
}

//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_AUTOCLICK));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCE(IDC_AUTOCLICK);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

_tstring PathRenameExtension(const TCHAR* szFileName, const TCHAR* szExtension)
{
    wstring strWideFileName = TCharConvert::ToWideChar(szFileName, _tcslen(szFileName));
    const size_t length = (strWideFileName.length() + 1);
    WCHAR szWideFileName[MAX_PATH] = { _T('\0') };
    wcscpy_s(szWideFileName, length, strWideFileName.c_str());
    wstring strWideExtension = TCharConvert::ToWideChar(szExtension ,_tcslen(szExtension));
    const HRESULT hResult = PathCchRenameExtension(szWideFileName, MAX_PATH, strWideExtension.c_str());
    _tstring result;

    switch (hResult) {
    case S_FALSE:
    case S_OK:
        result = TCharConvert::ToTChar(szWideFileName, MAX_PATH);
        break;

    default:
        result = _T("");
        break;
    }

    return result;
}

void SetHook(bool bEnable)
{
    if (bEnable) {
        if (hWinEventHook) {
            return;
        }

        hWinEventHook = SetWinEventHook(
            // EVENT_OBJECT_CREATE を指定するとウィンドウが生成される前に EnumChildWindows が呼び出され、
            // ハンドルが取得できない。
            EVENT_OBJECT_SHOW,
            EVENT_OBJECT_SHOW,
            nullptr,
            Callbacks::OnWinEvent,
            0,
            0,
            (WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS));
    }
    else {
        if (!hWinEventHook) {
            return;
        }

        UnhookWinEvent(hWinEventHook);
        hWinEventHook = nullptr;
    }

    bEnabled = bEnable;
    UpdateIconTips(&notifyIconData);
}

void SetIconTip(PNOTIFYICONDATA lpData)
{
    _tcscpy_s(lpData->szTip, szTitle);

    if (!bEnabled) {
        _tcscat_s(lpData->szTip, Constants::Application::ICONTIP_DISABLED);
    }
}

bool SetTaskTray(HWND hwnd)
{
    HICON hIcon;
    hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_AUTOCLICK));
    notifyIconData.cbSize = sizeof(notifyIconData);
    notifyIconData.hWnd = hwnd;
    notifyIconData.uID = 0;
    notifyIconData.uFlags = (NIF_MESSAGE | NIF_ICON | NIF_TIP);
    notifyIconData.uCallbackMessage = WM_AUTOCLICK_NOTIFYICON;
    notifyIconData.hIcon = hIcon;
    SetIconTip(&notifyIconData);

    for (int i = 0; i < nRetryLimit; ++i) {
        if (Shell_NotifyIcon(NIM_ADD, &notifyIconData)) {
            break;
        }

        if (GetLastError() != ERROR_TIMEOUT) {
            if (i > nRetryLimit) {
                if (hIcon) DestroyIcon(hIcon);

                ExitWithMessage(hwnd, Constants::Messages::FAILED_SET_TASKTRAYICON);
                return false;
            }
        }
        else if (Shell_NotifyIcon(NIM_MODIFY, &notifyIconData)) {
            break;
        }

        Sleep(1000);
    }

    if (hIcon) DestroyIcon(hIcon);

    return true;
}

int ShowErrorMessage(HWND hwnd, LPCTSTR szMessage)
{
    return MessageBox(hwnd, szMessage, szTitle, (MB_ICONERROR | MB_OK));
}

void UpdateIconTips(PNOTIFYICONDATA lpData)
{
    SetIconTip(lpData);
    Shell_NotifyIcon(NIM_MODIFY, lpData);
}

//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND  - アプリケーション メニューの処理
//  WM_PAINT    - メイン ウィンドウを描画する
//  WM_DESTROY  - 中止メッセージを表示して戻る
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_AUTOCLICK_NOTIFYICON:
    {
        bool bHandled = false;
        Callbacks::OnNotifyIcon(hWnd, lParam, bHandled);

        if (bHandled) {
            return TRUE;
        }
    }

    break;

    case WM_COMMAND:
    {
        bool bHandled = false;
        Callbacks::OnCommand(hWnd, wParam, bHandled);

        if (!bHandled) {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }

    break;

    case WM_CREATE:
        Callbacks::OnCreate(hWnd);
        break;

    case WM_DESTROY:
        Callbacks::OnDestroy(hWnd);
        break;

    case WM_PAINT:
        Callbacks::OnPaint(hWnd);
        break;

    default:
        if ((message == WM_TASKBARCREATED) && (WM_TASKBARCREATED > 0)) {
            Callbacks::OnTaskbarCreated(hWnd);
        }

        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

void Callbacks::OnCommand(HWND hwnd, WPARAM wParam, bool& bHandled)
{
    int wmId = LOWORD(wParam);

    // 選択されたメニューの解析:
    switch (wmId) {
    case IDM_ENABLE:
        SetHook(!bEnabled);
        break;

    case IDM_EXIT:
        DestroyWindow(hwnd);
        break;

    default:
        bHandled = false;
        return;
    }

    bHandled = true;
}

void Callbacks::OnCreate(HWND hwnd)
{
    if (!HandleMutex(hwnd)) {
        ExitWithMessage(hwnd, Constants::Messages::CANNOT_LAUNCH_MULTIINSTANCES);
        return;
    }

    const UINT uiMessage = RegisterWindowMessage(Constants::Windows::SHELL_WM_TASKBARCREATED);

    if (uiMessage != 0) {
        WM_TASKBARCREATED = uiMessage;
    }

    TCHAR szModuleFileName[MAX_PATH] = { _T('\0') };
    _tstring result;
    
    if (GetModuleFileName(nullptr, szModuleFileName, MAX_PATH) > 0) {
        _tstring iniFilePath = PathRenameExtension(szModuleFileName, Constants::FileNames::EXTENSION_SETTINGS);
        iniFilePath += Constants::FileNames::EXTENSION_PRIVATEPROFILE;
        settings = Settings::Load(iniFilePath.c_str());
        iniFilePath = PathRenameExtension(szModuleFileName, Constants::FileNames::EXTENSION_PRIVATEPROFILE);
        targets = PrivateProfileManager::Load(iniFilePath.c_str());

        if (targets.Sections.size() < 1) {
            ExitWithMessage(hwnd, Constants::Messages::FAILED_LOAD_PRIVATEPROFILE);
            return;
        }
    }

    if (!SetTaskTray(hwnd)) {
        return;
    }

    SetHook(bEnabled);
}

void Callbacks::OnDestroy(HWND hwnd)
{
    Shell_NotifyIcon(NIM_DELETE, &notifyIconData);
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    PostQuitMessage(0);
}

BOOL CALLBACK Callbacks::OnEnumChild(HWND hwnd, LPARAM lParam)
{
    size_t nSectionIndex = (size_t)lParam;
    vector<PrivateProfile::Section>* pSections = &targets.Sections;

    if ((nSectionIndex < 0) || (nSectionIndex > pSections->size())) {
        return FALSE;
    }

    PrivateProfile::Section* pSection = &pSections->at(nSectionIndex);
    vector<PrivateProfile::KeyAndValue>* pKeyAndValues = &pSection->Values;

    for (vector<PrivateProfile::KeyAndValue>::iterator iterator = pKeyAndValues->begin(); iterator != pKeyAndValues->end(); ++iterator) {
        const TCHAR* szValue = iterator->Value.c_str();
        TCHAR szText[MAX_LOADSTRING];

        if (!GetWindowText(hwnd, szText, MAX_LOADSTRING)) {
            return TRUE;
        }

        if (!_tcscmp(szText, szValue)) {
            // ウィンドウ タイトルが一致
            //SetActiveWindow(hwndTarget);
            //SendMessage(hwndTarget, WM_ACTIVATE, 0, 0);

            if (settings.bCheckClassName) {
                if (!GetClassName(hwnd, szText, MAX_LOADSTRING)) {
                    return TRUE;
                }

                if (_tcscmp(szText, iterator->Key.c_str())) {
                    return TRUE;
                }
            }

            if (!SetActiveWindow(hwndTarget)) {
                const DWORD dwForegroundProcessId = GetWindowThreadProcessId(hwnd, nullptr);
                const DWORD dwTargetWindowProcessId = GetWindowThreadProcessId(hwndTarget, nullptr);

                if (dwForegroundProcessId && dwTargetWindowProcessId) {
                    AttachThreadInput(dwTargetWindowProcessId, dwForegroundProcessId, TRUE);
                    SetForegroundWindow(hwndTarget);
                    AttachThreadInput(dwTargetWindowProcessId, dwForegroundProcessId, FALSE);
                }
            }

            SendMessage(hwnd, BM_CLICK, 0, 0);
            bOperated = true;
            return FALSE;
        }
    }

    return TRUE;
}

BOOL CALLBACK Callbacks::OnEnumWindows(HWND hwnd, LPARAM lParam)
{
    return !CheckWindow(hwnd);
}

void Callbacks::OnNotifyIcon(HWND hwnd, LPARAM lParam, bool& bHandled)
{
    switch (LOWORD(lParam)) {
    case WM_LBUTTONUP:
        if (bEnabled) {
            bOperated = false;
            EnumWindows(Callbacks::OnEnumWindows, 0);
        }

        break;

    case WM_RBUTTONUP:
        HMENU hMenu, hSubMenu;
        hMenu = LoadMenu(reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hwnd, GWLP_HINSTANCE)), MAKEINTRESOURCE(IDM_POPUPMENU));
        hSubMenu = GetSubMenu(hMenu, 0);

        {
            const UINT uChecked = (bEnabled ? MF_CHECKED : MF_UNCHECKED);
            CheckMenuItem(hSubMenu, IDM_ENABLE, uChecked);
        }

        POINT point;
        GetCursorPos(&point);
        SetForegroundWindow(hwnd);

        if (TrackPopupMenu(hSubMenu, TPM_LEFTALIGN, point.x, point.y, 0, hwnd, nullptr)) {
            PostMessage(hwnd, WM_NULL, 0, 0);
        }

        bHandled = DestroyMenu(hMenu);
        break;

    default:
        bHandled = false;
        break;
    }
}

void Callbacks::OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    EndPaint(hwnd, &ps);
}

void Callbacks::OnTaskbarCreated(HWND hwnd)
{
    SetTaskTray(hwnd);
}

VOID CALLBACK Callbacks::OnWinEvent(
    HWINEVENTHOOK hWinEventHook,
    DWORD event,
    HWND hwnd,
    LONG idObject,
    LONG idChild,
    DWORD idEventThread,
    DWORD dwmsEventTime)
{
    CheckWindow(hwnd);
}
