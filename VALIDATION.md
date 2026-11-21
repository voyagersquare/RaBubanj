# RaBubanj 0.16 validation

Native Release build and all nine suites passed. Mixer and sequencer preview runs passed. 64-step and 128-step final-page views were inspected, including 128 steps at minimum/default/maximum editor sizes. Pure sequencer AddressSanitizer/UndefinedBehaviorSanitizer checks passed.

Coverage includes length-dependent host phase, signed timing and loops, exact final-step/wrap audio onset, legacy 32-column state migration, per-bank length/notes/timing recall, retained inactive tails, shortened playback/export, four/eight-bar MIDI, wheel/scrollbar navigation, fixed PAD labels and scrolled editing.

Windows x64 build, all nine suites, pluginval strictness 5 and portable package verification are pending. [Previous validation](docs/VALIDATION-0.15.md).
