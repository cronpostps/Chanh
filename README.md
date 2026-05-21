# 🌶️ Cay — Zero-CRT Native C++ Vietnamese IME

[![Release](https://img.shields.io/github/v/release/tctvn/cay?style=flat-square&color=FF4500)](https://github.com/tctvn/cay/releases)
[![Size](https://img.shields.io/badge/size-21_KB-brightgreen?style=flat-square)](https://github.com/tctvn/cay/releases/download/cay/cay.exe)
[![Platform](https://img.shields.io/badge/platform-Windows-0078d7?style=flat-square)](https://github.com/tctvn/cay/releases)
[![License](https://img.shields.io/badge/license-GPL--3.0-blue?style=flat-square)](LICENSE)

Bộ gõ Telex siêu nhẹ dành cho developer hệ tối giản. Không giao diện màu mè, không cài đặt, không dependencies.

➡️ [**Download cay.exe (21 KB)**](https://github.com/tctvn/cay/releases/download/cay/cay.exe)

---

## ⚡ Technical Highlights

- **Zero-CRT & No STL:** Được build hoàn toàn không phụ thuộc C-Runtime. Dung lượng file thực thi bị ép xuống mức cực hạn (~21KB), footprint RAM gần như bằng 0.
- **No Heap Allocation:** Mọi buffer và mảng tra cứu (lookup tables) đều là `static const` và lưu tại read-only segment (`.rdata`). Không sử dụng cấp phát động (no `new`, no `malloc`).
- **O(n) Structural Validator:** Loại bỏ hoàn toàn Regex trong việc phân tích âm tiết. Sử dụng kỹ thuật Pointer Walk để xác thực cấu trúc tiếng Việt `(Initial)?(Nucleus)(Final)?(Tail)?` với độ trễ micro-giây.
- **Smart Diff Injection:** Giao tiếp với OS thông qua API Win32 `SendInput` kết hợp thuật toán tính toán độ lệch chuỗi (exact diffing), tránh xung đột với các IDE có tính năng auto-complete (VSCode, Chrome Omnibox).
- **Auto Bypass (Hard Filter):** Tự động nhận diện từ tiếng Anh hoặc các chuỗi sai quy tắc chính tả ngay từ ký tự đầu tiên để bypass, nhường lại quyền gõ phím nguyên thủy cho developer mà không làm hỏng code.

## 🛠️ Build từ Source

Yêu cầu hệ thống: `CMake` và `MSVC`.

```bash
git clone https://github.com/tctvn/cay.git
cd cay
cmake -B build
cmake --build build --config Release
```
*Output trả về nằm tại `build/Release/cay.exe`.*

## ⌨️ Sử dụng

- Chạy trực tiếp `cay.exe` (app chạy ngầm dưới System Tray).
- Chuẩn Telex: `aa`=â, `oo`=ô, `ee`=ê, `dd`=đ, `w`=ă/ư/ơ.
- Dấu: `s`=sắc, `f`=huyền, `r`=hỏi, `x`=ngã, `j`=nặng, `z`=xoá dấu.
- **Bật/Tắt nhanh:** `Ctrl + Shift`.

## 📜 License & Links
[GPL-3.0 License](LICENSE) © [tctvn](https://github.com/tctvn/cay).
Mã nguồn: [GitHub Repository](https://github.com/tctvn/cay).
