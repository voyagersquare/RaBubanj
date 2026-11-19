# Development roadmap

## 0.1 — playable foundation

Windows x64 VST3 + standalone, original generated starter kit, GM MIDI, layered round robin playback, channel mixer, room, state recall and portable build pipeline.

## 0.2 — compact mixer and parametric EQ

Eight compact drum strips with post-fader sends, an independent wet Reverb return, master strip, channel/return/global six-band parametric EQs, interactive response graphs, mute/solo, metering and backwards project-state migration.

## 0.2.1 — worn hardware interface

Corroded and distressed dark-blue metal, raised panels/buttons and rotary Reverb controls in one half of the split side panel; the other half is reserved. Header and Reverb routing captions removed.

## 0.3 — factory sounds and direct mouse workflow

Ten original factory sounds per drum channel (80 total), source selection recalled per channel, name-button audition, optional dry WAV previews and graph-only EQ editing with the duplicate selectors/buttons removed.

## 0.4 — expanded sounds, chromatic input and per-channel sample import

Twenty sounds per drum (160 total), refined snares/hats/cymbals, preset arrows, zero default sends, chromatic C1–G1 with optional GM mapping, DAW/local keyboard modes and asynchronous mono/stereo file drops with embedded recall.

## 0.5 — per-channel shaping and live metering

Channel-name level meters without audition, yellow bottom PLAY pads replacing factory selection, stereo EQ spectrum displays, and independent Pitch/Drive/opto-style one-knob compression for each drum.

## 0.6 — ochre console and node menus

Solid ochre finish, white command legends, stronger EQ bevels and console-inspired physical controls. Node context menus replace EQ type dropdowns and expose shape/gain commands with a dark themed palette.

## 0.7 — pads and host-synced step sequencer

PAD 1–8 names and larger bold legends, 32×8 sequencer child window, three velocity brushes, write/erase gestures, host PPQ/tempo/transport and loop following, embedded pattern recall, and MIDI pattern file drag-and-drop.

## 0.8 — four patterns and click velocity cycling

Four independent patterns selected from a dropdown, four velocity levels cycled with left-click, right-click/right-drag erasing and PAD labels moved to the left. Old single-pattern projects retain their notes and velocities in Pattern 1.

## Next: acoustic sample engine

- Load a documented kit manifest with licensed WAV samples, velocity zones, independent round-robin pools, tuning and per-articulation envelopes.
- Expand the existing per-channel background loader to load complete multi-sample kits.
- Add loading progress, missing-file errors, library path management and user kit recall.
- Retain explicit factory selection and missing-sample handling.

## Then: detailed kit and mixing

- Independent kick/snare articulations, hi-hat openness via CC4 and e-drum mappings.
- Close, overhead and room microphone buses with phase-safe routing and multi-output support.
- Per-channel compression, presets and a realistic multi-sampled acoustic kit.

## Then: grooves and release polish

- Licensed MIDI groove library and additional pattern editing tools.
- Audition representative DAWs on real Windows audio hardware, including device changes and project reloads.
- Signing and a versioned release download; an installer is deliberately outside the initial scope.

Reference products inform the workflow only. Artwork, sample recordings and implementation remain original or appropriately licensed.

## 0.9 — sequencer controls and live waveform

Explicit close/toggle controls with a red X, PAD-label audition and selection, brighter beat groups and a larger panel with a dedicated live selected-pad stereo waveform feed.


## 0.10 — paint notes and per-note microtiming

Left-drag writing retains click velocity cycling and right-drag erasing. Stationary long press opens a per-note early/late timing slider, with atomic velocity/timing state, host-loop-aware playback, project recall and shifted MIDI export.
