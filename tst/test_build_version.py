# Copyright (C) 2026 Shitty team
# MIT licensed
# See the file LICENSE.MIT for the full license.

import unittest
from pathlib import Path

from build_version import source_version


class BuildVersionTest(unittest.TestCase):
    def test_uses_utc_source_epoch_instead_of_wall_clock(self):
        self.assertEqual(source_version({"SOURCE_DATE_EPOCH": "1704067200"}), "2024.01.01")
        self.assertEqual(source_version({"SOURCE_DATE_EPOCH": "1704153599"}), "2024.01.01")
        self.assertEqual(source_version({"SOURCE_DATE_EPOCH": "1704153600"}), "2024.01.02")

    def test_requires_one_nonnegative_integer_epoch(self):
        for environment in [
            {},
            {"SOURCE_DATE_EPOCH": ""},
            {"SOURCE_DATE_EPOCH": "-1"},
            {"SOURCE_DATE_EPOCH": "+1"},
            {"SOURCE_DATE_EPOCH": "1.0"},
        ]:
            with self.subTest(environment=environment), self.assertRaises(ValueError):
                source_version(environment)

    def test_primary_build_has_no_wall_clock_version_source(self):
        source = Path("build.py").read_text()
        self.assertNotIn("date.today", source)
        self.assertIn('"--format=%ct"', source)
        self.assertIn("source_version(version_environment)", source)


if __name__ == "__main__":
    unittest.main()
