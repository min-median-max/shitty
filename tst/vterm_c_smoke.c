#include <vterm_c.h>

#include <assert.h>
#include <string.h>

int main(void) {
    SoksakShittyTerminal* terminal = 0;
    assert(soksak_shitty_terminal_new(12, 3, &terminal) == SOKSAK_SHITTY_SUCCESS);
    const uint8_t input[] = {'A', 0x1b, '[', '?', '1', 'h'};
    assert(soksak_shitty_terminal_feed(terminal, input, sizeof(input)) == SOKSAK_SHITTY_SUCCESS);
    SoksakShittySnapshot snapshot;
    assert(soksak_shitty_terminal_snapshot(terminal, &snapshot) == SOKSAK_SHITTY_SUCCESS);
    assert(snapshot.columns == 12 && snapshot.rows == 3);
    assert((snapshot.modes & SOKSAK_SHITTY_MODE_APPLICATION_CURSOR) != 0);
    const uint8_t cursor[] = {0x1b, '[', '6', ' ', 'q'};
    assert(soksak_shitty_terminal_feed(terminal, cursor, sizeof(cursor)) == SOKSAK_SHITTY_SUCCESS);
    assert(soksak_shitty_terminal_snapshot(terminal, &snapshot) == SOKSAK_SHITTY_SUCCESS);
    assert(snapshot.cursor_style == SOKSAK_SHITTY_CURSOR_BAR && snapshot.cursor_blinking == 0);
    assert(snapshot.cursor_blink_interval_ms == 500);
    SoksakShittyCell cell;
    uint32_t codepoint = 0;
    size_t required = 0;
    assert(soksak_shitty_terminal_cell(terminal, 0, 0, &cell, &codepoint, 1, &required) == SOKSAK_SHITTY_SUCCESS);
    assert(required == 1 && codepoint == 'A');
    assert(cell.wrapline == 0);
    SoksakShittyTerminal* selected_terminal = 0;
    assert(soksak_shitty_terminal_new(40, 3, &selected_terminal) == SOKSAK_SHITTY_SUCCESS);
    const uint8_t marker[] = "SELECT_SHITTY_1234567890";
    assert(soksak_shitty_terminal_feed(selected_terminal, marker, sizeof(marker) - 1) == SOKSAK_SHITTY_SUCCESS);
    assert(soksak_shitty_terminal_selection_start(
        selected_terminal, 0, 0, SOKSAK_SHITTY_SELECTION_LEFT, SOKSAK_SHITTY_SELECTION_CELL) == SOKSAK_SHITTY_SUCCESS);
    assert(soksak_shitty_terminal_selection_update(
        selected_terminal, sizeof(marker) - 1, 0, SOKSAK_SHITTY_SELECTION_LEFT) == SOKSAK_SHITTY_SUCCESS);
    size_t text_size = 0;
    assert(soksak_shitty_terminal_selection_text(selected_terminal, 0, 0, &text_size) == SOKSAK_SHITTY_OUT_OF_SPACE);
    uint8_t text[sizeof(marker)] = {0};
    assert(soksak_shitty_terminal_selection_text(selected_terminal, text, sizeof(text), &text_size) == SOKSAK_SHITTY_SUCCESS);
    assert(text_size == sizeof(marker) - 1 && memcmp(text, marker, text_size) == 0);
    uint16_t selection_start = 0, selection_end = 0;
    assert(soksak_shitty_terminal_selection_range(
        selected_terminal, 0, &selection_start, &selection_end) == SOKSAK_SHITTY_SUCCESS);
    assert(selection_start == 0 && selection_end == sizeof(marker) - 2);
    soksak_shitty_terminal_free(selected_terminal);
    assert(soksak_shitty_terminal_resize(terminal, 7, 4) == SOKSAK_SHITTY_SUCCESS);
    assert(soksak_shitty_terminal_snapshot(terminal, &snapshot) == SOKSAK_SHITTY_SUCCESS);
    assert(snapshot.columns == 7 && snapshot.rows == 4);
    soksak_shitty_terminal_free(terminal);
    return 0;
}
