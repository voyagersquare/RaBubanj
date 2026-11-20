# RaBubanj 0.11 validation

Verified 20 November 2026. Tested source: `7613975013e09c066fa924c67a71f691ed57253c`.

[Native Windows run](https://github.com/voyagersquare/RaBubanj/actions/runs/44797921119) · [Windows x64 ZIP](https://github.com/voyagersquare/RaBubanj/actions/runs/44797921119/artifacts/14660885460) · [pluginval logs](https://github.com/voyagersquare/RaBubanj/actions/runs/44797921119/artifacts/14661035248)

## Build and package

- MSVC Release x64 VST3 and standalone: passed. All **nine** CTest suites passed with zero failures.
- pluginval 1.0.4, strictness **5: SUCCESS**, explicitly awaited before packaging. The optional separate Steinberg validator is not configured.
- Downloaded ZIP integrity and SHA256 match GitHub's artifact digest. Both PE binaries are AMD64; the VST3 manifest is 0.11.0. Bundled project sources match the tested commit after Windows newline normalization. Corresponding JUCE source and licenses are included.
- Portable archive: 62,726,083 bytes, without an installer. The original JPEG is verified inside the bundled source and both binaries.
- The same nine suites passed locally in Linux Release. Actual JUCE mixer/sequencer previews were exported and inspected. A targeted sequencer preview run followed the final preview-fixture change to inherit the real metal look-and-feel.

## Original wood asset

- The user-supplied RAR contains `natural-light-brown-wood-grain-texture.jpg`; its unchanged bytes are committed as `Assets/wood-reference.jpg`.
- JPEG: **7680 × 4302**, 9,501,641 bytes. SHA256: `5f664206eec18dae81a2e6d4ca04dc0be0967afd1d981d44fdf7c87650f7cca5`.
- CMake generates a binary resource containing those source bytes. The image is decoded/cached on the GUI thread, shared across editor instances, and needs no external image path at runtime. Image loading does not occur in the audio callback.
- Housing, channel sections, EQ frames, Reverb/FX panels and sequencer enclosure show the original natural grain. Uniform scaling/cropping fits the panels; relief lighting is separate. No replacement texture or recoloured source image is used.

## Raised metal controls

- Tall rotary controls use metal top faces, concentric machining marks, knurled cylindrical sides and deep shadows. Fader caps and PAN/TIMING handles have greater thickness and extended travel padding.
- All pushbuttons, dropdown housings, PAD labels, sequencer keys, EQ nodes and screws use the metal material. Metal rendering is independent of the wood resource; no wood remains on control faces.
- White outlined legends remain readable over wood and metal. PLAY lights yellow, close controls remain red, and four sequence strengths, selected PADs, playhead and early/late note markers retain their semantics.
- Rendered minimum/default/maximum mixer views were exported. Minimum and default layouts plus the actual sequencer window were visually inspected. The standalone sequencer/timing preview fixture now inherits the editor look-and-feel and resets that reference before destruction.
- [Mixer](docs/preview.png) · [Sequencer window](docs/sequencer-window.png) · [Note timing](docs/note-timing.png) · [EQ menu](docs/eq-menu.png).

## Functional regression checks

Nine suites cover engine/pitch, EQ, legacy sources, processor/MIDI/state, mixer/editor, sample import/recall, channel colour/spectrum, sequencer timing and sequencer/MIDI/editor integration. No DSP, MIDI map, parameter identity, state schema, sample loading or sequencer scheduling changes are introduced in this release.

Existing coverage includes per-pad audition and selection, yellow PLAY feedback, EQ context actions, min/default/max editor/window bounds, explicit X and STEP SEQ toggling, waveform capture/isolation, four-pattern recall/migration, left-drag painting, four click-selected velocities, right-drag erase, per-note timing, MIDI file contents, host transport/loops and Space forwarding. Earlier timing/sanitizer validation is archived in [0.10 validation](docs/VALIDATION-0.10.md).

Actual Ableton sessions, native Windows sample/MIDI drops into a DAW, physical MIDI/audio hardware and native host Space handling have not been exercised here. Tests cover the plugin-side functionality and simulated host transport. Standalone audition/external MIDI remains available, but standalone does not supply DAW transport for the sequencer.

## Checksum

```text
e0a9d04c62b8b221e1e4f14d3d005c73d0738b72b74e49ad50753dca972283e9  RaBubanj-0.11.0-Windows-x64.zip
```

Artifacts are retained for 30 days and may require GitHub sign-in. Rebuild: **Actions → Windows VST3 → Run workflow**.
