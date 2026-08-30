import pathlib
import unittest


class ProviderPointerTests(unittest.TestCase):
    def test_provider_pointer_uses_live_vterm_mouse_encoder(self):
        header = pathlib.Path("lib/shitty/vterm_c.h").read_text()
        source = pathlib.Path("lib/shitty/vterm_c.cpp").read_text()
        self.assertIn("soksak_shitty_terminal_pointer", header)
        self.assertIn("soksak_shitty_terminal_pointer", source)
        self.assertIn("encodeMouseProtocol", source)
        self.assertIn("state.mouseEncoding", source)

    def test_provider_pointer_uses_distinct_live_tracking_modes(self):
        header = pathlib.Path("lib/shitty/vterm_c.h").read_text()
        source = pathlib.Path("lib/shitty/vterm_c.cpp").read_text()
        self.assertIn("SOKSAK_SHITTY_MODE_MOUSE_X10", header)
        self.assertIn("SOKSAK_SHITTY_MODE_MOUSE_HIGHLIGHT", header)
        self.assertIn("state.mouseMode", source)
        self.assertIn("MouseTrackingMode::X10_Compat", source)


if __name__ == "__main__":
    unittest.main()
