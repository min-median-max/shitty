# Soksak provider branch

The `soksak-provider-13` branch extends source release 13 with a headless provider ABI. Soksak
components pin an exact commit from this branch; they do not use a workspace path.

`VtermSnapshot` and `SoksakShittySnapshot` expose the terminal model's cursor shape, blink state,
and 500 ms animation interval. The C ABI retains hidden, filled block, hollow block, underline, and
bar as explicit values. A consumer keeps DECTCEM visibility separate and does not parse the input
stream again.

`soksak_shitty_terminal_pointer` routes normalized cell, event, button, and modifier facts through
the existing `encodeMouseProtocol` implementation using the live terminal's tracking mode and
mouse encoding. The snapshot exposes DEC 9 X10 and DEC 1001 highlight tracking as distinct bits.
X10 input suppresses modifiers at this provider boundary. Consumers do not copy X10, UTF-8, SGR,
URXVT, or motion encoding rules.

The headless SDK selects its portable base64 and hash implementations. Its archives have no
ambient simdutf or xxhash link dependency. `vterm-c-sdk` links and executes the C smoke case before
publishing the SDK tree.

Darwin's system archiver uses `rcs` with `ZERO_AR_DATE=1`; GNU and LLVM archivers use `rcsD`.
Both paths preserve deterministic archive metadata.

The source checks are:

```sh
python3 -m unittest tst.test_provider_cursor_snapshot
python3 -m unittest tst.test_provider_pointer
python3 -m unittest tst.test_build_metadata.BuildMetadataTests.test_static_sdk_bytes_exclude_node_work_paths_and_archive_metadata
```

The component owner must also build the declared SDK twice and compare its bytes before advancing
the pinned commit.
