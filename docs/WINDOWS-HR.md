# RaBubanj 0.3.0 — Windows x64, bez instalera

1. Preuzmi ZIP `RaBubanj-0.3.0-Windows-x64` iz uspješnog GitHub Actions builda i raspakiraj ga.
2. Zatvori DAW i kopiraj cijelu mapu **RaBubanj.vst3** u `C:\Program Files\Common Files\VST3\`. Zamijeni prethodnu verziju cijelim paketom. Nemoj izdvajati samo datoteku iz unutrašnjosti VST3 mape.
3. Otvori 64-bitni DAW i ponovno skeniraj VST3 instrumente. Dodaj RaBubanj na instrument/MIDI traku i uključi monitoring.
4. Sviraj MIDI note ili klikni **naziv bubnja** za preslušavanje njegova odabranog preseta. `RaBubanj.exe` služi za samostalno sviranje; nije installer. U njemu odaberi audio i MIDI uređaj.

## Izgled

Tamoplava metalna površina ima izražene ogrebotine, oguljenu boju, izloženi metal, nepravilne hrđave mrlje i prašinu. Paneli, gumbi i kružne kontrole imaju izdignute rubove i sjene. Uklonjeni su dodatni tekstovi iz zaglavlja i opis signalnog puta ispod reverb kontrola.

## Kompaktni mikser

- **Naziv kanala:** klik na Kick, Snare ili drugi drum naziv odabire njegov EQ i jednom odsvira izabrani zvuk. Klik drugdje na kanal odabire EQ bez preslušavanja. Reverb i Master ne sviraju drum zvukove.
- **Tvornički preset:** izbornik na dnu svakog drum kanala sadrži njegovih 10 zvukova. Izbor sam po sebi ne pokreće udarac; za preslušavanje klikni naziv kanala. Preseti ne prepisuju mikser, sendove ni EQ. Ukupno je 80 zvukova; [katalog i reference](FACTORY-PRESETS.md).
- **Level:** veći srebrni fader, s vrijednošću u dB. Dvoklik vraća 0 dB na drum kanalima, odnosno -8 dB na masteru.
- **SEND:** manji fader odmah uz Level. Određuje postotak signala koji taj bubanj šalje na zajednički Reverb bus. Dvoklik vraća 25%. Send prati Level i Mute kanala.
- **Pan:** mali horizontalni kliznik iznad fadera. Dvoklik centrira panoramu.
- **M / S:** mute / solo. Solo zadržava reverb od odabranog instrumenta. Prethodni reverb repovi prirodno se stišavaju.
- **Reverb kanal:** zaseban povratni fader u dB, Mute i Solo. Njegov Solo služi preslušavanju samo mokrog reverba. Klik na Reverb otvara njegov vlastiti EQ u kanalnom panelu.
- **Reverb postavke:** kružni potenciometri Size, Damping i Width u lijevoj polovici podijeljenog bočnog panela. Povuci mišem gore/dolje ili lijevo/desno, odnosno upiši postotak u polje ispod potenciometra. Desna polovica za sada ostaje prazna. Reverb ne dodaje dodatni suhi signal.
- **Master:** završni nivo, Mute i metar. Globalni EQ stalno je vidljiv gore desno i obrađuje cijeli stereo miks.

Reverb bus je interni kanal miksera; plugin i dalje ima jedan stereo izlaz u DAW-u. Nazivi i postojeći ID plugina ostaju isti, pa se nadogradnja učitava u postojećim projektima.

## Parametarski EQ

Svaki drum kanal, Reverb povrat i master imaju po šest neovisnih bandova.

1. Klikni kanal, zatim njegovu točku na grafu. Uklonjeni su duplicirani gumbi 1–6 i brojčani kliznici.
2. Povuci točku lijevo/desno za frekvenciju i gore/dolje za gain.
3. Kotačić miša mijenja **Q**, odnosno širinu banda. Zadrži miš na grafu za prikaz vrijednosti odabranog banda u tooltipu.
4. U padajućem izborniku odaberi **Bell**, **Low shelf**, **High shelf**, **Low cut**, **High cut** ili **Notch**. Low/High cut imaju nagib 12 dB po oktavi. Gain nije primjenjiv na cut i notch filtre.
5. **Desni klik na točku** (ili Alt-klik) uključuje/isključuje pojedini band, **BYPASS** zaobilazi cijeli odabrani EQ, a **RESET** vraća njegove bandove na ravnu krivulju. Dvoklik na točku vraća gain tog banda na 0 dB.

Raspon je 20 Hz–20 kHz, ±24 dB i Q 0,15–18. Graf prikazuje frekvencijski odziv filtara koji stvarno obrađuju audio. Ova verzija nema spektralni analizator, dinamički EQ ni linear-phase način rada.

Svi EQ, send i mixer parametri podržavaju automatizaciju i spremaju se u DAW projektu. Pamte se i odabrani kanal te zaseban tvornički preset svakog bubnja. Stari projekti automatski koriste prvi preset za očuvanje izvornog zvuka. Projekti iz verzije 0.1 učitavaju se s ravnim EQ-ima i sendovima na 100%, čime zadržavaju prethodni zajednički room feed. Novi projekti počinju sa sendovima na 25%.

## MIDI i zvukovi

Glavne note: Kick 36, Snare 38, Closed Hat 42, Open Hat 46, Low Tom 45, High Tom 48, Crash 49, Ride 51. Oktavne oznake razlikuju se među DAW-ovima; koristi brojeve nota. Plugin prihvaća sve MIDI kanale. Note-off ne prekida one-shot; zatvoreni hi-hat prekida otvoreni. CC120/123 prekida glasove i čisti EQ/reverb rep.

Banka ima 80 vlastitih analogno obojenih boom-bap zvukova, s tri velocity sloja i tri round-robin varijacije. Prvi preset svakog kanala zadržava prethodni zvuk; ostalih devet su nove varijacije. Preset se primjenjuje na sljedeći udarac, bez prekidanja postojećeg repa. Ako nema zvuka, provjeri monitoring, MIDI ulaz, M/S gumbe i audio izlaz DAW-a. Za snimanje i aranžiranje koristi DAW.

Za uklanjanje zatvori DAW i izbriši kopiranu `.vst3` mapu. Nema instalera.
