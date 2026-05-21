#ifndef UNICODE
#define UNICODE
#endif
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <thread>
#include <vector>
#include <string>

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001

#define NIM_ADD        0
#define NIM_DELETE     2
#define NIF_MESSAGE    1
#define NIF_ICON       2
#define NIF_TIP        4

typedef struct {
    DWORD cbSize; HWND hWnd; UINT uID; UINT uFlags; UINT uCallbackMessage;
    HICON hIcon; WCHAR szTip[128]; DWORD dwState; DWORD dwStateMask;
    WCHAR szInfo[256]; union { UINT uTimeout; UINT uVersion; } u;
    WCHAR szInfoTitle[64]; DWORD dwInfoFlags;
} MY_NID;

typedef BOOL(WINAPI* PFN_ShellIcon)(DWORD, MY_NID*);

MY_NID nid = { 0 }; 
std::vector<std::thread> threads;
bool keep_running = true;
HWND hwndMain = NULL;
const std::wstring PASSWORD = L"123456"; 

BOOL CallTray(DWORD msg, MY_NID* data) {
    HMODULE hMod = LoadLibraryW(L"shell32.dll");
    if (!hMod) return FALSE;
    PFN_ShellIcon pfn = (PFN_ShellIcon)GetProcAddress(hMod, "Shell_NotifyIconW");
    return pfn ? pfn(msg, data) : FALSE;
}

LRESULT CALLBACK PasswordProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    static HWND hEdit;
    if (msg == WM_CREATE) {
        hEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_PASSWORD | ES_AUTOHSCROLL, 20, 20, 200, 25, hwnd, NULL, NULL, NULL);
        CreateWindowExW(0, L"BUTTON", L"OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 80, 60, 80, 30, hwnd, (HMENU)IDOK, NULL, NULL);
    } else if (msg == WM_COMMAND && LOWORD(wp) == IDOK) {
        wchar_t buf[128] = {0};
        GetWindowTextW(hEdit, buf, 128);
        if (std::wstring(buf) == PASSWORD) {
            // 密碼正確：先銷毀自己
            DestroyWindow(hwnd);
            // 核心修復：直接向主窗口發送自定義退出指令，不再亂發 WM_CLOSE
            if (hwndMain) PostMessageW(hwndMain, WM_COMMAND, ID_TRAY_EXIT + 100, 0);
        } else {
            MessageBoxW(hwnd, L"Incorrect Password!", L"Error", MB_ICONERROR);
            DestroyWindow(hwnd);
        }
    } else if (msg == WM_CLOSE) {
        DestroyWindow(hwnd);
    } else {
        return DefWindowProcW(hwnd, msg, wp, lp);
    }
    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_TRAYICON && (lp == WM_RBUTTONUP || lp == WM_LBUTTONUP)) {
        HMENU hMenu = CreatePopupMenu();
        AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Exit Program");
        POINT pt; GetCursorPos(&pt);
        SetForegroundWindow(hwnd);
        TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);
        DestroyMenu(hMenu);
    } else if (msg == WM_COMMAND && LOWORD(wp) == ID_TRAY_EXIT) {
        WNDCLASSW wc = { 0 }; wc.lpfnWndProc = PasswordProc; wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = L"PassClass"; wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        RegisterClassW(&wc);
        HWND dlg = CreateWindowExW(WS_EX_TOPMOST | WS_EX_DLGMODALFRAME, L"PassClass", L"Enter Password", WS_POPUPWINDOW | WS_CAPTION, 
            (GetSystemMetrics(SM_CXSCREEN)-260)/2, (GetSystemMetrics(SM_CYSCREEN)-140)/2, 260, 140, hwnd, NULL, GetModuleHandle(NULL), NULL);
        ShowWindow(dlg, SW_SHOW); UpdateWindow(dlg);
        
        // 密碼框的子消息循環
        MSG dMsg; 
        while (GetMessageW(&dMsg, NULL, 0, 0)) { 
            TranslateMessage(&dMsg); 
            DispatchMessageW(&dMsg); 
            if (!IsWindow(dlg)) break; 
        }
    } else if (msg == WM_COMMAND && LOWORD(wp) == (ID_TRAY_EXIT + 100)) {
        // 密碼驗證成功後由主窗口安全觸發銷毀
        DestroyWindow(hwnd);
    } else if (msg == WM_CLOSE) {
        DestroyWindow(hwnd);
    } else if (msg == WM_DESTROY) {
        CallTray(NIM_DELETE, &nid); // 移除託盤
        keep_running = false;       // 終止 CPU 線程
        PostQuitMessage(0);         // 真正退出主循環
    } else {
        return DefWindowProcW(hwnd, msg, wp, lp);
    }
    return 0;
}

void cpu_burner() {
    while (keep_running) { double x = 1.2345; x = x * x; }
}

int main() {
    HWND console = GetConsoleWindow();
    if (console) ShowWindow(console, SW_HIDE);

    unsigned int cores = std::thread::hardware_concurrency();
    HINSTANCE hInst = GetModuleHandle(NULL);
    WNDCLASSW wc = { 0 }; wc.lpfnWndProc = WndProc; wc.hInstance = hInst; wc.lpszClassName = L"TrayClass";
    RegisterClassW(&wc);
    hwndMain = CreateWindowExW(0, L"TrayClass", L"", 0, 0, 0, 0, 0, NULL, NULL, hInst, NULL);

    nid.cbSize = sizeof(MY_NID); nid.hWnd = hwndMain; nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP; nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    lstrcpyW(nid.szTip, L"CPU Burner Active");
    CallTray(NIM_ADD, &nid);

    for (size_t i = 0; i < cores; ++i) threads.push_back(std::thread(cpu_burner));

    // 主消息循環：現在能 100% 接收到退出訊號
    MSG msg; while (GetMessageW(&msg, NULL, 0, 0)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
    
    // 安全回收所有 CPU 線程
    for (auto &t : threads) { if (t.joinable()) t.join(); }
    return 0;
}
