/* Copyright (C) 2026 Shitty team. MIT licensed. */
#include "vterm_c.h"

#include "composer.h"
#include "vterm.h"
#include "vterm_headless.h"

#include <std/ios/output.h>
#include <std/mem/obj_pool.h>

#include <new>
#include <memory>

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
        state.historyRows, terminal->replies->writes, modes(state), state.columns, state.rows,
        state.cursorX, state.cursorY,
    };
    return SOKSAK_SHITTY_SUCCESS;
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
