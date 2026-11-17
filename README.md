# RaBubanj

**[Preuzmi RaBubanj 0.1 za Windows x64 — ZIP bez instalera](https://github.com/voyagersquare/RaBubanj/actions/runs/44145272657/artifacts/14356307396)**

Raspakiraj ZIP i kopiraj cijelu mapu `RaBubanj.vst3` u `C:\Program Files\Common Files\VST3\`, zatim ponovno skeniraj plugine u DAW-u. `RaBubanj.exe` je samostalna aplikacija za sviranje i preslušavanje. [Upute na hrvatskom](docs/WINDOWS-HR.md) · [Rezultati provjera](VALIDATION.md)

![RaBubanj 0.1 interface](docs/preview.png)

Windows x64 VST3 drum instrument by **Voyagersquare**. Version 0.1.0 is a playable foundation for a future acoustic drum sampler, inspired by the drum-kit and mixer workflow of established drum instruments. It has its own interface and sounds.

**No installer.** Download the portable artifact from a successful **Actions → Windows VST3** run. Extract the ZIP, copy the complete `RaBubanj.vst3` folder to `C:\Program Files\Common Files\VST3\`, and rescan instruments in your DAW. The included standalone EXE can be launched separately to configure audio/MIDI and audition the pads. A DAW is required for recording and sequencing.

## Included in 0.1

- Eight drums: kick, snare, closed/open hi-hat, low/high tom, crash, ride.
- Original procedural starter samples, generated into memory when audio starts: three velocity layers and three round-robin variations per instrument.
- Sample-accurate MIDI note-on playback, velocity response, 64 shared voices, oldest-voice stealing, and a short hi-hat choke fade.
- Click-to-audition pads with hit indicators; channel level, pan and mute; master level, stereo output meter and room reverb.
- Automatable mixer parameters and DAW project state recall. MIDI channels 1–16 are accepted. Note-offs do not stop one-shots; CC120/123 resets all voices and the reverb tail.
- Stereo output and a portable standalone application. Generated samples are embedded in the engine; there are no sample downloads or external paths to configure.
- Static MSVC runtime in the Windows build; no separate Visual C++ runtime installation is intended.

**Scope:** this is an early playable prototype. The generated starter kit does not provide the acoustic realism or content library of Superior Drummer or Addictive Drums. There is currently no WAV import, multi-microphone sample library, user kit browser, multi-output routing, groove browser, internal sequencer, or e-drum articulation editor. MIDI sequencing comes from the host. The master uses gentle saturation to bound stacked hits; it is not a transparent mastering limiter. Mute stops a channel's new output; existing room tails decay naturally.

## MIDI map

| Instrument | Main note | Accepted aliases |
| --- | ---: | --- |
| Kick | 36 | 35 |
| Snare | 38 | 37, 40 |
| Closed hi-hat | 42 | 44 |
| Open hi-hat | 46 | — |
| Low tom | 45 | 41, 43 |
| High tom | 48 | 47, 50 |
| Crash | 49 | 52, 55, 57 |
| Ride | 51 | 53, 59 |

Aliases share the corresponding sound; they are not independent articulations. MIDI note numbers are authoritative because octave labels differ between hosts.

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

## Test only the engine

No JUCE, audio device or DAW required:

```sh
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -ISource tests/DrumEngineTests.cpp -o drum_engine_tests
./drum_engine_tests
```

Alternatively configure CMake with `-DRABUBANJ_BUILD_PLUGIN=OFF`. Tests cover silence, GM mappings, audible finite output, voice tails, block continuity, velocity dynamics, pan, mute, hi-hat choke, voice stealing, reset and round robin at 44.1/48/96 kHz. Host MIDI scheduling, automation and state recall are additionally exercised by pluginval.

## License

The project is GPL-3.0-only; see [LICENSE](LICENSE) and [THIRD_PARTY.md](THIRD_PARTY.md). No proprietary samples, graphics, product names or source code from other drum instruments are bundled. If a closed-source commercial release is desired later, review JUCE commercial licensing and the rights to all contributed code and sample content before changing distribution terms.
