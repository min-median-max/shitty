/* Copyright (C) 2026 Shitty team. MIT licensed. */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SoksakShittyTerminal SoksakShittyTerminal;

typedef enum {
    SOKSAK_SHITTY_SUCCESS = 0,
    SOKSAK_SHITTY_INVALID_VALUE = -1,
    SOKSAK_SHITTY_OUT_OF_SPACE = -2,
    SOKSAK_SHITTY_INTERNAL_ERROR = -3,
} SoksakShittyResult;

typedef enum {
    SOKSAK_SHITTY_COLOR_DEFAULT = 0,
    SOKSAK_SHITTY_COLOR_PALETTE = 1,
    SOKSAK_SHITTY_COLOR_RGB = 2,
} SoksakShittyColorTag;

typedef struct {
    uint8_t tag;
    uint8_t red_or_index;
    uint8_t green;
    uint8_t blue;
} SoksakShittyColor;

typedef struct {
    uint32_t history_rows;
    uint32_t suppressed_replies;
    uint32_t modes;
    uint32_t cursor_blink_interval_ms;
    uint16_t columns;
    uint16_t rows;
    uint16_t cursor_x;
    uint16_t cursor_y;
    uint8_t cursor_style;
    uint8_t cursor_blinking;
} SoksakShittySnapshot;

enum {
    SOKSAK_SHITTY_CURSOR_HIDDEN = 0,
    SOKSAK_SHITTY_CURSOR_BLOCK = 1,
    SOKSAK_SHITTY_CURSOR_HOLLOW_BLOCK = 2,
    SOKSAK_SHITTY_CURSOR_UNDERLINE = 3,
    SOKSAK_SHITTY_CURSOR_BAR = 4,
};

typedef struct {
    SoksakShittyColor foreground;
    SoksakShittyColor background;
    uint16_t attributes;
    uint8_t wide;
    uint8_t wide_continuation;
    uint8_t wrapline;
    uint8_t line_attribute;
} SoksakShittyCell;

enum {
    SOKSAK_SHITTY_MODE_BRACKETED_PASTE = 1u << 0,
    SOKSAK_SHITTY_MODE_APPLICATION_CURSOR = 1u << 1,
    SOKSAK_SHITTY_MODE_APPLICATION_KEYPAD = 1u << 2,
    SOKSAK_SHITTY_MODE_MOUSE_CLICK = 1u << 3,
    SOKSAK_SHITTY_MODE_MOUSE_DRAG = 1u << 4,
    SOKSAK_SHITTY_MODE_MOUSE_MOTION = 1u << 5,
    SOKSAK_SHITTY_MODE_SGR_MOUSE = 1u << 6,
    SOKSAK_SHITTY_MODE_UTF8_MOUSE = 1u << 7,
    SOKSAK_SHITTY_MODE_FOCUS_EVENTS = 1u << 8,
    SOKSAK_SHITTY_MODE_ALTERNATE_SCROLL = 1u << 9,
    SOKSAK_SHITTY_MODE_SHOW_CURSOR = 1u << 10,
    SOKSAK_SHITTY_MODE_LINE_WRAP = 1u << 11,
    SOKSAK_SHITTY_MODE_INSERT = 1u << 12,
    SOKSAK_SHITTY_MODE_ALTERNATE_SCREEN = 1u << 13,
};

enum {
    SOKSAK_SHITTY_ATTR_BOLD = 1u << 0,
    SOKSAK_SHITTY_ATTR_DIM = 1u << 1,
    SOKSAK_SHITTY_ATTR_ITALIC = 1u << 2,
    SOKSAK_SHITTY_ATTR_UNDERLINE = 1u << 3,
    SOKSAK_SHITTY_ATTR_INVERSE = 1u << 4,
    SOKSAK_SHITTY_ATTR_STRIKE = 1u << 5,
    SOKSAK_SHITTY_ATTR_HIDDEN = 1u << 6,
};

SoksakShittyResult soksak_shitty_terminal_new(
    uint16_t columns, uint16_t rows, SoksakShittyTerminal** terminal);
void soksak_shitty_terminal_free(SoksakShittyTerminal* terminal);
SoksakShittyResult soksak_shitty_terminal_feed(
    SoksakShittyTerminal* terminal, const uint8_t* data, size_t length);
SoksakShittyResult soksak_shitty_terminal_resize(
    SoksakShittyTerminal* terminal, uint16_t columns, uint16_t rows);
SoksakShittyResult soksak_shitty_terminal_snapshot(
    const SoksakShittyTerminal* terminal, SoksakShittySnapshot* snapshot);
SoksakShittyResult soksak_shitty_terminal_cell(
    const SoksakShittyTerminal* terminal, int32_t logical_row, uint16_t column,
    SoksakShittyCell* cell, uint32_t* codepoints, size_t capacity, size_t* required);

#ifdef __cplusplus
}
#endif
