# RaBubanj 0.1 validation

Verified 17 November 2026. Tested source: `4d76d3134d62e0bd8d2dbf036725c58cb0854cf5`.

[Successful native Windows run](https://github.com/voyagersquare/RaBubanj/actions/runs/44145272657) · [Portable ZIP](https://github.com/voyagersquare/RaBubanj/actions/runs/44145272657/artifacts/14356307396) · [Validation logs](https://github.com/voyagersquare/RaBubanj/actions/runs/44145272657/artifacts/14356831482)

## Windows x64

- Native MSVC Release builds of the VST3 and standalone application: passed.
- Both CTest suites, drum engine and processor integration: passed.
- pluginval 1.0.4, strictness 5: **SUCCESS**, with the process explicitly awaited and its exit code checked before packaging.
- pluginval covered scanning/loading, editor lifecycle, opening the editor during audio processing, audio at 44.1/48/96 kHz with 64–1024-sample buffers, state, automation and bus layouts.
- The separate Steinberg SDK validator was not configured; pluginval reports that optional test as skipped. No claim is made for that separate validator.
- Portable ZIP downloaded and checked for archive integrity and expected files: passed; 11 files, 28,248,424 bytes. It contains the complete x64 VST3 bundle, standalone EXE, Croatian instructions, project source, pinned JUCE source and licenses.
- Both Windows payloads were inspected as PE32+ x86-64. Imports use Windows system DLLs; no separate MSVC runtime DLL is imported.

## Additional checks

- Linux Release VST3 and standalone builds: passed.
- Linux CTest drum engine and processor integration: passed.
- Drum engine with AddressSanitizer and UndefinedBehaviorSanitizer at 44.1/48/96 kHz: passed.
- Rendered the actual JUCE editor at 1120 × 740 and 960 × 660; checked visible child bounds and inspected both images. The default render is [0.1 preview](https://github.com/voyagersquare/RaBubanj/blob/8d412a9a93ca1d017af1e5627b0f05c7523b9ab7/docs/preview.png).

The engine checks cover silence, finite audible output, velocity response, GM aliases, pan, mute, one-shot tails, hi-hat choke, bounded voice stealing, deterministic reset and round robin. Processor checks cover MIDI event offsets, exact-position panic including reverb, UI audition, saved mixer state and dense MIDI input.

Actual DAW sessions and physical Windows MIDI/audio hardware have not been exercised. The starter kit uses generated samples; acoustic realism and a recorded multi-microphone library are future work.

## Package checksum

```text
748d9beead10dfc4265f525d5b866ca9ff73f2b554558c6c0dfac6754e47ef73  RaBubanj-0.1.0-Windows-x64.zip
```

GitHub Actions artifacts are retained for 30 days and can require GitHub sign-in to download. The workflow supports manual rebuilds through **Actions → Windows VST3 → Run workflow**.
