#include "MacroManager.h"
#include "InputInjector.h"
#include <fstream>
#include <windows.h>

// --- HÀM CÔNG CỤ XỬ LÝ UNICODE/UTF-8 ---
static std::wstring Utf8ToW(const std::string& str) {
    if (str.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstr(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], size);
    return wstr;
}

static std::string WToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int size = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string str(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size, NULL, NULL);
    return str;
}
// ----------------------------------------

MacroManager& MacroManager::GetInstance() {
    static MacroManager instance;
    return instance;
}

MacroManager::MacroManager() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    std::wstring exePath(path);
    size_t pos = exePath.find_last_of(L"\\/");
    m_filePath = exePath.substr(0, pos) + L"\\chanh_macros.txt";

    LoadFromFile();

    if (m_macros.empty()) {
        m_macros[L"11"] = L"2";
        m_macros[L"=))"] = L"\xD83D\xDE01";
        SaveToFile();
    }
}

void MacroManager::LoadFromFile() {
    m_macros.clear();
    std::ifstream file(m_filePath, std::ios::binary); // Đọc Binary chống lỗi
    if (!file.is_open()) return;
    
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back(); // Xóa ký tự xuống dòng rác
        std::wstring wline = Utf8ToW(line);
        size_t delim = wline.find(L"=");
        if (delim != std::wstring::npos) {
            std::wstring trigger = wline.substr(0, delim);
            std::wstring result = wline.substr(delim + 1);
            if (!trigger.empty() && !result.empty()) {
                m_macros[trigger] = result;
            }
        }
    }
}

void MacroManager::SaveToFile() {
    int retries = 3;
    while (retries > 0) {
        // Ép mở file để ghi lại từ đầu (trunc), tránh lỗi ghi đè rác
        std::ofstream file(m_filePath, std::ios::out | std::ios::trunc | std::ios::binary);
        if (file.is_open()) {
            for (const auto& pair : m_macros) {
                std::string line = WToUtf8(pair.first) + "=" + WToUtf8(pair.second) + "\r\n";
                file.write(line.c_str(), line.size());
            }
            file.flush();
            file.close();
            return;
        }
        Sleep(100); // Nếu file bị khóa (bởi Antivirus/OneDrive), đợi 100ms rồi thử lại
        retries--;
    }
    // Báo lỗi nếu đã cố thử 3 lần mà vẫn bị chặn
    MessageBoxW(NULL, L"L\u1ed7i: Kh\u00f4ng th\u1ec3 ghi file chanh_macros.txt! Vui l\u00f2ng \u0111\u00f3ng c\u00e1c ph\u1ea7n m\u1ec1m kh\u00e1c \u0111ang m\u1edf file n\u00e0y.", L"Chanh - C\u1ea3nh b\u00e1o", MB_OK | MB_ICONWARNING);
}

void MacroManager::ResetBuffer() {
    m_recentChars.clear();
}

bool MacroManager::ProcessChar(wchar_t ch) {
    if (ch == 0) return false;

    m_recentChars += ch;
    if (m_recentChars.length() > 3) m_recentChars.erase(0, 1); 

    for (size_t len = m_recentChars.length(); len > 0; --len) {
        std::wstring trigger = m_recentChars.substr(m_recentChars.length() - len);
        auto it = m_macros.find(trigger);
        
        if (it != m_macros.end()) {
            int bsCount = (int)len - 1;
            ChanhIME::InputInjector::ReplaceText(bsCount, it->second.c_str(), (int)it->second.length());
            ResetBuffer();
            return true;
        }
    }
    return false;
}