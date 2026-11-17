# RaBubanj 0.2 validation

Verified 17 November 2026. Tested source: `217abffb887fae8b4d78fcf761190b027ca45f18`.

[Successful Windows run](https://github.com/voyagersquare/RaBubanj/actions/runs/44154266647) · [Windows ZIP](https://github.com/voyagersquare/RaBubanj/actions/runs/44154266647/artifacts/14361310301) · [pluginval logs](https://github.com/voyagersquare/RaBubanj/actions/runs/44154266647/artifacts/14359119768)

## Native Windows x64

- MSVC Release VST3 and standalone builds: passed.
- All four CTest suites: **passed**, zero failures. Suites: drum engine, parametric EQ, processor integration, and mixer routing/editor.
- pluginval 1.0.4, strictness 5: **SUCCESS**. Its process was explicitly awaited and its exit status checked before packaging.
- pluginval covered loading, editor lifecycle, editor open while processing, audio, project state, automation, editor automation, parameters and bus layouts. The separate Steinberg validator was not configured and its optional test was skipped.
- Final ZIP downloaded, archive integrity checked, and expected VST3 bundle, 0.2.0 manifest, standalone EXE, instructions and corresponding source/license files verified. Eleven files, 28,427,852 bytes. Its SHA256 matches GitHub's artifact digest.

## Audio and state coverage

- Send at zero produces no return; raising a send produces wet audio.
- Post-fader send follows channel gain; channel mute suppresses dry/send, return mute and return fader suppress wet output, and solo excludes unselected drums. Reverb solo isolates wet audio.
- Kick EQ changes Kick audio; a Snare EQ edit leaves Kick unchanged. Return EQ changes wet audio, and global EQ changes the final mix. EQ bypass restores flat response.
- Bell centre gain, shelf asymptotes, low/high cuts and notch response match expected responses. Actual sine-wave gain matches the graph. Tests at 44.1/48/96 kHz include frequency/gain/Q extremes, bypass, band disable, fast automation and independence from block boundaries.
- New EQ/send/reverb parameters and selected channel survive project state recall. Legacy 0.1 level/room values load with unity sends and neutral EQs; previous EQ edits do not leak into legacy projects.
- Existing MIDI-offset, exact-position panic, preview queue, state and dense MIDI tests pass. The drum engine tests continue to pass after splitting it into per-drum stems.

## UI and local checks

- Local Linux Release VST3 and standalone builds and all four CTest suites: passed.
- Drum engine and parametric EQ under AddressSanitizer and UndefinedBehaviorSanitizer: passed.
- Clicking each of the eight channel selectors and Reverb switches the upper channel EQ while the global panel stays on Master.
- Rebound parameter controls edit the selected channel and band only. Graph drag changes frequency/gain, wheel changes Q, and double-click resets gain.
- Visible controls fit their parents at 1120 × 740, 1280 × 820 and 1800 × 1160. Actual JUCE editor snapshots were rendered and inspected. Send percentage and return dB formatting were checked.
- [Preview](docs/preview.png) shows an example Kick EQ edit; factory EQ bands start flat.

Actual Ableton sessions and physical Windows audio/MIDI hardware have not been exercised here. The kit is still generated; this release changes the mixer, routing and EQ rather than adding recorded acoustic samples. The EQ is minimum-phase with a response graph, without dynamic/linear-phase modes or a spectrum analyser.

## Package checksum

```text
16b70a1f4f65000230766094ec1d95d37ee38330d57c0536f8fdc186b8907e37  RaBubanj-0.2.0-Windows-x64.zip
```

Actions artifacts are retained for 30 days and can require GitHub sign-in. Manual rebuild: **Actions → Windows VST3 → Run workflow**. The [0.1 validation record](docs/VALIDATION-0.1.md) remains available.
