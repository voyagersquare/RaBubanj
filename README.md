# RaBubanj 0.7.0

Windows x64 VST3 and standalone drum sample instrument by Voyagersquare, with a solid ochre console-style mixer, per-channel processing and a shared stereo Reverb. Portable ZIP, without an installer.

**[Preuzmi RaBubanj 0.7 za Windows x64 — ZIP bez instalera](https://github.com/voyagersquare/RaBubanj/actions/runs/44511975184/artifacts/14524047762)**

Windows build, svih devet testnih skupova i pluginval strictness 5 prolaze.

[Windows instructions in Croatian](docs/WINDOWS-HR.md) · [Validation](VALIDATION.md)

![RaBubanj 0.7 PAD mixer and spectrum displays](docs/preview.png)

## New in 0.7

- The eight sound strips are **PAD 1–PAD 8**, including channel EQ and the selected Pitch/Drive/Comp panel. Existing source assignments, sample filenames and parameter IDs are retained. Pad names and control legends use larger bold type; Reverb/master return labels span two lines for readability.
- **STEP SEQ** opens a dedicated floating child window inside the plugin. It has **32 sixteenth-note steps × 8 rows**, with PAD labels on the right. One pattern is two bars in 4/4.
- Choose **SLABA**, **SREDNJA** or **JAKA** (MIDI velocities **48 / 88 / 127**), then **right-click** to write steps. **Left-click** erases; hold and drag left to erase multiple cells. Fast drags also erase intervening cells.
- **SEQ ON** follows DAW tempo, playback position and Play/Stop. The playhead is derived from host PPQ on each block, including seeks and host loop boundaries. **Space stays with the DAW transport** in both mixer and sequencer views. There is no competing local clock or local Play button.
- Drag **DRAG MIDI** onto a DAW MIDI track to create a two-bar MIDI clip. The file preserves step positions, the three velocities and the active chromatic/GM mapping. Switch **SEQ OFF** when playing that exported clip to avoid playing both the internal pattern and the clip.
- Pattern, brush strength and sequencer enable are saved with DAW state. Older projects restore a blank pattern. New instances also start blank.

![Sequencer demonstration pattern](docs/sequencer.png)

The sequencer uses standard host transport information and requires an active DAW audio engine with the instrument track monitored. Standalone playback still supports pad audition and external MIDI, but does not supply a DAW transport for the internal sequencer. One-shot and Reverb tails can decay after Stop; no new sequencer hits are generated while stopped. MIDI export is file drag-and-drop, not live MIDI output routing. Exports use one type-0 track, 960 PPQ, channel 10, a 1/32-note gate and an explicit 8-beat end marker. Export files remain in the OS temporary `RaBubanj-MIDI` folder so the host can read them after the drop.

## Console and EQ workflow

- Solid ochre enamel replaces the scratched/rusted texture. Command legends are white; EQ gain/spectrum scales and the response curve use warm gold tones.
- More pronounced bevels and shadows around EQ panels. Rotary controls have ribbed light-plastic bodies, coloured caps and position marks inspired by the supplied SSL console reference. Faders and switches have raised console-style hardware details. Artwork is drawn by the project; no manufacturer assets or logos are bundled.
- **Right-click an EQ node** for a dark ochre-themed context menu with **Curve shape** (six types with curve icons), **Gain (dB)** presets, **Invert gain** and **Band enabled**. The previous filter-type dropdown is removed from both EQs. Gain choices/inversion are disabled for cut and notch filters.
- [Context-menu styling preview](docs/eq-menu.png).
- **Alt-click** a node still toggles it directly. Drag, wheel, double-click, bypass and reset retain their existing actions.

## Controls

- **Channel names show live level meters.** Clicking PAD 1, PAD 2 or another channel selects its EQ and processing controls, without playing a sound. Level meters also remain beside the faders.
- The **PLAY** button at the bottom of each drum strip auditions that channel's current sample once. It lights **yellow** while pressed, with a short visible pulse for quick clicks. Factory menus and arrows have been removed.
- Imported filenames stay below each channel name. Drop a mono/stereo WAV, AIFF, FLAC or Ogg file onto a drum strip to replace just that sound. The **x** button removes the imported source. Samples load in the background and their original bytes are embedded in saved DAW projects.
- Both EQ graphs show a **live spectrum** behind the editable response curve. The channel display follows the selected channel after its EQ, before the fader; the master display measures the final stereo output. The left scale shows EQ gain (±24 dB); the right scale shows spectrum level (−90 to 0 dBFS). Analysis uses both stereo channels, including opposite-phase audio.
- The panel beside Reverb contains **PITCH, DRIVE and COMP for the selected drum**. Each of the eight drums saves independent values and supports automation. Select another drum to edit its settings. These drum controls are disabled when the Reverb return is selected.
- **PITCH:** −24 to +24 semitones, default 0. It changes source playback speed and duration. Existing voices glide smoothly to automated pitch changes; new voices start at the requested pitch.
- **DRIVE:** 0–100% soft saturation with antiderivative antialiasing. Default 0% is neutral.
- **COMP:** 0–100% compression strength. Fixed −18 dBFS threshold, 3:1 ratio, 6 dB soft knee, 10 ms attack, and programme-dependent 150–350 ms opto-style release. Stereo linked, with no automatic makeup gain. This is an opto-inspired response, not a hardware circuit model.
- Double-click the three processing knobs to reset them to neutral. Rotary Reverb controls, pan, faders, mute/solo and graph EQ editing remain.

Each of the eight channels retains one built-in starter voice when no user sample is loaded. Old factory source IDs and source recipes remain only for backwards project compatibility; no factory selection controls or factory preview download are offered by the current interface. Old projects retain their selected source until it is replaced by a dropped sample.

## Mixer and EQ

Every drum has level, pan, mute/solo and a post-fader **SEND**, defaulting to **0%**. The Reverb return has its own EQ, level, mute/solo and rotary Size/Damping/Width controls. Master EQ stays visible on the right. The VST3 exposes one stereo output.

Six parametric bands per EQ offer Bell, Low shelf, High shelf, Low cut, High cut and Notch. Select a graph node, drag it for frequency/gain, use the wheel for Q, double-click to reset gain, right-click for shape/gain commands, or Alt-click to enable/disable. Bypass and Reset affect only the chosen EQ. The analyser is visual only and adds no audio latency. It performs FFTs on the GUI thread; audio writes to bounded lock-free buffers only for the visible channel and master.

```text
Sample → Pitch → Drive → Compressor → channel EQ → level/pan → dry mix ─┐
                                                      └→ Send → Reverb │
                                           → return EQ → return level ├→ master EQ → master → stereo out
                                                                      ┘
```

A drum mute also mutes its send. Reverb tails already generated decay naturally. Reverb Solo auditions the wet return only. The master retains its existing gentle saturation.

## MIDI and keyboard

| Pad | Ableton note | MIDI number | PLUGIN audition key |
| --- | --- | ---: | --- |
| PAD 1 | C1 | 36 | A |
| PAD 2 | C#1 | 37 | W |
| PAD 3 | D1 | 38 | S |
| PAD 4 | D#1 | 39 | E |
| PAD 5 | E1 | 40 | D |
| PAD 6 | F1 | 41 | F |
| PAD 7 | F#1 | 42 | T |
| PAD 8 | G1 | 43 | G |

All MIDI channels are accepted. Octave labels vary by host; note numbers are authoritative. **MIDI: GM** selects the legacy mapping for older clips. For computer keyboard recording, enable the DAW's computer MIDI keyboard, arm/monitor the track, choose the C1 octave and keep **KEYS: DAW**. **KEYS: PLUGIN** auditions locally with the window focused; it does not record MIDI in the host. Key repeat is suppressed; shortcuts, text editing and keys during host recording pass through.

## Samples and recall

Drop a single local audio file or file URI from the DAW browser or Explorer onto a drum strip. Proprietary clips that do not expose an audio file must first be exported/consolidated. Limits: mono/stereo, 8–384 kHz, 30 seconds, 32 MB encoded and decoded audio. Stereo is preserved; pan acts as balance on stereo sources. At −24 semitones a 30-second sample lasts up to 120 seconds.

Failed new loads keep the previous sound. Missing/corrupt restored audio stays silent with an error until replaced or cleared. Hover the source row for details; **x** also cancels a pending load. Wait for the filename after opening a project. Embedded audio increases project size, but project recall does not require the original file path. Changing source retains already-playing tails.

Existing parameter IDs and plugin identity are retained. Older projects receive neutral Pitch/Drive/Comp, preserve their mixer/EQs/imported audio, and keep the 0.4 chromatic MIDI and send migration rules. Prior validation and factory catalogues remain archived in `docs`.

## Build on Windows

Use Visual Studio 2022 with **Desktop development with C++**, Windows SDK, CMake 3.22+ and Git. Internet access is needed for the first JUCE fetch; its revision is pinned in CMake.

```powershell
cmake -S . -B build -A x64 -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

Outputs:

```text
build/RaBubanj_artefacts/Release/VST3/RaBubanj.vst3/
build/RaBubanj_artefacts/Release/Standalone/RaBubanj.exe
```

GitHub Actions builds both targets, runs the DSP tests, and validates the VST3 with pluginval at strictness 5 before uploading the portable package. Binaries are unsigned. See [Croatian Windows instructions](docs/WINDOWS-HR.md) and [development roadmap](docs/ROADMAP.md).

## Tests

A full build runs nine suites: drum engine/pitch, parametric EQ, legacy source regression, processor MIDI/state, mixer/editor, sample import/recall, channel colour/spectrum, sequencer timing and sequencer/MIDI/editor integration. The colour/spectrum suite checks neutral bypass, compressor calibration and stereo linking, saturation harmonics, exact pitch-rate conversion, FFT frequency/dBFS calibration, anti-phase stereo, decay, concurrent buffers, real processor taps and state recall. UI tests cover name meters without triggering, yellow PLAY buttons, curve/gain menu actions and cancellation, per-channel knob binding and minimum/default/maximum bounds.

`mixer_tests <absolute-directory>` exports actual JUCE editor snapshots. `sequencer_integration_tests <absolute-directory>` exports sequencer and window previews using a demonstration pattern and simulated host transport. Sequencer coverage includes fractional-BPM/block-boundary accuracy, loops inside a block, start/stop, seeks, pre-roll, same-offset panic, MIDI file parsing, mouse painting/erasing, state recall and Space forwarding. Native Windows CI also runs pluginval strictness 5 before packaging.

## License

The project is GPL-3.0-only; see [LICENSE](LICENSE) and [THIRD_PARTY.md](THIRD_PARTY.md). No proprietary samples, graphics, product names or source code from other drum instruments are bundled. If a closed-source commercial release is desired later, review JUCE commercial licensing and the rights to all contributed code and sample content before changing distribution terms.
