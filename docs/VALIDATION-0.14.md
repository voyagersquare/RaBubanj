# RaBubanj 0.14 validation

Verified 21 November 2026. Tested source: `d7c8aa4b36ae9a9829cc5b760459597455729578`.

[Windows run](https://github.com/voyagersquare/RaBubanj/actions/runs/44877626388) · [Windows x64 ZIP](https://github.com/voyagersquare/RaBubanj/actions/runs/44877626388/artifacts/14696612610) · [pluginval logs](https://github.com/voyagersquare/RaBubanj/actions/runs/44877626388/artifacts/14697240481)

## Build and package

- MSVC Release x64 VST3 and standalone: passed. All **nine** CTest suites passed, zero failures.
- pluginval 1.0.4 strictness **5: SUCCESS**, completed before packaging.
- Downloaded ZIP integrity and SHA256 match the GitHub digest. Both PE binaries are AMD64; the VST3 manifest is **0.14.0**. All **24** bundled project source/CMake files match the tested commit after Windows line-ending normalization. JUCE source and required licenses are included.
- Portable archive: **66,400,123 bytes**, 12 files, no installer. The original wood JPEG and Great Vibes font are present byte for byte inside the bundled source and both binaries. The portable font license matches `Assets/Fonts/OFL.txt`.
- All nine suites passed in Linux Release for the typography changes. After the final wood-tone change, the native build and mixer/editor and sequencer integration suites passed again. Windows tests cover the final source.
- Minimum/default/maximum mixer previews (1120×740, 1280×820 and 1800×1160) and the sequencer window were visually inspected for readability, layout and script clipping.

## Typography and wood finish

- Larger bold black wood captions cover channel/master EQ titles, the six-band caption, mixer instruction, sample-source labels, PAN/dB/SEND, WET RETURN/STEREO OUT, POST SENDS/OUT 1/2 and the lower-left EQ help line. A subtle pale lower edge aids separation from the grain. Load errors use dark red; dark control/screen captions remain light.
- The crossed-out bottom-right `C1-G1 MIDI / SAMPLE PLAYER` footer is removed. MIDI functions remain.
- The **dark-red RaBubanj wordmark** uses embedded **Great Vibes Regular** under SIL OFL 1.1, as a calligraphic interpretation of the supplied example. Cached glyph outlines are fitted by their actual ink bounds; no host-installed font is required. The unmodified font and license are bundled. This is a similar freely licensed font, rather than the proprietary Aventalia font shown in the reference.
- The original JPEG is rendered with a **20% translucent red-brown finish**, slightly darkening the displayed wood and making it warmer and more reddish. The source JPEG remains unchanged; grain, raised frames, metal controls and flat step cells remain.
- Knob/fader dimensions, control callbacks, parameter identities, DSP, state schema, imported sample names, four-pattern sequencing, DAW synchronization and MIDI export are unchanged.

## Original assets and regression

`Assets/wood-reference.jpg`: unmodified user-supplied 7680×4302 JPEG, 9,501,641 bytes, SHA256 `5f664206eec18dae81a2e6d4ca04dc0be0967afd1d981d44fdf7c87650f7cca5`.

`Assets/Fonts/GreatVibes-Regular.ttf`: 457,588 bytes, SHA256 `8d509802186f1b51572531ecf313e8098f9a5bfdfaca93f0c9b34467f9982d15`. Provenance and license are in [Assets/Fonts](Assets/Fonts/README.md).

Existing suites cover the engine/pitch, EQ, legacy sources, processor/MIDI/state, mixer/editor, sample import/recall, channel spectrum, sequencer timing and sequencer/MIDI/editor integration. GUI coverage includes channel selection and audition, yellow PLAY feedback, EQ menus, X/STEP SEQ toggling, window bounds, Space forwarding, live waveform isolation, pattern recall/migration, velocity cycling, left painting/right erasing, signed timing and MIDI contents.

[Default mixer](docs/preview.png) · [Minimum](docs/controls-minimum.png) · [Maximum](docs/controls-large.png) · [Sequencer](docs/sequencer-window.png) · [Note timing](docs/note-timing.png).

Actual Ableton sessions, native Windows DAW sample/MIDI drops, physical hardware and host Space handling have not been exercised here. Tests cover plugin-side actions and simulated host transport. [Previous validation](docs/VALIDATION-0.13.md).

## Checksum

```text
60267b445be78e8ac1eed4cc4d06e4d67b8766ff9c448b9891a9ecbad774564b  RaBubanj-0.14.0-Windows-x64.zip
```

The portable artifact is retained for 30 days and may require GitHub sign-in. Rebuild: **Actions → Windows VST3 → Run workflow**.
