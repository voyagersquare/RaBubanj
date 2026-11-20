# RaBubanj 0.13 validation

Verified 20 November 2026. Tested source: `ac11319d55fcecf400087df924df527d84218859`.

[Windows run](https://github.com/voyagersquare/RaBubanj/actions/runs/44805979313) · [Windows x64 ZIP](https://github.com/voyagersquare/RaBubanj/actions/runs/44805979313/artifacts/14662543878) · [pluginval logs](https://github.com/voyagersquare/RaBubanj/actions/runs/44805979313/artifacts/14662827912)

## Build and package

- MSVC Release x64 VST3 and standalone: passed. All **nine** CTest suites passed, zero failures.
- pluginval 1.0.4 strictness **5: SUCCESS**, awaited before packaging. The optional separate Steinberg validator is not configured.
- Downloaded ZIP integrity and SHA256 match the GitHub digest. Both PE binaries are AMD64; the VST3 manifest is 0.13.0. All bundled project sources match the tested commit after Windows line-ending normalization. Corresponding JUCE source and licenses are included.
- Portable archive: 66,217,340 bytes, no installer. The source JPEG is verified inside the bundled source and both binaries.
- All nine suites also passed in Linux Release. Mixer and sequencer preview runs passed with the final renderer; minimum/default/maximum mixer views and the sequencer window were visually inspected.

## Flat captions and enlarged rotaries

- Raised metal text buttons become flat white captions on dark legend plates, using the same fill/border treatment as the REVERB/PAD control headings. Header commands, EQ BYPASS/RESET, channel names, M/S and PLAY retain the same component bounds and click callbacks.
- The sequencer uses the same flat style for SEQ ON/OFF, DRAG MIDI, pattern selection, PAD labels and close controls. State borders/indicators, channel-name meters, yellow PLAY feedback and red close feedback remain.
- Rotary artwork is uniformly **15% larger than 0.12**: 0.7 × 1.15 = **0.805** of the approved reference-model size. Cylinder proportions, six cap colours and thick pointer bars remain. Slider bounds, mouse interaction, parameter ranges and automation do not change.
- Console-style saddle faders retain their metal profile and dark side cheeks. The wood image and raised section/EQ frames remain.

## Flat step cells

- Cells use opaque flat fills without metal texture, bevels, raised sides or cast shadows. Columns **1–4 are dark**, **5–8 light**, then alternate four/four through column 32.
- Four velocity bar heights and colours remain. Gold playhead outlines and cyan/orange timing markers retain their meaning. Selected PAD captions remain brighter.
- Editing, long-press microtiming, four pattern banks, DAW sync and selected-pattern MIDI export are unchanged.

## Original asset and functional regression

`Assets/wood-reference.jpg` remains the unmodified 7680×4302 JPEG from the user's RAR: 9,501,641 bytes, SHA256 `5f664206eec18dae81a2e6d4ca04dc0be0967afd1d981d44fdf7c87650f7cca5`. It is cached on the GUI thread and embedded in the binaries; no external texture file is needed.

The existing nine suites cover engine/pitch, EQ, legacy sources, processor/MIDI/state, mixer/editor, sample import/recall, channel colour/spectrum, sequencer timing and sequencer/MIDI/editor integration. GUI coverage includes channel selection without audition, PAD audition, yellow PLAY feedback, EQ menu actions, explicit X/STEP SEQ toggling, min/default/max window bounds, Space forwarding, waveform isolation, four-pattern recall/migration, click velocity cycling, left painting/right erasing, signed timing and MIDI contents. No DSP, parameter identity, state schema, sample import or sequencing algorithm changes are introduced.

[Default mixer](docs/preview.png) · [Minimum](docs/controls-minimum.png) · [Maximum](docs/controls-large.png) · [Sequencer](docs/sequencer-window.png) · [Note timing](docs/note-timing.png).

Actual Ableton sessions, native Windows DAW sample/MIDI drops, physical hardware and native host Space handling have not been exercised here. Tests cover plugin-side actions and simulated host transport. [Previous validation](docs/VALIDATION-0.12.md).

## Checksum

```text
6a78911cdf09c742104729f220d1dab84c5ebdeb5ea196f39a5b5641c56cc5a8  RaBubanj-0.13.0-Windows-x64.zip
```

Artifacts are retained for 30 days and may require GitHub sign-in. Rebuild: **Actions → Windows VST3 → Run workflow**.
