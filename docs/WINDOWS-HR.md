# RaBubanj 0.4.0 — Windows x64, bez instalera

1. Preuzmi ZIP `RaBubanj-0.4.0-Windows-x64` iz uspješnog GitHub Actions builda i raspakiraj ga.
2. Zatvori DAW i kopiraj cijelu mapu **RaBubanj.vst3** u `C:\Program Files\Common Files\VST3\`. Zamijeni prethodnu verziju cijelim paketom. Nemoj izdvajati samo datoteku iz unutrašnjosti VST3 mape.
3. Otvori 64-bitni DAW i ponovno skeniraj VST3 instrumente. Dodaj RaBubanj na instrument/MIDI traku i uključi monitoring.
4. Sviraj MIDI note ili klikni **naziv bubnja** za preslušavanje njegova odabranog preseta. `RaBubanj.exe` služi za samostalno sviranje; nije installer. U njemu odaberi audio i MIDI uređaj.

## Izgled

Tamoplava metalna površina ima izražene ogrebotine, oguljenu boju, izloženi metal, nepravilne hrđave mrlje i prašinu. Paneli, gumbi i kružne kontrole imaju izdignute rubove i sjene. Uklonjeni su dodatni tekstovi iz zaglavlja i opis signalnog puta ispod reverb kontrola.

## Kompaktni mikser

- **Naziv kanala:** klik na Kick, Snare ili drugi drum naziv odabire njegov EQ i jednom odsvira izabrani zvuk. Klik drugdje na kanal odabire EQ bez preslušavanja. Reverb i Master ne sviraju drum zvukove.
- **Tvornički preset:** strelice lijevo/desno na dnu svakog drum kanala prolaze kroz njegovih 20 zvukova. Izbor sam po sebi ne pokreće udarac; za preslušavanje klikni naziv kanala. Preseti ne prepisuju mikser, sendove ni EQ. Ukupno je 160 zvukova; [katalog i reference](FACTORY-PRESETS.md).
- **Level:** veći srebrni fader, s vrijednošću u dB. Dvoklik vraća 0 dB na drum kanalima, odnosno -8 dB na masteru.
- **SEND:** manji fader odmah uz Level. Određuje postotak signala koji taj bubanj šalje na zajednički Reverb bus. Dvoklik vraća 0%. Send prati Level i Mute kanala.
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

Svi EQ, send i mixer parametri podržavaju automatizaciju i spremaju se u DAW projektu. Pamte se i odabrani kanal te zaseban tvornički preset svakog bubnja. Stari projekti automatski koriste prvi preset za očuvanje izvornog zvuka. Projekti iz verzije 0.1 učitavaju se s ravnim EQ-ima i sendovima na 100%, čime zadržavaju prethodni zajednički room feed. Novi projekti počinju sa sendovima na 0%. Spremljeni sendovi od 25% iz prethodnih verzija prelaze na 0%, dok se ostale prilagođene vrijednosti čuvaju.

## MIDI i zvukovi

Zadani raspored prati sve tipke redom, uključujući crne: **C1 Kick (36), C#1 Snare (37), D1 Closed Hat (38), D#1 Open Hat (39), E1 Low Tom (40), F1 High Tom (41), F#1 Crash (42), G1 Ride (43)**. Ovo su Abletonove oznake oktave; u drugim DAW-ovima prati brojeve nota. Prihvaćaju se svi MIDI kanali. Za stare GM klipove klikni gumb **MIDI: C1-G1** u zaglavlju da prijeđe na **MIDI: GM** (stari raspored 36/38/42/46/45/48/49/51 i njegove alias note).

Za sviranje računalnom tipkovnicom u Abletonu uključi njegov computer MIDI keyboard, arm/monitoring trake i postavi početnu oktavu na C1. Na pluginu ostavi **KEYS: DAW** kako bi DAW primao tipke i mogao snimati MIDI. **KEYS: PLUGIN** služi lokalnom preslušavanju tipkama **A W S E D F T G**, redom kroz osam instrumenata, dok prozor ima fokus. Držanje tipke ne ponavlja udarce. Lokalno preslušavanje ne zapisuje MIDI u DAW; tijekom snimanja tipke se prosljeđuju hostu. MIDI kontroler radi u oba načina.

Note-off ne prekida one-shot; zatvoreni hi-hat prekida otvoreni. CC120/123 prekida glasove i čisti EQ/reverb rep.

Banka ima 160 vlastitih, sintetski generiranih boom-bap zvukova, s tri velocity sloja i tri round-robin varijacije. Prvi preset svakog kanala zadržava prethodni zvuk; ostalih 19 pruža razne varijacije. Prerađeni snare ima slojeve tijela, rezonancija i žica; činela i otvoreni hi-hat imaju gušći metalni odziv, glađi napad i prirodnije propadanje. Preset se primjenjuje na sljedeći udarac, bez prekidanja postojećeg repa.

## Vlastiti semplovi

Povuci **jednu audio datoteku na željeni drum kanal** iz DAW preglednika ili Explorera. Plugin prima lokalne datoteke i file URI-je; interni DAW klipovi koji ne daju audio datoteku trebaju se prvo izvesti/konsolidirati. Podržani su WAV, AIFF, FLAC i Ogg: mono/stereo, 8–384 kHz, do 30 sekundi i najviše 32 MB datoteke i dekodiranog audija.

Ispod naziva kanala prikazuje se učitavanje, zatim naziv sempla. Stereo se čuva, a brzina reprodukcije prilagođava DAW-u. Sempl mijenja samo taj instrument, uz postojeći EQ i mikser. **x** ga uklanja i vraća trenutni tvornički zvuk; strelice također vraćaju tvorničku banku. Učitavanje radi u pozadini. Neuspjelo novo učitavanje zadržava prethodni zvuk; detalji greške nalaze se u tooltipu.

Audio datoteka ugrađuje se u spremljeni DAW projekt, pa projekt radi i nakon premještanja izvorne datoteke, uz veće zauzeće prostora. Pri ponovnom otvaranju pričekaj da se prikaže naziv učitanog sempla. Neispravan ili nedostupan spremljeni sempl ostaje tih uz poruku greške; povuci zamjenu ili klikni **x**.

Ako nema zvuka, provjeri MIDI raspored, monitoring, ulaz kontrolera, M/S gumbe i audio izlaz DAW-a. Za snimanje i aranžiranje koristi DAW.

Za uklanjanje zatvori DAW i izbriši kopiranu `.vst3` mapu. Nema instalera.
