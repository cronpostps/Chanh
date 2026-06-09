#include "ExcludeManager.h"
#include <fstream>
#include <windows.h>

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

ExcludeManager& ExcludeManager::GetInstance() {
    static ExcludeManager instance;
    return instance;
}

ExcludeManager::ExcludeManager() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    std::wstring exePath(path);
    size_t pos = exePath.find_last_of(L"\\/");
    m_filePath = exePath.substr(0, pos) + L"\\chanh_exclude.txt";
    LoadFromFile();

    if (m_excluded.empty()) {
        m_excluded.insert(L"csgo.exe");
        m_excluded.insert(L"league of legends.exe");
        m_excluded.insert(L"dota2.exe");
        m_excluded.insert(L"valorant-win64-shipping.exe");
        SaveToFile();
    }
}

void ExcludeManager::LoadFromFile() {
    m_excluded.clear();
    std::ifstream file(m_filePath, std::ios::binary);
    if (!file.is_open()) return;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        std::wstring wline = Utf8ToW(line);
        for (auto& c : wline) c = towlower(c); // Đưa về chữ thường
        m_excluded.insert(wline);
    }
}

void ExcludeManager::SaveToFile() {
    int retries = 3;
    while (retries > 0) {
        // Ép mở file để ghi lại từ đầu (trunc), tránh lỗi ghi đè rác
        std::ofstream file(m_filePath, std::ios::out | std::ios::trunc | std::ios::binary);
        if (file.is_open()) {
            for (const auto& app : m_excluded) {
                std::string line = WToUtf8(app) + "\r\n"; // Chuẩn hóa xuống dòng Windows
                file.write(line.c_str(), line.size());
            }
            file.flush();
            file.close();
            return; // Ghi thành công thì thoát
        }
        Sleep(100); // Nếu file bị khóa (bởi Antivirus/OneDrive), đợi 100ms rồi thử lại
        retries--;
    }
    // Báo lỗi nếu đã cố thử 3 lần mà vẫn bị chặn
    MessageBoxW(NULL, L"L\u1ed7i: Kh\u00f4ng th\u1ec3 ghi file chanh_exclude.txt! Vui l\u00f2ng \u0111\u00f3ng c\u00e1c ph\u1ea7n m\u1ec1m kh\u00e1c \u0111ang m\u1edf file n\u00e0y.", L"Chanh - C\u1ea3nh b\u00e1o", MB_OK | MB_ICONWARNING);
}

bool ExcludeManager::IsExcluded(const std::wstring& exeName) {
    std::wstring lowerName = exeName;
    for (auto& c : lowerName) c = towlower(c);
    return m_excluded.find(lowerName) != m_excluded.end();
}

void ExcludeManager::Add(const std::wstring& exeName) {
    std::wstring lowerName = exeName;
    for (auto& c : lowerName) c = towlower(c);
    m_excluded.insert(lowerName);
    SaveToFile();
}

void ExcludeManager::Remove(const std::wstring& exeName) {
    std::wstring lowerName = exeName;
    for (auto& c : lowerName) c = towlower(c);
    m_excluded.erase(lowerName);
    SaveToFile();
}