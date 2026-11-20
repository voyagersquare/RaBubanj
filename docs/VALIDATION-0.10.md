# RaBubanj 0.10 validation

Verified 19 November 2026. Tested source: `28a89c8ee2b805c34c07af2baad91b28f65f4838`.

[Native Windows run](https://github.com/voyagersquare/RaBubanj/actions/runs/44527504050) · [Windows x64 ZIP](https://github.com/voyagersquare/RaBubanj/actions/runs/44527504050/artifacts/14532861959) · [pluginval logs](https://github.com/voyagersquare/RaBubanj/actions/runs/44527504050/artifacts/14532332114)

## Build and package

- Native MSVC Release x64 VST3 and standalone: passed. All **nine** CTest suites passed, zero failures.
- pluginval 1.0.4 strictness **5: SUCCESS**. The process is explicitly awaited before packaging. The optional separate Steinberg SDK validator was not configured and was skipped.
- Downloaded ZIP integrity and SHA256 match the GitHub artifact digest. Both binaries have AMD64 PE headers; the VST3 manifest is 0.10.0. All bundled project sources match the tested commit after Windows line-ending normalization. Corresponding JUCE source and licenses are included.
- Portable Windows archive: 29,454,022 bytes, no installer.
- The same nine suites pass locally in Linux Release, with mixer/sequencer preview runs after the painting/microtiming changes, followed by a targeted sequencer/editor run after the final tempo-readout refresh. Both the pure sequencer timing suite and project-source sequencer integration pass AddressSanitizer and UndefinedBehaviorSanitizer; the integration run links against the native JUCE library.

## Sequencer timing and audio

- Four independent blank 32×8 patterns are the default. Five cell states represent Off and MIDI velocities 32/48/88/127. Only the selected pattern plays; changing patterns retains the DAW phase. Atomics keep GUI edits safe for the processing thread; processing takes a fixed-size atomic strength/timing snapshot per block.
- Timing tests cover 44.1/48/96 kHz, tempos 61.1/93/127.35/199.99 BPM, and block sizes 37/511/1024. Two cycles produce exactly 64 sixteenth-note boundaries, at the expected nearest sample, without missing or duplicated steps at fractional-tempo block edges.
- Host loops inside a block wrap to their reported PPQ start. Starting between grid boundaries aligns to the next boundary; seeks derive phase from the current host position instead of an accumulated free clock. Negative pre-roll retains modulo phase. Stopped or missing host position emits no sequence notes.
- Processor integration verifies an exact 240-sample start offset, DAW Stop, SEQ OFF, pad-index triggering in GM mode, and MIDI panic cancelling a sequence hit at the same sample offset. Incoming MIDI and pad previews still use the existing voice engine and mixer path.
- Scheduling and event merging use fixed stack storage with no sequencer allocation in the audio callback. Tempo/PPQ/loop information is read on the processing thread; GUI status reads published atomics only.

## Editing, MIDI export and recall

- Left-click cycles 127 → 88 → 48 → 32 → 127; it never erases. Right-click erases. A single fast right-drag from step 1 to 32 erases every intervening cell, with no gaps. Left-drag paints strongest notes into empty cells across the whole stroke, including intervening cells in fast drags; existing velocities and timing survive repainting and revisiting. A right-drag stays bound to the pattern selected at gesture start, even if selection changes during the gesture.
- MIDI files are written and parsed back in the integration suite: type 0, one track, 960 PPQ, channel 10, expected note numbers/timestamps/all four velocities, tempo metadata and an explicit 7680-tick (8-beat) end. Chromatic and legacy GM mappings are both covered. Export includes only the selected pattern; its index is captured when a native drag starts.
- Native file drag uses JUCE's external file-drop API from the GUI mouse-drag path. Unique export files remain in the OS temp folder for deferred host reading. The drag callback uses a safe component pointer. Live MIDI output routing is not enabled.
- All four patterns, per-note signed timing, selected-pattern parameter and sequencer enable recall in DAW state (schema 8). Schema-7 banks from 0.8/0.9 retain their cells and receive zero offsets. A schema-6 single pattern migrates into Pattern 1, preserving its actual velocities 48/88/127; Patterns 2–4 remain blank. Older states without a sequencer restore all patterns blank. Previous samples/mixer/EQ/processing settings remain. Independent playback and recall are checked for every bank.

## UI and transport key handling

- PAD 1–8 labels appear on sound strips, channel EQ and selected-pad Pitch/Drive/Comp. Display names change while parameter IDs and source slots remain compatible. Control legends and pad names are larger and bold; Reverb/master captions use two lines.
- STEP SEQ toggles a dedicated floating child panel inside the VST3 editor. Its explicit red X button hides it through a direct callback. The panel avoids DocumentWindow title-button recreation during look-and-feel changes. Hiding leaves pattern playback governed by SEQ ON/OFF. The grid has 32 columns and eight rows, with PAD labels on the left, a four-pattern dropdown, SEQ ON/OFF, host tempo/play status and DRAG MIDI. The three velocity buttons are removed; cell heights and brightness distinguish four strengths.
- The child-window approach keeps focus in the host-owned plugin view. Grid, pattern selector, export and window controls do not take keyboard focus; both the window and editor leave Space unhandled for the DAW. The plugin's VST3 keyboard-focus setting remains unchanged. Processor playback follows the host's resulting transport state.
- Tests verify window creation, direct X callback and hit-target accessibility, repeated STEP SEQ open/close, close after look-and-feel changes, immediate capture shutdown on close, bounds after main-editor resizing at 1120×740, 1280×820 and 1800×1160, Space forwarding, mouse editing and project recall.
- Actual JUCE previews were rendered and visually inspected. [Mixer](preview.png) · [Sequencer](sequencer.png) · [Child window](sequencer-window.png). Sequencer previews use a demonstration pattern and simulated host transport; new instances start empty.

Actual Ableton sessions, native Windows MIDI-file drops into a DAW, physical MIDI/audio hardware and native host Space handling have not been exercised here. Automated tests cover plugin-side key forwarding, host transport scheduling, file contents and the drag source implementation. Host PPQ is preferred; a sample-clock fallback is available when PPQ is missing. Standalone audition/external MIDI remains available, but standalone does not provide the DAW transport required by the sequencer. One-shot/Reverb tails may decay after Stop.

## PAD audition and live waveform

- Left-click each of the eight PAD labels: integration tests verify the matching channel is selected, one audition hit is generated and pattern cells remain unchanged. The selected label is brighter. Four-step light backgrounds have stronger contrast.
- Enlarged 1160×650 child window (width shrinks within the minimum editor), with a waveform panel below the grid. Actual rendered previews show the red X, brighter selection/beat groups and waveform without clipping.
- The live waveform follows the selected sequencer pad after Pitch/Drive/Comp/channel EQ, before fader/pan. Its 2048-point history represents approximately one second of audio. Stereo minima/maxima are reduced into envelopes at approximately 2 kHz; opposite-phase stereo remains visible.
- Real processor audio reaches the display. Switching pads clears history; unselected tails are excluded. Per-generation tags reject queued audio from prior pad selections, including selecting away and back. The view holds its last received audio if the host stops processing.
- Capture uses a dedicated bounded SPSC queue, independent of EQ spectrum consumers. Tests cover stereo peak preservation, time reduction, bounded overflow when GUI consumption pauses, per-pad isolation and disabling capture while hidden. Audio-side storage is fixed; no scope allocation or locking occurs in the callback. The GUI drains/draws at 30 Hz.

## Per-note timing and paint gestures

- A stationary left press of 650 ms opens a dark inline TIMING slider. Short-click velocity cycling is applied on mouse release, so a held existing note preserves its velocity. A held empty cell first creates a strongest hit. Movement of 4 px starts painting and cancels the hold action. Right-button erase/drag retains its previous semantics.
- Timing range −50..+50 percent of one sixteenth step, integer increments. The slider also displays milliseconds at current host BPM and refreshes when tempo changes. Double-click returns to zero. Per-note cyan/orange indicators mark nonzero offsets. The popup closes on pattern selection changes and stays inside the grid.
- Strength and timing are packed into one lock-free cell atomic. Erasing resets timing; cycling strength preserves it. Gesture edits remain bound to their starting bank. Timing is independent for every cell of all four patterns.
- A fixed-capacity 768-node heap schedules individual hits in the audio callback, with no dynamic allocation. The visual playhead continues to show the unshifted sixteenth grid. Negative first-step timing wraps around the two-bar pattern; earlier notes cannot be emitted before DAW playback begins. Host loops wrap shifted boundary notes around the host-loop length, including short and non-grid loops.
- Timing tests compare all 256 shifted cells with an independent event calendar over two cycles at 44.1/48/96 kHz, four fractional tempos and three block sizes. Additional calendars cover short/long/non-grid/negative host loops across three buffer sizes. Exact early/late audio onset at sample 240 is verified through the real processor; existing panic and host-transport tests still pass.
- MIDI export preserves signed shifts at 960 PPQ, wrapping an early step-1 note to the clip end and retaining an exact 7680-tick length. Parsed tests verify a −25% note moves by −60 ticks and an early first note lands at tick 7560. MIDI tick resolution may round fractional-tick positions.
- GUI integration covers 32-cell fast left strokes, preserving existing strengths/timing, stationary long-press detection, signed slider edits, no unintended velocity cycling, bank isolation and popup dismissal when changing banks. [Timing slider preview](note-timing.png).

## Checksum

```text
4d58d2b9c30aa0d12fb032ad4e2413cb3474caec551a26f7b0bb39cd1ec3ce96  RaBubanj-0.10.0-Windows-x64.zip
```

Artifacts are retained for 30 days and may require GitHub sign-in. Rebuild: **Actions → Windows VST3 → Run workflow**. [Previous validation](VALIDATION-0.9.md).
