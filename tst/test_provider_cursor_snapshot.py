from pathlib import Path
import unittest


class ProviderCursorSnapshotTest(unittest.TestCase):
    def test_c_snapshot_exposes_engine_cursor_state(self):
        header = Path("lib/shitty/vterm_c.h").read_text()
        implementation = Path("lib/shitty/vterm_c.cpp").read_text()
        self.assertIn("cursor_style", header)
        self.assertIn("cursor_blinking", header)
        self.assertIn("state.cursorStyle", implementation)
        self.assertIn("state.cursorBlink", implementation)


if __name__ == "__main__":
    unittest.main()
