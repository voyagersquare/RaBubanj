# RaBubanj 0.15 validation

Verified 21 November 2026. Tested source: `0e614e324b626ab28933d515377b3c71e04a8fe1`.

[Windows run](https://github.com/voyagersquare/RaBubanj/actions/runs/44879091226) · [Windows x64 ZIP](https://github.com/voyagersquare/RaBubanj/actions/runs/44879091226/artifacts/14697371613) · [pluginval logs](https://github.com/voyagersquare/RaBubanj/actions/runs/44879091226/artifacts/14697386561)

## Build and package

- MSVC Release x64 VST3 and standalone: passed. All **nine** CTest suites passed, zero failures.
- pluginval 1.0.4 strictness **5: SUCCESS**, completed before packaging.
- Downloaded ZIP integrity and SHA256 match the GitHub digest. Both PE binaries are AMD64 and the VST3 manifest is **0.15.0**. All **34** bundled Source/test/CMake/workflow files match the tested commit after Windows line-ending normalization.
- Portable archive: **65,727,234 bytes**, 12 files, no installer. Original wood JPEG and Great Vibes font are embedded byte for byte in both binaries and the source ZIP. Font license matches the source; JUCE source and required licenses are included.
- Native Linux Release build and all nine suites passed. Mixer and sequencer preview runs passed. Mixer and sequencer views were visually inspected at **1120×740, 1280×820 and 1800×1160**, plus the timing popup.

## Dedicated sequencer page

- STEP SEQ replaces the mixer below the shared plugin header. Mixer strips, EQ panels and Reverb/Pitch/Drive/Comp controls are hidden; their painted legends are omitted too. No clipped mixer controls appear behind the sequencer.
- The upper navigation button changes to **MIXER** while sequencing. MIXER and the red **X** share a return callback that immediately restores all mixer controls. The sequencer page fills the working area on resize and cannot be dragged over other components.
- Integration coverage checks hidden/restored controls and EQ panels, working-area layout at all three sizes, unobstructed navigation/close hit targets, repeated open/close, look-and-feel changes, immediate waveform capture shutdown and unhandled Space for host transport.

## Readability and materials

- A **48% matte red-brown finish** calms grain contrast while preserving the warm reddish appearance. The original user JPEG remains unchanged. Raised wood frames, existing metal knobs/faders and the dark-red script wordmark remain.
- Sequencer title, tempo/status, editing help and timing-popup wood captions are **larger, bold and black**. Text on dark controls/screens stays light for contrast.
- Eight-step groups have bar/range headings and vertical separators after steps 8 and 24; a stronger gold separator divides steps **16 and 17**. Existing four-dark/four-light cells, velocity bars, playhead and timing markers remain.
- Waveform, grid and both EQ screen interiors are **opaque solid dark**, keeping wood grain on their carved frames. Actual selected-pad waveform capture and spectrum processing remain unchanged.

## Regression and assets

All nine suites cover the engine/pitch, EQ, legacy sources, processor/MIDI/state, mixer/editor, sample import/recall, channel spectrum, sequencer timing and sequencer/MIDI/editor integration. Four velocity levels, left painting/right erasing, long-press timing, selected-pad audition, pattern recall/migration, host synchronization, MIDI export and waveform isolation continue to pass. No DSP, parameter identity, state schema or sequencing algorithm changes.

Original wood JPEG: 7680×4302, 9,501,641 bytes, SHA256 `5f664206eec18dae81a2e6d4ca04dc0be0967afd1d981d44fdf7c87650f7cca5`.
Embedded Great Vibes font: 457,588 bytes, SHA256 `8d509802186f1b51572531ecf313e8098f9a5bfdfaca93f0c9b34467f9982d15`.

[Mixer](docs/preview.png) · [Sequencer](docs/sequencer-window.png) · [Minimum sequencer](docs/sequencer-minimum.png) · [Maximum sequencer](docs/sequencer-large.png) · [Note timing](docs/note-timing.png).

Actual Ableton sessions, native Windows DAW sample/MIDI drops, physical hardware and host Space handling have not been exercised here. Tests cover plugin-side actions and simulated host transport. [Previous validation](docs/VALIDATION-0.14.md).

## Checksum

```text
a86c554f99b5b952f73dcc6469f3b4e1d7045faf45eedae28405e43d2e558f73  RaBubanj-0.15.0-Windows-x64.zip
```

The portable artifact is retained for 30 days and may require GitHub sign-in. Rebuild: **Actions → Windows VST3 → Run workflow**.
