# 🍋‍🟩 Chanh — Bộ gõ Telex Tiếng Việt Tối Giản & Mạnh Mẽ

[![Platform](https://img.shields.io/badge/Platform-Windows-F38020?style=for-the-badge&logo=cloudflare&logoColor=white)](#)
[![Language](https://img.shields.io/badge/Language-C++-3776AB?style=for-the-badge&logo=python&logoColor=white)](#)

[![GitHub](https://img.shields.io/badge/GitHub-181717?style=for-the-badge&logo=github&logoColor=white)](https://github.com/cronpostps/chanh)
[![Latest Version](https://img.shields.io/badge/-Latest%20Version-blue?style=for-the-badge)](https://github.com/cronpostps/chanh/releases/download/latest/ChanhEngine_latest.zip)


> **Chanh Telex** là một bộ gõ Telex siêu nhẹ, "plug-and-play" dành cho anh em developer và game thủ. Không bloatware, không quảng cáo, bật lên là gõ với trải nghiệm mượt mà, được thiết kế để hạn chế tối đa việc phá hỏng code, gõ nhầm lệnh hay giật lag khi chơi game.

---

## 🌱 1. Nguồn gốc dự án (Credits)

Dự án **Chanh** được xây dựng và phát triển dựa trên nền tảng mã nguồn mở tuyệt vời của dự án **[Cay](https://github.com/tctvn/cay)** (bởi tác giả [tctvn](https://github.com/tctvn)). 

Chúng tôi xin gửi lời cảm ơn chân thành đến tác giả của **Cay** vì một kiến trúc lõi C++ cực kỳ thông minh và tối ưu. **Chanh** kế thừa trọn vẹn triết lý "siêu nhẹ, zero-config" của người tiền nhiệm, đồng thời được "độ" thêm một loạt tính năng cao cấp để trở thành một bộ gõ toàn diện hơn cho nhu cầu sử dụng hàng ngày.

---

## ✨ 2. Tính năng "ăn tiền" của Chanh

- **Siêu nhẹ & Tối ưu hóa (Zero-Bloat):** File thực thi chỉ khoảng ~561KB. Được build với cờ tối ưu hóa cấp cao nhất của MSVC (`/O2`, `/OPT:ICF`), không phụ thuộc thư viện bên thứ ba, ngốn RAM gần như bằng 0.
- **Ngủ đông thông minh (App Bypass):** Tự động phát hiện và nhường lại quyền điều khiển bàn phím khi bạn mở các ứng dụng/game trong danh sách Bypass (CS:GO, LoL, Dota 2, Valorant...Lưu ý nhớ nhập đúng file thực thi tiến trình (.exe) khi thêm vào danh sách). Tha hồ vừa chat vừa chơi game!
- **Gõ tắt (Macro):** Hỗ trợ thiết lập các từ gõ tắt cá nhân hóa, giúp tăng tốc độ gõ văn bản và code.
- **Giao diện "Pro":** Bổ sung cửa sổ thiết lập UI hiện đại, quản lý danh sách Gõ tắt và Ngủ đông trực quan, lưu trữ dữ liệu tự động.
- **Khắc phục triệt để lỗi trình duyệt:** Sử dụng kỹ thuật tiêm phím ảo đột phá, giải quyết dứt điểm 100% lỗi nuốt chữ, nhảy nháy khi gõ trên thanh địa chỉ Chrome, Edge hay VSCode...
- **Tùy biến linh hoạt:** Hỗ trợ chuẩn hóa dấu tiếng Việt (òa -> oà), tùy chọn gõ `w` thành `ư`.
- **Auto-Start an toàn:** Tự khởi động nhanh chóng cùng Windows.

## 🛠️ 3. Build từ Source

Yêu cầu: Có sẵn `CMake` và `MSVC` (Visual Studio C++ Build Tools).

```bash
git clone https://github.com/tctvn/cay.git
cd cay
cmake -B build
cmake --build build --config Release
```
*File build xong sẽ nằm tại build/Release/chanh.exe.*

## ⌨️ 4. Cách dùng

1. Chạy file chanh.exe -> app sẽ chạy ngầm dưới khay hệ thống. Để Chanh hoạt động với các ứng dụng lõi của Windows (eg. MS Edge...), cần chạy nó dưới quyền quản trị (Run as Administrator).

2. Click chuột phải vào biểu tượng dưới khay hệ thống để mở bảng Thiết lập (Thêm Gõ tắt, Thêm App Game cần Bypass, Tự khởi động...).

3. Cú pháp chuẩn Telex: aa=â, oo=ô, ee=ê, dd=đ, w=ă/ư/ơ.

4. Dấu: s=sắc, f=huyền, r=hỏi, x=ngã, j=nặng, z=xoá dấu.

5. Phím tắt Bật/Tắt nhanh: Ctrl + Shift.

## ☎️ 5. Đóng góp & Mã nguồn mở

**Mọi đóng góp, tối ưu code & báo lỗi đều được hoan nghênh tại kho lưu trữ chính thức!**

**👨‍💻 Dev by ANHNN**

[![Telegram](https://img.shields.io/badge/Telegram-2CA5E0?style=for-the-badge&logo=telegram&logoColor=white)](https://t.me/anhnn83)
[![Email](https://img.shields.io/badge/Email-D14836?style=for-the-badge&logo=gmail&logoColor=white)](mailto:anhnn@dgd.vn)
[![website](https://img.shields.io/badge/Website-anhnn.cronpost.com-181717?style=for-the-badge&logo=google-chrome&logoColor=white)](https://anhnn.cronpost.com)

<hr>
<div align="center">
  &copy; 2026 <a href="https://github.com/cronpostps">anhnn</a>. Mọi quyền được bảo lưu.<br>
  <b>Chanh</b> được phát hành dưới giấy phép <a href="LICENSE">GNU GPLv3</a>.
</div>