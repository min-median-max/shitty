# Copyright (C) 2026 Shitty team
# MIT licensed
# See the file LICENSE.MIT for the full license.

from datetime import datetime, timezone
from typing import Mapping


def source_version(environment: Mapping[str, str]) -> str:
    raw = environment.get("SOURCE_DATE_EPOCH", "")
    if not raw or not raw.isascii() or not raw.isdecimal():
        raise ValueError("SOURCE_DATE_EPOCH must be one nonnegative integer UTC epoch")
    try:
        instant = datetime.fromtimestamp(int(raw), timezone.utc)
    except (OverflowError, OSError, ValueError) as error:
        raise ValueError("SOURCE_DATE_EPOCH is outside the supported UTC range") from error
    return instant.strftime("%Y.%m.%d")
