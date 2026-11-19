> Historical 0.4 catalogue. Factory selection was removed from the 0.5 interface; source recipes remain for old-project compatibility.

# RaBubanj 0.4 — factory sounds and references

Each of the eight drum channels has twenty original factory sounds. Use the left/right arrows at the bottom of a channel, then click its channel-name button to audition. Choosing a preset alone does not play a hit. The name click also selects the channel EQ. Other channel clicks select its EQ without auditioning.

Preset choices affect only the source instrument; they do not overwrite channel EQ, level, pan, sends or Reverb settings. Choices are automatable and saved with the DAW project. Existing sounding notes keep their original source until they end; the next note uses the new choice.

## The 160 presets

| Channel | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Kick | Thump | Tape | Sub | Knock | Hollow | Crunch | Dust | Damped | Round | Slam |
| Snare | Back | Crack | Wood | Deep | Clap | Snap | Grit | Brush | Wire | Ring |
| Closed Hat | Tick | Tight | Tape | Dust | Metal | Chick | Soft | Crunch | Vinyl | Cut |
| Open Hat | Sizzle | Short | Tape | Dust | Metal | Loose | Soft | Crunch | Wash | Chop |
| Low Tom | Body | Tape | Floor | Knock | Hollow | Crunch | Dust | Damped | Round | Tribal |
| High Tom | Body | Tape | Rack | Knock | Hollow | Crunch | Dust | Damped | Round | Tribal |
| Crash | Classic | Tape | Dust | Thin | Dark | Trash | Short | Soft | Crunch | Wash |
| Ride | Bell | Tape | Dust | Ping | Dark | Metal | Short | Soft | Crunch | Wash |

| Channel | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Kick | Break | Felt | Knuckle | DeepTap | Warm | Cabin | Brick | Velvet | Boombox | Punch |
| Snare | Chop | Philly | Oak | Rim | Fat | Paper | Vinyl2 | Room | Dusty | Back2 |
| Closed Hat | Silk | Brush | Bronze | Break | Pss | Mellow | Stick | Dry90 | Sandy | Hush |
| Open Hat | Silk | Bronze | Sandy | Break | Wash2 | DryAir | Dark2 | Open90 | Smooth | Shhh |
| Low Tom | Maple | Skin | Dry90 | Satin | Thud | Wood2 | Root | Lounge | Shell | Warm |
| High Tom | Maple | Skin | Dry90 | Satin | Thud | Wood2 | Root | Lounge | Shell | Warm |
| Crash | Bronze | Break | Velvet | Paper | Room | Dust90 | Brass | Air | Ribbon | Soft90 |
| Ride | Bronze | Jazz | Soft90 | Stick | Velvet | Edge | Dry90 | Dark2 | Air | Room |

## Sound design

- **Kick:** Classic Thump keeps the previous voice. Tape/Round/Sub favour a warmer low body; Knock/Hollow emphasise different shell overtones; Crunch/Dust add reduced-bandwidth grit; Damped is shorter; Slam combines a heavier transient and driven body.
- **0.4 refinements:** Snare shell modes and a two-stage wire envelope separate body from crispness. Hat and cymbal voices combine 12–18 damped inharmonic modes with band-filtered noise. Open hats and crashes have softer rises and longer, smoother fades; their coarser presets retain colour with less harsh bit/hold processing. Slots 11–20 add warmer break, bronze, silk, wood, room and dusty variations. These are original synthesized sources, not recordings of an acoustic kit.
- **Snare:** Back keeps the previous dry backbeat. Crack/Snap have faster attacks; Wood changes the shell modes and body/noise balance; Deep adds a lower, longer body; Clap adds delayed noise bursts; Grit is filtered and driven; Brush is softer; Wire sustains the noise layer; Ring emphasises shell resonance.
- **Closed/Open Hat:** Tight/Short/Chop are more sharply gated; Tape/Dust/Soft have reduced brightness; Metal/Loose expose more inharmonic oscillator content; Chick adds a stronger onset; Crunch adds coarser saturation/quantisation; Vinyl/Wash favour filtered noise. Closed hats choke open hats across all preset combinations.
- **Low/High Tom:** Tape/Floor/Rack/Round favour deeper bodies; Knock/Damped emphasise shorter attacks; Hollow/Tribal use stronger inharmonic shell modes; Dust/Crunch add coloration. High Tom voices occupy a complementary higher tuning range.
- **Crash/Ride:** Tape/Dust/Dark/Soft reduce brightness; Thin/Ping/Metal emphasise selected modes and bell attack; Trash/Crunch use more aggressive coloration; Short trims the tail; Wash sustains the noise texture.

Slot 01 in every channel preserves that channel's original sound from 0.2.1. Slots 02–20 are designed recipes, with refined snare/hat/cymbal rendering in 0.4. Variations change pitch envelopes, modal content, body/noise proportions, attack and decay, filtering, saturation, quantisation and sample-hold rate. These are source-level variations, not mixer presets or renamed copies.

Every sound has three velocity layers and three deterministic round-robin variations. New samples are generated once at 32 kHz and shared between instances; the original slot-01 voices retain native-rate generation for backwards sound compatibility. Switching choices performs no generation, decoding, locking or allocation in the processing callback. New one-shots use linear interpolation at the host sample rate. This palette deliberately targets coloured, warm, grainy drums rather than a pristine recorded acoustic kit.

## Research and design references

The references below informed the design direction; their samples, recordings and presets are not bundled. This work is original analogue-style digital synthesis, rather than an exact hardware-circuit model.

- [Roland: Designing Custom Drums for TR-909 and TR-808](https://articles.roland.com/designing-custom-drums-for-roland-clouds-tr-909-and-tr-808/) — pitch/tone/decay and instrument-specific voicing provided a starting point for contrasting bass, noise and metallic sources.
- [Roland TR-808 technical specifications](https://support.roland.com/hc/en-us/articles/201963539-TR-808-Technical-Specifications) — documented bass-drum tone/decay, snare tone/snappy, tom tuning and cymbal/hat decay support the chosen families of controls.
- [Native Instruments: What is boom bap?](https://www.native-instruments.com/blogs/blog/what-is-boom-bap) — vintage, snappy drums informed the short attacks and deliberately coloured palette.
- [MPC Store: Marco Polo Lo-Fi Boom Bap Vol. 1](https://www.thempcstore.com/expansions/marco-polo-lo-fi-boom-bap-volume-1/) — its described warmth, grit, dusty snares and hard kick character informed the tonal targets. The linked product also provides an audio demonstration for comparison; no claim is made to reproduce that pack's sounds.

Sources consulted 17 November 2026. The resulting recipes are the project's own interpretation of these characteristics.

## Optional dry WAV previews

The [**RaBubanj-0.4.0-Factory-Audio** download](https://github.com/voyagersquare/RaBubanj/actions/runs/44183161228/artifacts/14371589929) contains all 160 dry one-shot previews, labelled by channel, slot and name, plus this catalogue. They are mono 32 kHz / 16-bit PCM, rendered at velocity 0.92 with the first round robin. They have no channel EQ, pan, Reverb or master coloration. This optional package is for listening; the plugin generates its factory bank internally.

Developers can render the same files with `factory_preset_tests <output-directory>`. Normal test mode checks every choice for distinct, finite, audible output, velocity response, bounded tails and supported host rates, plus choke and preservation of playing notes when a preset changes.
