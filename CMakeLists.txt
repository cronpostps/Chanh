cmake_minimum_required(VERSION 3.20)
project(cay LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# Sources (Đã xóa no_crt.cpp)
# ---------------------------------------------------------------------------
set(SOURCES
    src/main.cpp
    src/CayData.cpp
    src/CayEngine.cpp
    src/InputInjector.cpp
    src/KeyboardHookManager.cpp
    src/MacroManager.cpp
    src/SettingsUI.cpp
)

add_executable(cay WIN32 ${SOURCES})

target_include_directories(cay PRIVATE src)

# ---------------------------------------------------------------------------
# Cấu hình MSVC Runtime Library
# Dùng Static CRT (/MT) để file .exe tự do chạy trên máy khác mà không cần cài VC Redist
# ---------------------------------------------------------------------------
set_property(TARGET cay PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# ---------------------------------------------------------------------------
# Compiler + linker flags (MSVC)
# Đã dọn dẹp các cờ cấm thư viện chuẩn, cho phép dùng C++ STL thoải mái
# ---------------------------------------------------------------------------
if(MSVC)
    target_compile_options(cay PRIVATE
        /W3          # Warning level 3
        /O2          # Tối ưu hóa tốc độ (thay vì /O1 ép dung lượng)
        /DUNICODE /D_UNICODE
        /DWIN32 /D_WINDOWS
        /utf-8       # <--- THÊM DÒNG NÀY: Ép MSVC dịch file theo chuẩn UTF-8
    )

    target_link_options(cay PRIVATE
        /SUBSYSTEM:WINDOWS
        /OPT:REF
        /OPT:ICF
        /machine:x64
    )

    # Win32 API Standard Libraries
    target_link_libraries(cay PRIVATE
        kernel32.lib
        user32.lib
        gdi32.lib
        shell32.lib
        advapi32.lib
        comctl32.lib
    )
endif()