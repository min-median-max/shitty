/* Copyright (C) 2026 Shitty team. MIT licensed. */
#include "vterm_c.h"

#include "composer.h"
#include "mouse_protocol.h"
#include "vterm.h"
#include "vterm_headless.h"

#include <std/ios/output.h>
#include <std/mem/obj_pool.h>
#include <std/str/builder.h>
#include <std/str/view.h>

#include <new>
#include <memory>
#include <cstring>
#include <cstring>

using namespace stl;

namespace {
    struct ReplyCounter final: Output {
        size_t writeImpl(const void*, size_t length) override {
            ++writes;
            return length;
        }

        uint32_t writes = 0;
    };

    SoksakShittyColor color(CellColor value) {
        switch (value.source()) {
            case CellColor::Source::Indexed:
                return {SOKSAK_SHITTY_COLOR_PALETTE, value.index(), 0, 0};
            case CellColor::Source::Direct: {
                const Color rgb = value.color();
                return {SOKSAK_SHITTY_COLOR_RGB, rgb.red, rgb.green, rgb.blue};
            }
            default:
                return {SOKSAK_SHITTY_COLOR_DEFAULT, 0, 0, 0};
        }
    }

    SoksakShittyColor color(Color value) {
        return {SOKSAK_SHITTY_COLOR_RGB, value.red, value.green, value.blue};
    }

    uint32_t modes(const VtermSnapshot& state) {
        uint32_t result = 0;
        const auto set = [&](uint32_t bit, bool enabled) {
            if (enabled) result |= bit;
        };
        set(SOKSAK_SHITTY_MODE_BRACKETED_PASTE, state.bracketedPaste);
        set(SOKSAK_SHITTY_MODE_APPLICATION_CURSOR, state.applicationCursor);
        set(SOKSAK_SHITTY_MODE_APPLICATION_KEYPAD, state.applicationKeypad);
        set(SOKSAK_SHITTY_MODE_MOUSE_CLICK, state.mouseClick);
        set(SOKSAK_SHITTY_MODE_MOUSE_DRAG, state.mouseDrag);
        set(SOKSAK_SHITTY_MODE_MOUSE_MOTION, state.mouseMotion);
        set(SOKSAK_SHITTY_MODE_SGR_MOUSE, state.sgrMouse);
        set(SOKSAK_SHITTY_MODE_UTF8_MOUSE, state.utf8Mouse);
        set(SOKSAK_SHITTY_MODE_FOCUS_EVENTS, state.focusEvents);
        set(SOKSAK_SHITTY_MODE_ALTERNATE_SCROLL, state.alternateScroll);
        set(SOKSAK_SHITTY_MODE_SHOW_CURSOR, state.showCursor);
        set(SOKSAK_SHITTY_MODE_LINE_WRAP, state.lineWrap);
        set(SOKSAK_SHITTY_MODE_INSERT, state.insert);
        set(SOKSAK_SHITTY_MODE_ALTERNATE_SCREEN, state.alternateScreen);
        return result;
    }
}

struct SoksakShittyTerminal {
    explicit SoksakShittyTerminal(ObjPool::Ref pool_)
        : pool(static_cast<ObjPool::Ref&&>(pool_))
    {
    }

    ObjPool::Ref pool;
    Composer* composer = nullptr;
    ReplyCounter* replies = nullptr;
    VtermHeadless* headless = nullptr;
};

SoksakShittyResult soksak_shitty_terminal_new(
    uint16_t columns, uint16_t rows, SoksakShittyTerminal** terminal) {
    if (terminal == nullptr || columns == 0 || rows == 0) return SOKSAK_SHITTY_INVALID_VALUE;
    *terminal = nullptr;
    try {
        auto result = std::make_unique<SoksakShittyTerminal>(ObjPool::fromMemory());
        result->composer = result->pool->make<Composer>(
            result->pool.mutPtr(), ComposerProfile::HeadlessTerminal);
        result->replies = result->pool->make<ReplyCounter>();
        result->headless = VtermHeadless::create(
            *result->composer, nullptr, result->replies, columns, rows);
        *terminal = result.release();
        return SOKSAK_SHITTY_SUCCESS;
    } catch (...) {
        return SOKSAK_SHITTY_INTERNAL_ERROR;
    }
}

void soksak_shitty_terminal_free(SoksakShittyTerminal* terminal) {
    delete terminal;
}

SoksakShittyResult soksak_shitty_terminal_feed(
    SoksakShittyTerminal* terminal, const uint8_t* data, size_t length) {
    if (terminal == nullptr || (data == nullptr && length != 0)) return SOKSAK_SHITTY_INVALID_VALUE;
    try {
        terminal->headless->feed(reinterpret_cast<const u8*>(data), length);
        return SOKSAK_SHITTY_SUCCESS;
    } catch (...) {
        return SOKSAK_SHITTY_INTERNAL_ERROR;
    }
}

