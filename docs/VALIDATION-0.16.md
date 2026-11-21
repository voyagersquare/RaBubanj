# RaBubanj 0.16 validation

Verified 21 November 2026. Tested source: `15565a1deb87611f8230810d2c3f93701b7db2a1`.

[Windows run](https://github.com/voyagersquare/RaBubanj/actions/runs/44881454949) · [Windows x64 ZIP](https://github.com/voyagersquare/RaBubanj/actions/runs/44881454949/artifacts/14697942611) · [pluginval logs](https://github.com/voyagersquare/RaBubanj/actions/runs/44881454949/artifacts/14698102240)

## Build and package

- MSVC Release x64 VST3 and standalone: passed. All **nine** CTest suites passed, zero failures.
- pluginval 1.0.4 strictness **5: SUCCESS**, completed before packaging.
- Downloaded ZIP integrity and SHA256 match the GitHub digest. Both PE binaries are AMD64 and the VST3 manifest is **0.16.0**. All **34** bundled Source/test/CMake/workflow files match the tested commit after Windows line-ending normalization.
- Portable archive: **68,975,224 bytes**, 12 files, no installer. Original wood JPEG and Great Vibes font are embedded byte for byte in both binaries and the source ZIP. The font license matches the source; corresponding JUCE source and required licenses are included.
- Native Linux Release build and all nine suites passed. Mixer and sequencer preview runs passed. Pure sequencer AddressSanitizer/UndefinedBehaviorSanitizer checks passed without errors.
- Inspected 64-step and 128-step final-page previews, including **128 steps at 1120×740, 1280×820 and 1800×1160**. Three-digit step labels, fixed PAD labels, bar headings, viewport status and scrollbar fit within the working area.

## Pattern length and scrolling

- Each of four patterns independently selects and saves **32, 64 or 128 steps** via the STEPS dropdown: **2, 4 or 8 bars in 4/4**. New patterns default to 32 steps.
- The grid keeps 32 visible columns at the existing readable cell size. Vertical/horizontal wheel input or the horizontal scrollbar reaches the final column. PAD labels remain fixed on the left. VIEW shows the actual visible range, such as **97–128 / 128**.
- Eight-step headings, four-dark/four-light cells and stronger bar dividers extend through step 128. Length changes clamp the scroll range, and changing banks returns to that bank's beginning.
- Integration tests exercise wheel navigation in both directions/axes, scrollbar navigation, final-step click and long-press timing, left-drag painting and right-drag erasing in scrolled tail cells, fixed PAD labels, per-bank choices and shorter-length range clamping.

## Audio, state and MIDI

- Host phase, playhead wrap and note streams use the selected length. Bounded, allocation-free scheduling retains sample-accurate signed timing across fractional BPM, blocks, pre-roll and non-grid/multi-cycle host loops. The fixed heap allows one regular and at most one boundary stream per cell; timing sign determines which loop boundary can be crossed.
- Processor tests verify actual audio onset at the final step and the next cycle for 64 and 128 steps, plus exclusion of inactive tail notes after shortening.
- Shortening retains all hidden tail notes and signed timing. Re-expanding restores them; inactive notes are excluded from audio and MIDI export. Project recall verifies inactive tail retention too.
- **Schema 9** stores full 128-column rows, signed timing and each pattern's selected length. Prior schemas use their original 32-column row stride and default length 32. Legacy velocity migration, schema-7 neutral timing and schema-8 signed timing/row positions are tested. Invalid length/cell data restore safely.
- MIDI export uses the selected pattern and its full selected length: 960 PPQ, 240 ticks per step, **7680 / 15360 / 30720 end ticks**. Tests parse final-column notes, wrapped early notes, velocities, selected-bank isolation, exact end markers and shortened exports excluding tail notes.
- Existing MIDI mapping, sample import/recall, DSP, EQ, mixer/sequence navigation, PAD audition, waveform isolation, Space forwarding and four velocity strengths continue to pass.

[32-step page](docs/sequencer-window.png) · [64 steps](docs/sequencer-64-steps.png) · [128 steps](docs/sequencer-128-steps.png) · [128 minimum](docs/sequencer-128-minimum.png) · [128 maximum](docs/sequencer-128-large.png).

Actual Ableton sessions, native Windows DAW sample/MIDI drops, physical hardware and host Space handling have not been exercised here. Tests cover plugin-side actions and simulated host transport. [Previous validation](docs/VALIDATION-0.15.md).

## Checksum

```text
a2a1be3baf79b07c95d851722e562bd80cb64b4dd2194a95453de780f3c6c4e8  RaBubanj-0.16.0-Windows-x64.zip
```

The portable artifact is retained for 30 days and may require GitHub sign-in. Rebuild: **Actions → Windows VST3 → Run workflow**.
