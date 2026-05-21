#pragma once
#include <windows.h>

namespace CayIME {

class InputInjector {
public:
    // Sentinel value stamped on every synthetic event so our own hook ignores them.
    static const ULONG_PTR MAGIC_EXTRA_INFO = 0x1234;

    // Replace text at the current caret position.
    // Sends: [backspaceCount x VK_BACK] + [newText characters]
    // all in a single SendInput call to prevent Chrome autocomplete races.
    static void ReplaceText(int backspaceCount, const wchar_t* newText, int newTextLen);

    // Inject a single Unicode character.
    static void SendChar(wchar_t ch);

    // Inject a sequence of backspaces.
    static void SendBackspaces(int count);
};

} // namespace CayIME