SoksakShittyResult soksak_shitty_terminal_resize(
    SoksakShittyTerminal* terminal, uint16_t columns, uint16_t rows) {
    if (terminal == nullptr || columns == 0 || rows == 0) return SOKSAK_SHITTY_INVALID_VALUE;
    try {
        terminal->headless->resize(columns, rows);
        return SOKSAK_SHITTY_SUCCESS;
    } catch (...) {
        return SOKSAK_SHITTY_INTERNAL_ERROR;
    }
}

SoksakShittyResult soksak_shitty_terminal_snapshot(
    const SoksakShittyTerminal* terminal, SoksakShittySnapshot* snapshot) {
    if (terminal == nullptr || snapshot == nullptr) return SOKSAK_SHITTY_INVALID_VALUE;
    const VtermSnapshot state = terminal->headless->terminal()->snapshot();
    *snapshot = {
        state.historyRows, terminal->replies->writes, modes(state), state.cursorBlinkIntervalMs,
        state.columns, state.rows,
        state.cursorX, state.cursorY, (uint8_t)(state.cursorStyle), (uint8_t)(state.cursorBlink),
    };
    return SOKSAK_SHITTY_SUCCESS;
}

SoksakShittyResult soksak_shitty_terminal_theme_overrides(
    const SoksakShittyTerminal* terminal, SoksakShittyThemeOverrides* overrides) {
    if (terminal == nullptr || overrides == nullptr) return SOKSAK_SHITTY_INVALID_VALUE;
    const VtermThemeOverrides state = terminal->headless->terminal()->themeOverrides();
    overrides->foreground = color(state.foreground);
    overrides->background = color(state.background);
    overrides->cursor = color(state.cursor);
    for (size_t index = 0; index < 256; ++index) {
        overrides->palette[index] = color(state.palette[index]);
    }
    memcpy(overrides->palette_override_mask, state.paletteMask, sizeof(state.paletteMask));
    overrides->foreground_overridden = state.hasForeground;
    overrides->background_overridden = state.hasBackground;
    overrides->cursor_overridden = state.hasCursor;
    return SOKSAK_SHITTY_SUCCESS;
}

SoksakShittyResult soksak_shitty_terminal_pointer(
    const SoksakShittyTerminal* terminal, uint16_t column, uint16_t row,
    SoksakShittyPointerEvent event, int32_t button, uint32_t modifiers,
    uint8_t* output, size_t capacity, size_t* required) {
    if (terminal == nullptr || required == nullptr || (output == nullptr && capacity != 0)) {
        return SOKSAK_SHITTY_INVALID_VALUE;
    }
    MouseEventType type;
    switch (event) {
        case SOKSAK_SHITTY_POINTER_PRESS: type = MouseEventType::Press; break;
        case SOKSAK_SHITTY_POINTER_RELEASE: type = MouseEventType::Release; break;
        case SOKSAK_SHITTY_POINTER_MOTION: type = MouseEventType::Motion; break;
        default: return SOKSAK_SHITTY_INVALID_VALUE;
    }
    try {
        const VtermSnapshot state = terminal->headless->terminal()->snapshot();
        StringBuilder encoded;
        const int motionButton = type == MouseEventType::Motion ? button : 0;
        if (!encodeMouseProtocol(
                encoded, state.mouseEncoding, type, modifiers, motionButton, button,
                (int)column + 1, (int)row + 1)) {
            return SOKSAK_SHITTY_INVALID_VALUE;
        }
        const StringView bytes(encoded);
        *required = bytes.length();
        if (capacity < bytes.length()) return SOKSAK_SHITTY_OUT_OF_SPACE;
        if (!bytes.empty()) memcpy(output, bytes.data(), bytes.length());
        return SOKSAK_SHITTY_SUCCESS;
    } catch (...) {
        return SOKSAK_SHITTY_INTERNAL_ERROR;
    }
}

SoksakShittyResult soksak_shitty_terminal_cell(
    const SoksakShittyTerminal* terminal, int32_t logical_row, uint16_t column,
    SoksakShittyCell* cell, uint32_t* codepoints, size_t capacity, size_t* required) {
    if (terminal == nullptr || cell == nullptr || required == nullptr ||
        (codepoints == nullptr && capacity != 0)) return SOKSAK_SHITTY_INVALID_VALUE;
    VtermSnapshotCell state;
    if (!terminal->headless->terminal()->snapshotCell(logical_row, column, state)) {
        return SOKSAK_SHITTY_INVALID_VALUE;
    }
    const size_t count = state.grapheme.empty() ? (state.cell.uc_pt == 0 ? 0 : 1) : state.grapheme.size();
    *required = count;
    uint16_t attributes = 0;
    if (state.cell.bold) attributes |= SOKSAK_SHITTY_ATTR_BOLD;
    if (state.cell.faint) attributes |= SOKSAK_SHITTY_ATTR_DIM;
    if (state.cell.italic) attributes |= SOKSAK_SHITTY_ATTR_ITALIC;
    if (state.cell.underlined()) attributes |= SOKSAK_SHITTY_ATTR_UNDERLINE;
    if (state.cell.inverse) attributes |= SOKSAK_SHITTY_ATTR_INVERSE;
    if (state.cell.strike) attributes |= SOKSAK_SHITTY_ATTR_STRIKE;
    if (state.cell.conceal) attributes |= SOKSAK_SHITTY_ATTR_HIDDEN;
    *cell = {
        color(state.cell.foreground()), color(state.cell.background()), attributes,
        (uint8_t)(state.cell.dwidth != 0), (uint8_t)(state.cell.dwidth_cont != 0),
        (uint8_t)(state.cell.wrap != 0), state.lineAttribute,
    };
    if (capacity < count) return SOKSAK_SHITTY_OUT_OF_SPACE;
    if (!state.grapheme.empty()) {
        for (size_t index = 0; index < count; ++index) codepoints[index] = state.grapheme[index];
    } else if (count == 1) {
        codepoints[0] = state.cell.uc_pt;
    }
    return SOKSAK_SHITTY_SUCCESS;
}

