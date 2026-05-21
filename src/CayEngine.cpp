#include "CayEngine.h"

// ============================================================================
// CayEngine.cpp  –  Free-style Telex state machine (RULE 3)
//
// Architecture overview
// ---------------------
// The engine maintains two parallel arrays:
//   _buffer[_bufferCount]  – every raw keystroke the user typed
//   _text[_textLen]        – the Unicode output that has been injected so far
//
// On each keydown the engine:
//   1. Checks for special/control keys (Backspace, Escape, Space, etc.)
//   2. Tries to apply the key as a double-key circumflex modifier.
//   3. Tries to apply the key as a hook/breve modifier ('w').
//   4. Tries to apply the key as a tone mark (s/f/r/x/j/z).
//   5. Falls back to appending the character as a plain letter.
//
// After every mutation the updated _text is injected via InputInjector::ReplaceText.
// ============================================================================

namespace Cay {

// ---------------------------------------------------------------------------
// Static helpers
// ---------------------------------------------------------------------------
bool TelexEngine::IsAlpha(wchar_t ch) {
    return (ch >= L'a' && ch <= L'z') || (ch >= L'A' && ch <= L'Z');
}

wchar_t TelexEngine::ToLowerViet(wchar_t c) {
    if (c >= L'A' && c <= L'Z') return c + 32;
    if (c >= 0x00C0 && c <= 0x00DD && c != 0x00D7) return c + 0x20; // Latin-1
    if (c >= 0x0102 && c <= 0x01AF && (c % 2 == 0)) return c + 1;   // Ă, Đ, Ĩ, Ũ, Ơ, Ư
    if (c >= 0x1EA0 && c <= 0x1EF8 && (c % 2 == 0)) return c + 1;   // Ạ..Ỹ (Latin Extended Additional)
    return c;
}

wchar_t TelexEngine::ToUpperViet(wchar_t c) {
    if (c >= L'a' && c <= L'z') return c - 32;
    if (c >= 0x00E0 && c <= 0x00FD && c != 0x00F7) return c - 0x20;
    if (c >= 0x0103 && c <= 0x01B0 && (c % 2 != 0)) return c - 1;
    if (c >= 0x1EA1 && c <= 0x1EF9 && (c % 2 != 0)) return c - 1;
    return c;
}

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
TelexEngine::TelexEngine() {
    _canRestore = false;
    ResetState();
}

// ---------------------------------------------------------------------------
// ResetState – zero everything without touching the display.
// ---------------------------------------------------------------------------
void TelexEngine::ResetState() {
    _bufferCount = 0;
    _textLen     = 0;
    _toneIndex   = -1;
    for (int i = 0; i < MAX_BUFFER; i++) {
        _buffer[i].raw    = 0;
        _buffer[i].output = 0;
        _text[i]          = 0;
    }
}

// ---------------------------------------------------------------------------
// ResetFull – discard buffer and invalidate recall state.
// ---------------------------------------------------------------------------
void TelexEngine::ResetFull() {
    ResetState();
    _canRestore = false;
}

// ---------------------------------------------------------------------------
// SaveState – cache the current word state for recall.
// ---------------------------------------------------------------------------
void TelexEngine::SaveState() {
    if (_bufferCount > 0) {
        _savedBufferCount = _bufferCount;
        for (int i = 0; i < _bufferCount; i++) _savedBuffer[i] = _buffer[i];
        
        _savedTextLen = _textLen;
        for (int i = 0; i < _textLen; i++) _savedText[i] = _text[i];
        
        _savedToneIndex = _toneIndex;
        _canRestore = true;
    }
}

// ---------------------------------------------------------------------------
// CommitWord – save state for recall, then reset.
// ---------------------------------------------------------------------------
void TelexEngine::CommitWord() {
    SaveState();
    ResetState();
}

// ---------------------------------------------------------------------------
// Commit – replace the currently displayed word with _text[0.._textLen).
// ---------------------------------------------------------------------------
void TelexEngine::Commit(int extraBs) {
    int bs = _textLen + extraBs;
    if (bs < 0) bs = 0;
    CayIME::InputInjector::ReplaceText(bs, _text, _textLen);
}

// ---------------------------------------------------------------------------
// FallbackToRaw – revert to the raw ASCII characters the user typed.
// Called when the engine decides the input is English.
// ---------------------------------------------------------------------------
void TelexEngine::FallbackToRaw() {
    // Build raw ASCII string from _buffer.
    wchar_t raw[MAX_BUFFER];
    int rawLen = 0;
    for (int i = 0; i < _bufferCount && rawLen < MAX_BUFFER - 1; i++) {
        raw[rawLen++] = _buffer[i].raw;
    }
    raw[rawLen] = L'\0';

    CayIME::InputInjector::ReplaceText(_textLen, raw, rawLen);

    // Update _text to reflect the fallback.
    for (int i = 0; i < rawLen; i++) _text[i] = raw[i];
    _textLen = rawLen;
    _toneIndex = -1;
    _canRestore = false;
}

// ---------------------------------------------------------------------------
// ShouldBypassWord
//
// Returns true if the current typed sequence looks English and should bypass
// Vietnamese transformation.
//
// RULE 3 requirement: MUST start with HasVietnameseMark check to protect
// already-accented valid words.
// ---------------------------------------------------------------------------
bool TelexEngine::ShouldBypassWord() const {
    if (CayData::HasVietnameseMark(_text, _textLen)) return false;
    if (_bufferCount == 0) return false;

    // Trích xuất các phím gốc đã gõ (chuyển về chữ thường để dễ check)
    wchar_t raw[16] = {0};
    int len = _bufferCount < 15 ? _bufferCount : 15;
    for(int i = 0; i < len; i++) {
        raw[i] = ToLowerViet(_buffer[i].raw);
    }

    // LEVEL 1: Bắt đầu bằng w, f, j, z -> Bỏ qua ngay
    if (raw[0] == L'w' || raw[0] == L'f' || raw[0] == L'j' || raw[0] == L'z') return true;

    // LEVEL 2: Sai quy tắc chính tả tiếng Việt
    if (len >= 2) {
        // q không đi với u (qa, qe...)
        if (raw[0] == L'q' && raw[1] != L'u') return true;
        
        // c không đi với i, e, ê, y
        if (raw[0] == L'c' && (raw[1] == L'i' || raw[1] == L'e' || raw[1] == L'\u00EA' || raw[1] == L'y')) return true;
        
        // k bắt buộc đi với h, i, e, ê, y
        if (raw[0] == L'k' && !(raw[1] == L'h' || raw[1] == L'i' || raw[1] == L'e' || raw[1] == L'\u00EA' || raw[1] == L'y')) return true;
        
        // g không đi với e, ê, y (gh thì hợp lệ, check ở dưới)
        if (raw[0] == L'g' && (raw[1] == L'e' || raw[1] == L'\u00EA' || raw[1] == L'y')) return true;
        
        // gh bắt buộc đi với i, e, ê, y
        if (len >= 3 && raw[0] == L'g' && raw[1] == L'h') {
            if (raw[2] != L'i' && raw[2] != L'e' && raw[2] != L'\u00EA' && raw[2] != L'y') return true;
        }

        // ng không đi với i, e, ê, y
        if (len >= 3 && raw[0] == L'n' && raw[1] == L'g' && raw[2] != L'h') {
            if (raw[2] == L'i' || raw[2] == L'e' || raw[2] == L'\u00EA' || raw[2] == L'y') return true;
        }

        // ngh bắt buộc đi với i, e, ê, y
        if (len >= 4 && raw[0] == L'n' && raw[1] == L'g' && raw[2] == L'h') {
            if (raw[3] != L'i' && raw[3] != L'e' && raw[3] != L'\u00EA' && raw[3] != L'y') return true;
        }
    }

    // Heuristic an toàn: Gõ đến 5 ký tự mà chưa có nguyên âm nào thì chắc chắn là tiếng Anh/từ viết tắt
    bool hasVowel = false;
    for (int i = 0; i < len; i++) {
        if (CayData::IsVowel(raw[i])) { hasVowel = true; break; }
    }
    if (!hasVowel && len >= 5) return true;

    return false;
}

// ---------------------------------------------------------------------------
// FindTonePosition
//
// Returns the index in _text[] where the tone mark should be placed.
// ---------------------------------------------------------------------------
int TelexEngine::FindTonePosition() const {
    if (_textLen == 0) return -1;

    // Collect vowel positions.
    int first = -1, last = -1, count = 0;
    for (int i = 0; i < _textLen; i++) {
        if (CayData::IsVowel(_text[i])) {
            if (first == -1) first = i;
            last = i;
            count++;
        }
    }
    if (count == 0) return -1;
    if (count == 1) return first;

    // Does a consonant follow the last vowel?
    bool hasConsonantFinal = false;
    for (int i = last + 1; i < _textLen; i++) {
        if (IsAlpha(_text[i]) && !CayData::IsVowel(_text[i])) {
            hasConsonantFinal = true;
            break;
        }
    }

    // Helper: get plain ASCII base of a potentially toned/hooked vowel.
    auto baseVowel = [](wchar_t c) -> wchar_t {
        return ToLowerViet(CayData::StripAccent(CayData::StripTone(c)));
    };

    if (count == 2) {
        if (hasConsonantFinal) return last; // e.g. "tuấn", "điện"

        wchar_t v1 = baseVowel(_text[first]);
        wchar_t v2 = baseVowel(_text[last]);

        // oa, oe
        if (v1 == L'o' && (v2 == L'a' || v2 == L'e')) return last;
        // uê, uy, uơ
        if (v1 == L'u' && (v2 == L'e' || v2 == L'y' || v2 == L'o')) return last;
        // iê
        if (v1 == L'i' && v2 == L'e') return last;

        // qu + vowel (e.g. quá)
        if (v1 == L'u' && first > 0 && ToLowerViet(_text[first - 1]) == L'q') return last;
        // gi + vowel (e.g. già)
        if (v1 == L'i' && first > 0 && ToLowerViet(_text[first - 1]) == L'g') return last;

        // Default for open 2-vowel syllable: first vowel (e.g. rồi, mèo, đôi, bơi, múa)
        return first;
    }

    if (count >= 3) {
        if (hasConsonantFinal) return last; // e.g. "tuyến", "giường"

        wchar_t v1 = baseVowel(_text[first]);
        wchar_t v2 = baseVowel(_text[first + 1]);
        wchar_t v3 = baseVowel(_text[last]);

        // uyê (incomplete "nguyễ", "chuyế") -> tone on ê
        if (v1 == L'u' && v2 == L'y' && v3 == L'e') return last;
        // giuô, giươ (incomplete "giuộ", "giượ") -> tone on ô/ơ
        if (v1 == L'i' && v2 == L'u' && v3 == L'o') return last;

        // Default 3-vowel: middle vowel (e.g. người, ngoài, khuya)
        return first + 1;
    }

    return last;
}


// ---------------------------------------------------------------------------
// StripAllTones – rewrite _text[] removing any tone mark from every vowel.
// ---------------------------------------------------------------------------
void TelexEngine::StripAllTones() {
    for (int i = 0; i < _textLen; i++) {
        wchar_t stripped = CayData::StripTone(_text[i]);
        _text[i] = stripped;
    }
}

// ---------------------------------------------------------------------------
// ApplyDoubleKeys
//
// Handles aa->â, ee->ê, oo->ô, dd->đ.
//
// RULE 3 backward-scan logic:
//   When the user types a vowel key (a/e/o) or 'd':
//     Scan _text[] backward for the nearest plain matching ASCII character.
//     If found and it is still plain -> transform it (circumflex / stroke).
//     If found and it is already circumflexed -> UNDO: revert to plain and
//       append a literal copy of the key (toggle behaviour).
//     If not found -> append plain character (handled by the caller).
//
// Returns true if the key was consumed as a modifier; false to fall through.
// ---------------------------------------------------------------------------
bool TelexEngine::ApplyDoubleKeys(wchar_t key) {
    wchar_t lo = ToLowerViet(key);

    // Which plain target character does this key double-up on?
    wchar_t target = 0;
    wchar_t replacement = 0;
    wchar_t already = 0; // the already-transformed character (for undo detection)

    switch (lo) {
    case L'a':
        target      = L'a';
        replacement = L'\u00E2'; // â
        already     = L'\u00E2';
        break;
    case L'e':
        target      = L'e';
        replacement = L'\u00EA'; // ê
        already     = L'\u00EA';
        break;
    case L'o':
        target      = L'o';
        replacement = L'\u00F4'; // ô
        already     = L'\u00F4';
        break;
    case L'd':
        target      = L'd';
        replacement = L'\u0111'; // đ
        already     = L'\u0111';
        break;
    default:
        return false; // Not a double-key candidate.
    }

    // Scan backward through _text[] for the target.
    for (int i = _textLen - 1; i >= 0; i--) {
        wchar_t tc = _text[i];
        wchar_t tcLo = ToLowerViet(tc);

        // Case A: found a plain target – transform it.
        if (tcLo == target) {
            // Preserve case.
            wchar_t r = (tc >= L'A' && tc <= L'Z') ?
                        (wchar_t)(replacement - 0x20) : // uppercase variant (may not exist, falls back)
                        replacement;
            // Vietnamese uppercase circumflex: Â Ê Ô Đ
            if (tc >= L'A' && tc <= L'Z') {
                switch (replacement) {
                case L'\u00E2': r = L'\u00C2'; break; // Â
                case L'\u00EA': r = L'\u00CA'; break; // Ê
                case L'\u00F4': r = L'\u00D4'; break; // Ô
                case L'\u0111': r = L'\u0110'; break; // Đ
                }
            }
            _text[i] = r;
            // If a tone was active, re-apply it at the new position.
            if (_toneIndex >= 0) {
                wchar_t toned = CayData::GetToneMark(_text[i], _toneIndex);
                if (toned) _text[i] = toned;
            }
            Commit(0);
            return true;
        }

        // Case B: found the already-transformed character – UNDO.
        if (tc == already ||
            // Also catch toned variant of the circumflex (e.g. â + sắc = ấ)
            CayData::StripAccent(CayData::StripTone(tc)) == target) {
            // Revert to plain.
            _text[i] = (wchar_t)target;
            // Re-apply current tone if any.
            if (_toneIndex >= 0) {
                wchar_t toned = CayData::GetToneMark(_text[i], _toneIndex);
                if (toned) _text[i] = toned;
            }
            // Append the literal key as a new character.
            if (_textLen < MAX_BUFFER - 1) {
                _text[_textLen++] = lo;
            }
            Commit(0);
            return true;
        }

        // Non-vowel consonant – stop scanning (don't cross consonant boundaries).
        if (!CayData::IsVowel(tcLo) && tcLo != L'd' && tcLo != L'\u0111') {
            break;
        }
    }

    return false;
}

// ---------------------------------------------------------------------------
// ApplyHookKeys
//
// Handles the 'w' hook key:
//   ow (in context of uo) -> ươ  (both o and preceding u get hooks)
//   aw (in context of ua) -> ưă  (u -> ư, a stays but gets breve via aw->ă)
//   a -> ă,  o -> ơ,  u -> ư
//
// RULE 3: backward-scan with full undo support.
// ---------------------------------------------------------------------------
bool TelexEngine::ApplyHookKeys(wchar_t key) {
    if (ToLowerViet(key) != L'w') return false;

    // Scan backward for the first vowel.
    for (int i = _textLen - 1; i >= 0; i--) {
        wchar_t tc  = _text[i];
        wchar_t tcB = CayData::StripTone(tc); // base without tone
        wchar_t tcA = CayData::StripAccent(tcB); // plain ASCII

        // ----------------------------------------------------------------
        // Special case: 'o' preceded by 'u' -> hook BOTH (uo -> ươ)
        // This is the "truowng" -> "trương" rule.
        // ----------------------------------------------------------------
        if ((tcA == L'o') && i > 0) {
            wchar_t prev  = _text[i - 1];
            wchar_t prevB = CayData::StripTone(prev);
            wchar_t prevA = CayData::StripAccent(prevB);

            if (prevA == L'u') {
                // Check undo: if 'u' is already ư and 'o' is already ơ -> revert.
                bool alreadyHooked = (prevB == L'\u01B0' || CayData::StripTone(prev) == L'\u01B0') &&
                                     (tcB   == L'\u01A1' || CayData::StripTone(tc)   == L'\u01A1');
                if (alreadyHooked) {
                    // Undo: revert both to plain.
                    _text[i - 1] = prevA; // u
                    _text[i]     = tcA;   // o
                    // Re-apply tone if any.
                    if (_toneIndex >= 0) {
                        int tp = FindTonePosition();
                        if (tp >= 0) {
                            wchar_t toned = CayData::GetToneMark(_text[tp], _toneIndex);
                            if (toned) _text[tp] = toned;
                        }
                    }
                    // Append literal 'w'.
                    if (_textLen < MAX_BUFFER - 1) _text[_textLen++] = L'w';
                    Commit(0);
                    return true;
                }

                // Apply: u -> ư, o -> ơ.
                _text[i - 1] = L'\u01B0'; // ư
                _text[i]     = L'\u01A1'; // ơ
                // Re-apply tone.
                if (_toneIndex >= 0) {
                    int tp = FindTonePosition();
                    if (tp >= 0) {
                        wchar_t toned = CayData::GetToneMark(_text[tp], _toneIndex);
                        if (toned) _text[tp] = toned;
                    }
                }
                Commit(0);
                return true;
            }
        }

        // ----------------------------------------------------------------
        // Special case: 'a' preceded by 'u' -> hook 'u' only (ua+w = ưa)
        // CRITICAL: 'a' must NOT be changed to 'ă'. Only 'u' -> 'ư'.
        // ----------------------------------------------------------------
        if ((tcA == L'a') && i > 0) {
            wchar_t prev  = _text[i - 1];
            wchar_t prevStripTone = CayData::StripTone(prev);
            wchar_t prevA = CayData::StripAccent(prevStripTone);
            if (prevA == L'u') {
                // Undo: if 'u' is already ư (U+01B0) -> revert to 'u', append literal 'w'.
                bool alreadyHooked = (prevStripTone == L'\u01B0');
                if (alreadyHooked) {
                    _text[i - 1] = (prev >= L'A' && prev <= L'Z') ? L'U' : L'u';
                    // 'a' stays as 'a' (no change needed)
                    if (_textLen < MAX_BUFFER - 1) _text[_textLen++] = L'w';
                    Commit(0);
                    return true;
                }
                // Apply: only hook 'u' -> 'ư'. Keep 'a' unchanged (ua+w = ưa, NOT ưă).
                _text[i - 1] = (prev >= L'A' && prev <= L'Z') ? L'\u01AF' : L'\u01B0'; // ư
                // _text[i] ('a') is intentionally NOT changed.
                if (_toneIndex >= 0) {
                    int tp = FindTonePosition();
                    if (tp >= 0) {
                        wchar_t toned = CayData::GetToneMark(_text[tp], _toneIndex);
                        if (toned) _text[tp] = toned;
                    }
                }
                Commit(0);
                return true;
            }
        }

        // ----------------------------------------------------------------
        // General single-vowel hook.
        // ----------------------------------------------------------------
        if (!CayData::IsVowel(tc)) {
            // Not a vowel, keep scanning.
            continue;
        }

        wchar_t hooked = 0;
        wchar_t plain  = tcA;

        switch (plain) {
        case L'a': hooked = L'\u0103'; break; // ă
        case L'o': hooked = L'\u01A1'; break; // ơ
        case L'u': hooked = L'\u01B0'; break; // ư
        default:   break;
        }

        if (!hooked) {
            // This vowel can't take a hook – keep scanning.
            continue;
        }

        // Undo: if already hooked, revert and append 'w'.
        if (tcB == hooked) {
            _text[i] = plain;
            if (_toneIndex >= 0) {
                wchar_t toned = CayData::GetToneMark(_text[i], _toneIndex);
                if (toned) _text[i] = toned;
            }
            if (_textLen < MAX_BUFFER - 1) _text[_textLen++] = L'w';
            Commit(0);
            return true;
        }

        // Apply hook.
        _text[i] = hooked;
        if (_toneIndex >= 0) {
            wchar_t toned = CayData::GetToneMark(_text[i], _toneIndex);
            if (toned) _text[i] = toned;
        }
        Commit(0);
        return true;
    }

    return false;
}

// ---------------------------------------------------------------------------
// ApplyToneMarks
//
// Applies or changes the tone mark on the appropriate vowel of _text[].
// If the same tone is applied twice, removes it (toggle).
// ---------------------------------------------------------------------------
bool TelexEngine::ApplyToneMarks(int toneIndex) {
    if (_textLen == 0) return false;

    int currentTone = 0;
    int tonePos = -1;

    // 1. Identify if the word currently has a tone and where it is
    for (int i = 0; i < _textLen; i++) {
        wchar_t c = _text[i];
        wchar_t base = CayData::StripTone(c);
        if (base != c) {
            tonePos = i;
            for (int t = 1; t <= 5; t++) {
                if (CayData::GetToneMark(base, t) == c || 
                    CayData::GetToneMark(ToLowerViet(base), t) == ToLowerViet(c)) {
                    currentTone = t;
                    break;
                }
            }
            break;
        }
    }

    // 2. Handle 'z' key (toneIndex == 0)
    if (toneIndex == 0) {
        if (currentTone > 0) {
            _text[tonePos] = CayData::StripTone(_text[tonePos]); // Remove tone
            _toneIndex = -1;
            Commit(0);
            return true; // Consumed 'z', do not append it
        }
        return false; // No tone to remove, return false so 'z' gets appended normally
    }

    // 3. Handle double-typing the SAME tone key (Undo tone and append raw key)
    if (currentTone == toneIndex) {
        _text[tonePos] = CayData::StripTone(_text[tonePos]); // Remove tone
        _toneIndex = -1;
        Commit(0);
        return false; // Return false so the raw tone key (e.g., 's') gets appended
    }

    // 4. Apply or replace tone
    int targetPos = (tonePos >= 0) ? tonePos : FindTonePosition();
    if (targetPos >= 0) {
        wchar_t originalChar = _text[targetPos];
        // Kiểm tra xem chữ gốc có phải là chữ IN HOA không
        bool isUpper = (originalChar == ToUpperViet(originalChar) && originalChar != ToLowerViet(originalChar));

        wchar_t base = CayData::StripTone(originalChar);
        wchar_t baseLo = ToLowerViet(base); // Đưa về chữ thường để lấy dấu trong thư viện

        wchar_t tonedLo = CayData::GetToneMark(baseLo, toneIndex);
        if (tonedLo != L'\0') {
            // Nâng lên lại IN HOA nếu cần
            _text[targetPos] = isUpper ? ToUpperViet(tonedLo) : tonedLo;
            _toneIndex = toneIndex;
            Commit(0);
            return true;
        }
    }

    return false;
}

// ---------------------------------------------------------------------------
// OnKeyDown – main entry point
// ---------------------------------------------------------------------------
void TelexEngine::OnKeyDown(CayIME::InputHookManager* sender, CayIME::HookKeyEventArgs& e) {
    DWORD vk = e.keyCode;

    // -----------------------------------------------------------------------
    // 1. Non-alpha keys that reset or terminate the current word.
    // -----------------------------------------------------------------------
    switch (vk) {
    case VK_BACK:
        if (_bufferCount == 0 && _canRestore) {
            e.handled = true;
            
            // Restore state
            _bufferCount = _savedBufferCount;
            for (int i = 0; i < _bufferCount; i++) _buffer[i] = _savedBuffer[i];
            
            _textLen = _savedTextLen;
            for (int i = 0; i < _textLen; i++) _text[i] = _savedText[i];
            
            _toneIndex = _savedToneIndex;
            _canRestore = false;

            // Send 1 backspace to OS to delete the space or punctuation that followed the word
            CayIME::InputInjector::ReplaceText(1, nullptr, 0);
            return;
        }

        if (_bufferCount > 0) {
            e.handled = true; // suppress the raw backspace
            _canRestore = false;
            // Remove last character from our text.
            if (_textLen > 0) _textLen--;
            if (_bufferCount > 0) _bufferCount--;

            // Re-derive tone index.
            _toneIndex = -1;
            for (int i = 0; i < _bufferCount; i++) {
                int ti = CayData::GetToneIndex(_buffer[i].raw);
                if (ti >= 0) _toneIndex = ti;
            }

            if (_bufferCount == 0) {
                // Send one backspace to clear the last remaining displayed char.
                CayIME::InputInjector::ReplaceText(1, nullptr, 0);
                ResetState();
            } else {
                Commit(1); // +1 because we already decremented _textLen
            }
        }
        return;

    case VK_ESCAPE:
        ResetFull();
        return;

    case VK_RETURN:
    case VK_TAB:
    case VK_SPACE:
        if (_bufferCount > 0) CommitWord();
        else ResetFull();
        return;

    case VK_LEFT: case VK_RIGHT: case VK_UP: case VK_DOWN:
    case VK_HOME: case VK_END:  case VK_PRIOR: case VK_NEXT:
    case VK_DELETE:
        ResetFull();
        return;
    }

    // -----------------------------------------------------------------------
    // 2. Only process printable ASCII alpha characters.
    // -----------------------------------------------------------------------
    if (vk < 'A' || vk > 'Z') {
        // Non-alpha printable (digits, punctuation) – commit word.
        if (_bufferCount > 0) CommitWord();
        else ResetFull();
        return;
    }

    // Determine the actual character pressed (respecting Shift).
    bool shifted = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    bool capsLk  = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
    bool upper   = shifted ^ capsLk;
    wchar_t ch   = upper ? (wchar_t)vk : (wchar_t)(vk + 32);
    wchar_t lo   = ToLowerViet(ch);

    // -----------------------------------------------------------------------
    // 3. Guard: buffer overflow -> fall through as plain text.
    // -----------------------------------------------------------------------
    if (_bufferCount >= MAX_BUFFER - 1 || _textLen >= MAX_BUFFER - 1) {
        ResetFull();
        CayIME::InputInjector::SendChar(ch);
        return;
    }

    // -----------------------------------------------------------------------
    // 4. Record raw keystroke.
    // -----------------------------------------------------------------------
    if (_bufferCount == 0) _canRestore = false;

    _buffer[_bufferCount].raw    = ch;
    _buffer[_bufferCount].output = ch; // will be updated after transformation
    _bufferCount++;

    // -----------------------------------------------------------------------
    // 5. Try modifier keys (bypass = english mode check).
    // -----------------------------------------------------------------------
    bool bypass = ShouldBypassWord();

    // 5a. Double-key circumflex / stroke (aa oo ee dd).
    if (!bypass && (lo == L'a' || lo == L'e' || lo == L'o' || lo == L'd')) {
        if (_textLen > 0 && ApplyDoubleKeys(ch)) {
            e.handled = true;
            return;
        }
    }

    // 5b. Hook key 'w'.
    if (!bypass && lo == L'w' && _textLen > 0) {
        if (ApplyHookKeys(ch)) {
            e.handled = true;
            return;
        }
    }

    // 5c. Tone mark keys (s f r x j z).
    if (!bypass) {
        int ti = CayData::GetToneIndex(lo);
        if (ti >= 0 && _textLen > 0) {
            if (ApplyToneMarks(ti)) {
                e.handled = true;
                return;
            }
        }
    }

    // -----------------------------------------------------------------------
    // 6. Plain character – append to text buffer and inject.
    // -----------------------------------------------------------------------
    _text[_textLen++] = ch;
    _text[_textLen]   = L'\0';

    e.handled = true;
    CayIME::InputInjector::SendChar(ch);
}

// ---------------------------------------------------------------------------
// OnKeyUp – currently unused; reserved for future modifier tracking.
// ---------------------------------------------------------------------------
void TelexEngine::OnKeyUp(CayIME::InputHookManager* sender, CayIME::HookKeyEventArgs& e) {
    // No-op for now.
    (void)sender; (void)e;
}

} // namespace Cay
