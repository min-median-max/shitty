# Soksak provider branch

The `soksak-provider-13` branch extends source release 13 with a headless provider ABI. Soksak
components pin an exact commit from this branch; they do not use a workspace path.

`VtermSnapshot` and `SoksakShittySnapshot` expose the terminal model's cursor shape and blink state.
The C ABI retains hidden, filled block, hollow block, underline, and bar as explicit values. A
consumer keeps DECTCEM visibility separate and does not parse the input stream again.

Darwin's system archiver uses `rcs` with `ZERO_AR_DATE=1`; GNU and LLVM archivers use `rcsD`.
Both paths preserve deterministic archive metadata.

The source checks are:

```sh
python3 -m unittest tst.test_provider_cursor_snapshot
python3 -m unittest tst.test_build_metadata.BuildMetadataTests.test_static_sdk_bytes_exclude_node_work_paths_and_archive_metadata
```

The component owner must also build the declared SDK twice and compare its bytes before advancing
the pinned commit.
