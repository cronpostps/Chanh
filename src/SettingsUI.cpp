#include "SettingsUI.h"
#include "MacroManager.h"
#include <windows.h>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

extern bool g_normalizeTone; // Biến cấu hình lấy từ main.cpp
extern bool g_shortcutW; // Thêm dòng này

LRESULT CALLBACK SettingsWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hList, hTrigger, hResult, hAdd, hDel, hChkTone;
    switch(msg) {
        case WM_CREATE: {
            // Khởi tạo thư viện Listview
            INITCOMMONCONTROLSEX icex;
            icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
            icex.dwICC  = ICC_LISTVIEW_CLASSES;
            InitCommonControlsEx(&icex);

            // Tạo bảng 2 cột
            hList = CreateWindowExW(0, WC_LISTVIEWW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_BORDER, 10, 10, 360, 150, hWnd, (HMENU)1, NULL, NULL);
            ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
            LVCOLUMNW lvc = {0};
            lvc.mask = LVCF_TEXT | LVCF_WIDTH;
            lvc.cx = 100; lvc.pszText = (LPWSTR)L"Phím gán"; ListView_InsertColumn(hList, 0, &lvc);
            lvc.cx = 240; lvc.pszText = (LPWSTR)L"Kết quả"; ListView_InsertColumn(hList, 1, &lvc);
            
            // Tạo ô nhập liệu và Nút bấm
            CreateWindowExW(0, L"STATIC", L"Phím gán:", WS_CHILD | WS_VISIBLE, 10, 170, 80, 20, hWnd, NULL, NULL, NULL);
            hTrigger = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 10, 190, 80, 25, hWnd, NULL, NULL, NULL);
            
            CreateWindowExW(0, L"STATIC", L"Kết quả:", WS_CHILD | WS_VISIBLE, 100, 170, 80, 20, hWnd, NULL, NULL, NULL);
            hResult = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 100, 190, 180, 25, hWnd, NULL, NULL, NULL);
            
            hAdd = CreateWindowExW(0, L"BUTTON", L"Thêm", WS_CHILD | WS_VISIBLE, 290, 190, 80, 25, hWnd, (HMENU)2, NULL, NULL);
            hDel = CreateWindowExW(0, L"BUTTON", L"Xóa chọn", WS_CHILD | WS_VISIBLE, 290, 220, 80, 25, hWnd, (HMENU)3, NULL, NULL);
            
            // Tạo Checkbox Chuẩn hóa dấu
            hChkTone = CreateWindowExW(0, L"BUTTON", L"Chuẩn hóa vị trí dấu (vd: òa -> oà)", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 10, 230, 250, 20, hWnd, (HMENU)4, NULL, NULL);
            SendMessage(hChkTone, BM_SETCHECK, g_normalizeTone ? BST_CHECKED : BST_UNCHECKED, 0);

            // TẠO THÊM CHECKBOX w = ư Ở ĐÂY
            HWND hChkW = CreateWindowExW(0, L"BUTTON", L"Gõ w để được ư (w = ư)", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 10, 255, 250, 20, hWnd, (HMENU)5, NULL, NULL);
            SendMessage(hChkW, BM_SETCHECK, g_shortcutW ? BST_CHECKED : BST_UNCHECKED, 0);

            // Nạp dữ liệu hiện tại vào bảng
            auto& macros = MacroManager::GetInstance().GetMacros();
            int i = 0;
            for (auto& pair : macros) {
                LVITEMW lvi = {0}; lvi.mask = LVIF_TEXT; lvi.iItem = i++;
                lvi.pszText = (LPWSTR)pair.first.c_str();
                ListView_InsertItem(hList, &lvi);
                ListView_SetItemText(hList, i-1, 1, (LPWSTR)pair.second.c_str());
            }
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 2) { // Bấm nút THÊM
                wchar_t trig[32], res[64];
                GetWindowTextW(hTrigger, trig, 32);
                GetWindowTextW(hResult, res, 64);
                if (lstrlenW(trig) > 0 && lstrlenW(res) > 0) {
                    // Cập nhật RAM và File ngay lập tức
                    MacroManager::GetInstance().GetMacros()[trig] = res;
                    MacroManager::GetInstance().SaveToFile();
                    
                    // Thêm vào bảng UI
                    int count = ListView_GetItemCount(hList);
                    LVITEMW lvi = {0}; lvi.mask = LVIF_TEXT; lvi.iItem = count;
                    lvi.pszText = trig;
                    ListView_InsertItem(hList, &lvi);
                    ListView_SetItemText(hList, count, 1, res);
                    
                    // Xóa trắng ô nhập
                    SetWindowTextW(hTrigger, L""); SetWindowTextW(hResult, L"");
                }
            } else if (LOWORD(wParam) == 3) { // Bấm nút XÓA
                int sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
                if (sel != -1) {
                    wchar_t trig[32];
                    ListView_GetItemText(hList, sel, 0, trig, 32);
                    MacroManager::GetInstance().GetMacros().erase(trig);
                    MacroManager::GetInstance().SaveToFile();
                    ListView_DeleteItem(hList, sel);
                }
            } else if (LOWORD(wParam) == 4) { // Tích Checkbox Chuẩn hóa dấu
                g_normalizeTone = (SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
            } else if (LOWORD(wParam) == 5) { // Tích Checkbox w = ư
                g_shortcutW = (SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
            }
            break;
        }
        case WM_CLOSE: DestroyWindow(hWnd); break;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

void ShowSettingsUI() {
    static bool registered = false;
    if (!registered) {
        WNDCLASSEXW wc = {sizeof(WNDCLASSEXW), 0, SettingsWndProc, 0, 0, GetModuleHandle(NULL), NULL, LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_WINDOW+1), NULL, L"CaySettingsUI", NULL};
        RegisterClassExW(&wc);
        registered = true;
    }
    // Mở cửa sổ cấu hình
    CreateWindowExW(0, L"CaySettingsUI", L"Cay - Gán phím & Cài đặt", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 200, 200, 400, 340, NULL, NULL, GetModuleHandle(NULL), NULL);
}