SoksakShittyResult soksak_shitty_terminal_selection_start(
    SoksakShittyTerminal* terminal, uint16_t column, int32_t logical_row,
    SoksakShittySelectionSide side, SoksakShittySelectionKind kind) {
    if (terminal == nullptr || side < SOKSAK_SHITTY_SELECTION_LEFT || side > SOKSAK_SHITTY_SELECTION_RIGHT ||
        kind < SOKSAK_SHITTY_SELECTION_CELL || kind > SOKSAK_SHITTY_SELECTION_EXTEND) {
        return SOKSAK_SHITTY_INVALID_VALUE;
    }
    try {
        return terminal->headless->terminal()->selectionStartCell(
                   logical_row, column, side == SOKSAK_SHITTY_SELECTION_RIGHT,
                   static_cast<VtermSelectionKind>(kind))
            ? SOKSAK_SHITTY_SUCCESS : SOKSAK_SHITTY_INVALID_VALUE;
    } catch (...) {
        return SOKSAK_SHITTY_INTERNAL_ERROR;
    }
}

SoksakShittyResult soksak_shitty_terminal_selection_update(
    SoksakShittyTerminal* terminal, uint16_t column, int32_t logical_row,
    SoksakShittySelectionSide side) {
    if (terminal == nullptr || side < SOKSAK_SHITTY_SELECTION_LEFT || side > SOKSAK_SHITTY_SELECTION_RIGHT) {
        return SOKSAK_SHITTY_INVALID_VALUE;
    }
    try {
        return terminal->headless->terminal()->selectionUpdateCell(
                   logical_row, column, side == SOKSAK_SHITTY_SELECTION_RIGHT)
            ? SOKSAK_SHITTY_SUCCESS : SOKSAK_SHITTY_INVALID_VALUE;
    } catch (...) {
        return SOKSAK_SHITTY_INTERNAL_ERROR;
    }
}

SoksakShittyResult soksak_shitty_terminal_selection_clear(SoksakShittyTerminal* terminal) {
    if (terminal == nullptr) return SOKSAK_SHITTY_INVALID_VALUE;
    try {
        terminal->headless->terminal()->selectionClear();
        return SOKSAK_SHITTY_SUCCESS;
    } catch (...) {
        return SOKSAK_SHITTY_INTERNAL_ERROR;
    }
}

SoksakShittyResult soksak_shitty_terminal_selection_text(
    SoksakShittyTerminal* terminal, uint8_t* output, size_t capacity, size_t* required) {
    if (terminal == nullptr || required == nullptr || (output == nullptr && capacity != 0)) {
        return SOKSAK_SHITTY_INVALID_VALUE;
    }
    try {
        const VtermTextResult selected = terminal->headless->terminal()->selectionText();
        if (!selected.status) {
            *required = 0;
            return SOKSAK_SHITTY_NO_VALUE;
        }
        *required = selected.text.length();
        if (capacity < selected.text.length()) return SOKSAK_SHITTY_OUT_OF_SPACE;
        if (!selected.text.empty()) memcpy(output, selected.text.data(), selected.text.length());
        return SOKSAK_SHITTY_SUCCESS;
    } catch (...) {
        return SOKSAK_SHITTY_INTERNAL_ERROR;
    }
}

SoksakShittyResult soksak_shitty_terminal_selection_range(
    const SoksakShittyTerminal* terminal, int32_t logical_row, uint16_t* start, uint16_t* end) {
    if (terminal == nullptr || start == nullptr || end == nullptr) return SOKSAK_SHITTY_INVALID_VALUE;
    try {
        return terminal->headless->terminal()->selectionRange(logical_row, *start, *end)
            ? SOKSAK_SHITTY_SUCCESS : SOKSAK_SHITTY_NO_VALUE;
    } catch (...) {
        return SOKSAK_SHITTY_INTERNAL_ERROR;
    }
}
