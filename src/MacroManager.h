#pragma once
#include <windows.h>
#include <string>
#include <unordered_map>

class MacroManager {
public:
    static MacroManager& GetInstance();

    bool ProcessChar(wchar_t ch);
    void ResetBuffer();
    
    // Nạp và lưu danh sách Macro từ file txt
    void LoadFromFile();
    void SaveToFile();

    // Dùng cho giao diện UI sau này
    std::unordered_map<std::wstring, std::wstring>& GetMacros() { return m_macros; }

private:
    MacroManager();
    std::unordered_map<std::wstring, std::wstring> m_macros;
    std::wstring m_recentChars;
    std::wstring m_filePath;
};