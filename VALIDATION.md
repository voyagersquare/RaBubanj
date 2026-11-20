# RaBubanj 0.12 validation

Verified 20 November 2026. Tested source: `a2aa5e18dc80121f37b5401bf2ed0a024d3e304c`.

[Native Windows run](https://github.com/voyagersquare/RaBubanj/actions/runs/44801723658) · [Windows x64 ZIP](https://github.com/voyagersquare/RaBubanj/actions/runs/44801723658/artifacts/14660988968) · [pluginval logs](https://github.com/voyagersquare/RaBubanj/actions/runs/44801723658/artifacts/14661004023)

## Build and package

- MSVC Release x64 VST3 and standalone: passed. All **nine** CTest suites passed with zero failures.
- pluginval 1.0.4, strictness **5: SUCCESS**, explicitly awaited before packaging. The optional separate Steinberg validator is not configured.
- Downloaded ZIP integrity and SHA256 match GitHub's artifact digest. Both PE binaries are AMD64; the VST3 manifest is 0.12.0. Bundled project sources match the tested commit after Windows newline normalization. Corresponding JUCE source and licenses are included.
- Portable archive: 66,535,110 bytes, without an installer. The original JPEG is verified inside the bundled source and both binaries.
- The same nine suites passed locally in Linux Release. Actual JUCE mixer/sequencer previews were exported and inspected. Mixer and sequencer preview runs used the final reference-control renderer and the real metal look-and-feel.

## Original wood asset

- The user-supplied RAR contains `natural-light-brown-wood-grain-texture.jpg`; its unchanged bytes are committed as `Assets/wood-reference.jpg`.
- JPEG: **7680 × 4302**, 9,501,641 bytes. SHA256: `5f664206eec18dae81a2e6d4ca04dc0be0967afd1d981d44fdf7c87650f7cca5`.
- CMake generates a binary resource containing those source bytes. The image is decoded/cached on the GUI thread, shared across editor instances, and needs no external image path at runtime. Image loading does not occur in the audio callback.
- Housing, channel sections, EQ frames, Reverb/FX panels and sequencer enclosure show the original natural grain. Uniform scaling/cropping fits the panels; relief lighting is separate. No replacement texture or recoloured source image is used.

## Reference-shaped controls, reduced rotary size and clearer legends

- Rotary controls model the supplied reference: tall metal cylinders with broad vertical flutes, black bases, scalloped silver lips and coloured matte top discs. Thick bars reach the rim; DAMPING uses a black bar on its off-white cap, while the other five caps use white bars. Colour mapping is SIZE teal, WIDTH burgundy, PITCH blue, DRIVE red and COMP purple.
- The complete rotary artwork is uniformly scaled to **70%** of the approved reference-model preview dimensions, keeping the cylinder proportions. Slider bounds, parameter ranges, automation and mouse interaction are retained.
- Command labels increase from 12 to **16 px bold**, on dark contrast plates with light borders. Reverb/PAD section captions increase from 14 to **17 px bold**.
- Faders follow the console photo with contoured saddles, raised wings, dark end cheeks, brushed aluminium bridges and centred white index lines. SEND uses a smaller cap; PAN/TIMING use the same profile rotated for horizontal travel. Increased travel padding keeps the raised caps inside the control at the extremes.
- All pushbuttons, dropdown housings, PAD labels, sequencer keys, EQ nodes and screws use the metal material. Metal rendering is independent of the wood resource; no wood remains on control faces.
- White outlined legends remain readable over wood and metal. PLAY lights yellow, close controls remain red, and four sequence strengths, selected PADs, playhead and early/late note markers retain their semantics.
- Rendered minimum/default/maximum mixer views were exported and visually inspected for label legibility, smaller rotary art and cap clearance. The actual sequencer window was also exported. The standalone sequencer/timing preview fixture now inherits the editor look-and-feel and resets that reference before destruction.
- [Mixer](docs/preview.png) · [Minimum layout](docs/controls-minimum.png) · [Maximum layout](docs/controls-large.png) · [Sequencer window](docs/sequencer-window.png) · [Note timing](docs/note-timing.png) · [EQ menu](docs/eq-menu.png).

## Functional regression checks

Nine suites cover engine/pitch, EQ, legacy sources, processor/MIDI/state, mixer/editor, sample import/recall, channel colour/spectrum, sequencer timing and sequencer/MIDI/editor integration. No DSP, MIDI map, parameter identity, state schema, sample loading or sequencer scheduling changes are introduced in this release.

Existing coverage includes per-pad audition and selection, yellow PLAY feedback, EQ context actions, min/default/max editor/window bounds, explicit X and STEP SEQ toggling, waveform capture/isolation, four-pattern recall/migration, left-drag painting, four click-selected velocities, right-drag erase, per-note timing, MIDI file contents, host transport/loops and Space forwarding. Earlier timing/sanitizer validation is archived in [0.10 validation](docs/VALIDATION-0.10.md). [Previous material validation](docs/VALIDATION-0.11.md).

Actual Ableton sessions, native Windows sample/MIDI drops into a DAW, physical MIDI/audio hardware and native host Space handling have not been exercised here. Tests cover the plugin-side functionality and simulated host transport. Standalone audition/external MIDI remains available, but standalone does not supply DAW transport for the sequencer.

## Checksum

```text
caa498d3f7d6ba886c4a0ac0146c35cf2e5c519b2a10a58db7ee59861e22adde  RaBubanj-0.12.0-Windows-x64.zip
```

Artifacts are retained for 30 days and may require GitHub sign-in. Rebuild: **Actions → Windows VST3 → Run workflow**.
