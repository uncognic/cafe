#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#define WM_TRAYICON (WM_USER + 1)

#define ID_NORMAL 100
#define ID_SYSTEM 101
#define ID_SYSTEM_LCD 102
#define ID_EXIT 200

static NOTIFYICONDATA nid;
static HMENU menu;
static EXECUTION_STATE current = 0;

void apply_state(EXECUTION_STATE state)
{
    current = state;
    SetThreadExecutionState(ES_CONTINUOUS | state);
}

void rebuild_menu(void)
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
                0, hwnd, NULL);
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

int WINAPI WinMain(HINSTANCE h, HINSTANCE _, LPSTR __, int ___)
{
    WNDCLASS wc = {0};
    wc.lpfnWndProc = wndproc;
    wc.hInstance = h;
    wc.lpszClassName = "awake_tray";

    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        wc.lpszClassName,
        "",
        0, 0, 0, 0, 0,
        HWND_MESSAGE,
        NULL, h, NULL);

    menu = CreatePopupMenu();
    AppendMenu(menu, MF_STRING, ID_NORMAL, "Normal");
    AppendMenu(menu, MF_STRING, ID_SYSTEM, "Keep system awake");
    AppendMenu(menu, MF_STRING, ID_SYSTEM_LCD, "Keep system + screen awake");
    AppendMenu(menu, MF_SEPARATOR, 0, NULL);
    AppendMenu(menu, MF_STRING, ID_EXIT, "Exit");

    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    lstrcpy(nid.szTip, "Cafe");

    Shell_NotifyIcon(NIM_ADD, &nid);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}