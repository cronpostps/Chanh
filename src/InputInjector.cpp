#include "InputInjector.h"

// Maximum characters in a single word + ZWJ + backspaces headroom.
// 1 ZWJ + 64 backspaces + 256 unicode chars = 321 INPUT structs worst case.
#define MAX_INPUTS 321

namespace CayIME {

// ---------------------------------------------------------------------------
// Internal helper – fill an INPUT for a Unicode keydown/keyup pair.
// ---------------------------------------------------------------------------
static void FillUnicodeInput(INPUT* inp, wchar_t ch, DWORD flags) {
    inp->type           = INPUT_KEYBOARD;
    inp->ki.wVk         = 0;
    inp->ki.wScan       = (WORD)ch;
    inp->ki.dwFlags     = flags | KEYEVENTF_UNICODE;
    inp->ki.time        = 0;
    inp->ki.dwExtraInfo = InputInjector::MAGIC_EXTRA_INFO;
}

// ---------------------------------------------------------------------------
// Internal helper – fill an INPUT for a virtual-key keydown/keyup pair.
// ---------------------------------------------------------------------------
static void FillVkInput(INPUT* inp, WORD vk, DWORD flags) {
    inp->type           = INPUT_KEYBOARD;
    inp->ki.wVk         = vk;
    inp->ki.wScan       = (WORD)MapVirtualKeyW(vk, MAPVK_VK_TO_VSC);
    inp->ki.dwFlags     = flags;
    inp->ki.time        = 0;
    inp->ki.dwExtraInfo = InputInjector::MAGIC_EXTRA_INFO;
}

// ---------------------------------------------------------------------------
// ReplaceText – the core batched injection.
//
// Layout of the single INPUT array:
//   [0]      ZWJ down
//   [1]      ZWJ up
//   [2..2+2*bs-1]   bs*(VK_BACK down + VK_BACK up)
//   remaining       newText unicode pairs
// ---------------------------------------------------------------------------
void InputInjector::ReplaceText(int backspaceCount, const wchar_t* newText, int newTextLen) {
    // 1. Calculate buffer size: (backspaces * 2) + (newText * 2)
    int totalEvents = (backspaceCount * 2) + (newTextLen * 2);
    if (totalEvents > MAX_INPUTS) return; // Safety check

    INPUT inputs[MAX_INPUTS];
    int idx = 0;

    // 2. Batch: Send Backspaces
    for (int i = 0; i < backspaceCount; i++) {
        inputs[idx].type = INPUT_KEYBOARD;
        inputs[idx].ki.wVk = VK_BACK;
        inputs[idx].ki.wScan = 0;
        inputs[idx].ki.dwFlags = 0; // Down
        inputs[idx].ki.time = 0;
        inputs[idx].ki.dwExtraInfo = MAGIC_EXTRA_INFO;
        idx++;

        inputs[idx].type = INPUT_KEYBOARD;
        inputs[idx].ki.wVk = VK_BACK;
        inputs[idx].ki.wScan = 0;
        inputs[idx].ki.dwFlags = KEYEVENTF_KEYUP;
        inputs[idx].ki.time = 0;
        inputs[idx].ki.dwExtraInfo = MAGIC_EXTRA_INFO;
        idx++;
    }

    // 3. Batch: Send New Text (Unicode)
    for (int i = 0; i < newTextLen; i++) {
        inputs[idx].type = INPUT_KEYBOARD;
        inputs[idx].ki.wVk = 0;
        inputs[idx].ki.wScan = newText[i];
        inputs[idx].ki.dwFlags = KEYEVENTF_UNICODE;
        inputs[idx].ki.time = 0;
        inputs[idx].ki.dwExtraInfo = MAGIC_EXTRA_INFO;
        idx++;

        inputs[idx].type = INPUT_KEYBOARD;
        inputs[idx].ki.wVk = 0;
        inputs[idx].ki.wScan = newText[i];
        inputs[idx].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
        inputs[idx].ki.time = 0;
        inputs[idx].ki.dwExtraInfo = MAGIC_EXTRA_INFO;
        idx++;
    }

    // 4. Send the entire batch at once
    SendInput((UINT)idx, inputs, sizeof(INPUT));
}

// ---------------------------------------------------------------------------
// SendChar – inject a single Unicode character (two INPUT events).
// ---------------------------------------------------------------------------
void InputInjector::SendChar(wchar_t ch) {
    INPUT inputs[2];
    FillUnicodeInput(&inputs[0], ch, 0);
    FillUnicodeInput(&inputs[1], ch, KEYEVENTF_KEYUP);
    SendInput(2, inputs, sizeof(INPUT));
}

// ---------------------------------------------------------------------------
// SendBackspaces – inject N backspace keypresses.
// ---------------------------------------------------------------------------
void InputInjector::SendBackspaces(int count) {
    if (count <= 0) return;
    if (count > 64) count = 64;

    INPUT inputs[128]; // 64 * 2
    int idx = 0;
    for (int i = 0; i < count; i++) {
        FillVkInput(&inputs[idx++], VK_BACK, 0);
        FillVkInput(&inputs[idx++], VK_BACK, KEYEVENTF_KEYUP);
    }
    SendInput((UINT)idx, inputs, sizeof(INPUT));
}

} // namespace CayIME
