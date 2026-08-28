#include <vterm_c.h>

#include <assert.h>

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
    SoksakShittyCell cell;
    uint32_t codepoint = 0;
    size_t required = 0;
    assert(soksak_shitty_terminal_cell(terminal, 0, 0, &cell, &codepoint, 1, &required) == SOKSAK_SHITTY_SUCCESS);
    assert(required == 1 && codepoint == 'A');
    assert(cell.wrapline == 0);
    assert(soksak_shitty_terminal_resize(terminal, 7, 4) == SOKSAK_SHITTY_SUCCESS);
    assert(soksak_shitty_terminal_snapshot(terminal, &snapshot) == SOKSAK_SHITTY_SUCCESS);
    assert(snapshot.columns == 7 && snapshot.rows == 4);
    soksak_shitty_terminal_free(terminal);
    return 0;
}
