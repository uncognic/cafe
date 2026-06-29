#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

namespace
{
    constexpr UINT WM_TRAYICON = WM_USER + 1;

    constexpr UINT ID_NORMAL = 100;
    constexpr UINT ID_SYSTEM = 101;
    constexpr UINT ID_SYSTEM_LCD = 102;
    constexpr UINT ID_EXIT = 200;

    NOTIFYICONDATA nid{};
    HMENU menu{};
    EXECUTION_STATE current{};
}

void apply_state(EXECUTION_STATE state)
{
    current = state;
    SetThreadExecutionState(ES_CONTINUOUS | state);
}

void rebuild_menu()
{
    CheckMenuItem(menu, ID_NORMAL,
                  MF_BYCOMMAND | (current == 0 ? MF_CHECKED : MF_UNCHECKED));

    CheckMenuItem(menu, ID_SYSTEM,
                  MF_BYCOMMAND | (current == ES_SYSTEM_REQUIRED ? MF_CHECKED : MF_UNCHECKED));

    CheckMenuItem(menu, ID_SYSTEM_LCD,
                  MF_BYCOMMAND | ((current & (ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED)) == (ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED)
                                      ? MF_CHECKED
                                      : MF_UNCHECKED));
}

LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM w, LPARAM l)
{
    switch (msg)
    {
    case WM_TRAYICON:
        if (l == WM_RBUTTONUP)
        {
            POINT p;
            GetCursorPos(&p);
            SetForegroundWindow(hwnd);
            rebuild_menu();
            TrackPopupMenu(
                menu,
                TPM_RIGHTBUTTON,
                p.x, p.y,
                0, hwnd, nullptr);
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(w))
        {
        case ID_NORMAL:
            apply_state(0);
            break;
        case ID_SYSTEM:
            apply_state(ES_SYSTEM_REQUIRED);
            break;
        case ID_SYSTEM_LCD:
            apply_state(ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
            break;
        case ID_EXIT:
            PostQuitMessage(0);
            break;
        }
        break;

    case WM_DESTROY:
        apply_state(0);
        Shell_NotifyIcon(NIM_DELETE, &nid);
        break;
    }
    return DefWindowProc(hwnd, msg, w, l);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    WNDCLASS wc{};
    wc.lpfnWndProc = wndproc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"awake_tray";

    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        wc.lpszClassName,
        L"",
        0, 0, 0, 0, 0,
        HWND_MESSAGE,
        nullptr, hInstance, nullptr);

    menu = CreatePopupMenu();
    AppendMenu(menu, MF_STRING, ID_NORMAL, L"Normal");
    AppendMenu(menu, MF_STRING, ID_SYSTEM, L"Keep system awake");
    AppendMenu(menu, MF_STRING, ID_SYSTEM_LCD, L"Keep system + screen awake");
    AppendMenu(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(menu, MF_STRING, ID_EXIT, L"Exit");

    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcscpy_s(nid.szTip, L"Cafe");

    Shell_NotifyIcon(NIM_ADD, &nid);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
