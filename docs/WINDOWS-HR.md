# RaBubanj 0.7.0 — Windows x64, bez instalera

1. Preuzmi ZIP `RaBubanj-0.7.0-Windows-x64` iz uspješnog GitHub Actions builda i raspakiraj ga.
2. Zatvori DAW i kopiraj cijelu mapu **RaBubanj.vst3** u `C:\Program Files\Common Files\VST3\`. Zamijeni cijeli prethodni paket.
3. Ponovno skeniraj VST3 instrumente u 64-bitnom DAW-u, dodaj RaBubanj na MIDI traku i uključi arm/monitoring.
4. `RaBubanj.exe` je samostalna aplikacija, bez instalera; u njoj odaberi audio i MIDI uređaj.

## PAD nazivi i veći natpisi

Osam zvučnih kanala sada su **PAD 1–PAD 8**. Isti nazivi prikazuju se u kanalnom EQ-u i uz PITCH/DRIVE/COMP. Zaokruženi natpisi imaju veći, podebljani font. Prijašnji zvukovi i svi uvezeni semplovi ostaju na svojim kanalima.

## Step sekvencer

1. Klikni **STEP SEQ** u zaglavlju. Otvara se pomični prozor unutar plugina, s **32 koraka i 8 redova**. Desno su PAD 1–PAD 8. Koraci su šesnaestinke; cijeli obrazac traje dva takta u 4/4.
2. Odaberi **SLABA / SREDNJA / JAKA** (MIDI velocity **48 / 88 / 127**).
3. **Desni klik upisuje korak. Lijevi klik briše.** Drži lijevu tipku i povlači za brisanje više koraka. I pri brzom povlačenju brišu se koraci između dvaju položaja miša.
4. Ostavi **SEQ ON** i pokreni DAW razmaknicom. Sekvencer prati njegov tempo, položaj pjesme, Play/Stop i loop. Razmaknica se prosljeđuje DAW-u; nema zasebnog lokalnog sata ni Play tipke. Instrument traka mora imati uključen monitoring/arm i aktivan audio engine.
5. Povuci **DRAG MIDI** na MIDI traku u DAW-u. Izvezeni klip ima ista mjesta i jačine udaraca te trenutačni MIDI raspored. Pri sviranju izvezenog klipa postavi **SEQ OFF** da ne sviraju istodobno i klip i interni obrazac.

Obrazac, jačina za upis i SEQ ON/OFF spremaju se u projekt. Nova instanca i stari projekti bez sekvencera počinju praznim obrascem. Zatvaranje prozora ne briše obrazac i ne isključuje ga. Stop zaustavlja nove udarce; repovi semplova i reverba mogu prirodno odjeknuti.

MIDI izvoz je standardna datoteka jednog tracka, 960 PPQ, kanal 10, duljine osam beatova. Privremene MIDI datoteke ostaju u OS temp mapi `RaBubanj-MIDI` da ih DAW može naknadno pročitati. Samostalni EXE nema DAW transport za sinkronizaciju sekvencera, ali zadržava preslušavanje padova i MIDI ulaz.

## Oker konzola i EQ izbornik

Jednobojna oker površina zamjenjuje prijašnju hrđu i ogrebotine. Nazivi komandi su bijeli. EQ paneli imaju dublje reljefne rubove, a svijetli rebrasti potenciometri s obojenim kapicama, faderi i tipke oblikovani su prema priloženom primjeru studijske miksete. Graf i izbornici koriste tamnu toplu podlogu, uz zlatnu krivulju i skale.

## Nazivi kanala i preslušavanje

**Naziv kanala više ne svira zvuk.** U njemu se prikazuje indikator glasnoće, a klik odabire kanalni EQ i njegove procesore. Donji **PLAY** gumb jednom odsvira trenutačni sempl tog kanala i zasvijetli žuto pri pritisku. Uklonjeni su izbornici i strelice tvorničkih preseta. Nazivi uvezenih semplova i drag-and-drop ostaju na istom mjestu.

## PITCH, DRIVE i COMP

U panelu uz Reverb nalaze se tri kružna gumba za trenutačno odabrani drum kanal. Klikni PAD 1 za njegove postavke, PAD 2 za njegove i tako redom. Svaki od osam kanala pamti zasebne vrijednosti, uz automatizaciju i spremanje u DAW projektu.

- **PITCH:** −24 do +24 polutona, početno 0. Mijenja visinu, brzinu i trajanje sampla. Promjene tijekom sviranja su zaglađene.
- **DRIVE:** 0–100% zasićenja, početno 0%. Obrada koristi antialiasing kako bi se smanjila oštrina visokih harmonika.
- **COMP:** jedan gumb za jačinu kompresije, početno 0%. Fiksni threshold −18 dBFS, ratio 3:1, soft knee 6 dB, attack 10 ms i opto stil releasea 150–350 ms ovisno o redukciji. Stereo kanali obrađuju se povezano; nema automatskog makeup gaina.

Dvoklik vraća neutralnu vrijednost. Za Reverb povrat ova tri drum procesora su onemogućena. Postojeće Reverb Size/Damping/Width kontrole ostaju zasebne.

## Mikser

Level, pan, M/S, SEND i Reverb/master ostaju kao prije. SEND počinje i dvoklikom se vraća na **0%**, prati level/mute kanala i šalje na zajednički mokri Reverb povrat. Solo Reverb kanala služi preslušavanju samo mokrog signala. Plugin ima jedan stereo izlaz.

## Parametarski EQ

Svaki drum kanal, Reverb povrat i master imaju po šest neovisnih bandova.

1. Klikni kanal, zatim njegovu točku na grafu. Uklonjeni su duplicirani gumbi 1–6 i brojčani kliznici.
2. Povuci točku lijevo/desno za frekvenciju i gore/dolje za gain.
3. Kotačić miša mijenja **Q**, odnosno širinu banda. Zadrži miš na grafu za prikaz vrijednosti odabranog banda u tooltipu.
4. **Desnim klikom na točku** otvori tamni izbornik, pa u **Curve shape** odaberi **Bell**, **Low shelf**, **High shelf**, **Low cut**, **High cut** ili **Notch**. Stari padajući izbornik ispod grafa uklonjen je. Low/High cut imaju nagib 12 dB po oktavi. Gain nije primjenjiv na cut i notch filtre.
5. U istom izborniku **Gain (dB)** bira gain u koracima, **Invert gain** obrće njegov predznak, a **Band enabled** uključuje/isključuje band. Gain nije ponuđen za cut/notch. **Alt-klik** izravno uključuje/isključuje band, **BYPASS** zaobilazi cijeli odabrani EQ, a **RESET** vraća njegove bandove na ravnu krivulju. Dvoklik na točku vraća gain tog banda na 0 dB.

Raspon je 20 Hz–20 kHz, ±24 dB i Q 0,15–18. Graf prikazuje frekvencijski odziv filtara koji stvarno obrađuju audio. Ova verzija nema dinamički EQ ni linear-phase način rada.

U oba EQ grafa sada radi **živi analizator spektra** iza krivulje EQ-a. Kanalni prati odabrani kanal nakon EQ-a, prije fadera; master prati završni stereo izlaz. Lijeva skala ±24 dB odnosi se na EQ, a desna skala −90 do 0 dBFS na spektar. Analizator je samo prikaz i ne dodaje latenciju zvuku.

Svi mikser, EQ i novi procesorski parametri spremaju se u projekt. Stari projekti dobivaju neutralne nove procesore i zadržavaju svoje uvezene semplove i postavke. Stari interni izvori zvuka ostaju za kompatibilnost; tvornički odabir uklonjen je iz sučelja.

## MIDI i zvukovi

Zadani raspored prati sve tipke redom: **C1 PAD 1 (36), C#1 PAD 2 (37), D1 PAD 3 (38), D#1 PAD 4 (39), E1 PAD 5 (40), F1 PAD 6 (41), F#1 PAD 7 (42), G1 PAD 8 (43)**. Ovo su Abletonove oznake oktave; u drugim DAW-ovima prati brojeve nota. Prihvaćaju se svi MIDI kanali. Za stare GM klipove klikni gumb **MIDI: C1-G1** u zaglavlju da prijeđe na **MIDI: GM** (stari raspored 36/38/42/46/45/48/49/51 i njegove alias note).

Za sviranje računalnom tipkovnicom u Abletonu uključi njegov computer MIDI keyboard, arm/monitoring trake i postavi početnu oktavu na C1. Na pluginu ostavi **KEYS: DAW** kako bi DAW primao tipke i mogao snimati MIDI. **KEYS: PLUGIN** služi lokalnom preslušavanju tipkama **A W S E D F T G**, redom kroz osam instrumenata, dok prozor ima fokus. Držanje tipke ne ponavlja udarce. Lokalno preslušavanje ne zapisuje MIDI u DAW; tijekom snimanja tipke se prosljeđuju hostu. MIDI kontroler radi u oba načina.

Note-off ne prekida one-shot; zatvoreni hi-hat prekida otvoreni. CC120/123 prekida glasove i čisti EQ/reverb rep.

## Vlastiti semplovi

Povuci **jednu audio datoteku na željeni drum kanal** iz DAW preglednika ili Explorera. Plugin prima lokalne datoteke i file URI-je; interni DAW klipovi koji ne daju audio datoteku trebaju se prvo izvesti/konsolidirati. Podržani su WAV, AIFF, FLAC i Ogg: mono/stereo, 8–384 kHz, do 30 sekundi i najviše 32 MB datoteke i dekodiranog audija.

Ispod naziva kanala prikazuje se učitavanje, zatim naziv sempla. Stereo se čuva, a brzina reprodukcije prilagođava DAW-u. Sempl mijenja samo taj instrument, uz postojeći EQ i mikser. **x** ga uklanja i vraća početni izvor tog kanala. Učitavanje radi u pozadini. Neuspjelo novo učitavanje zadržava prethodni zvuk; detalji greške nalaze se u tooltipu.

Audio datoteka ugrađuje se u spremljeni DAW projekt, pa projekt radi i nakon premještanja izvorne datoteke, uz veće zauzeće prostora. Pri ponovnom otvaranju pričekaj da se prikaže naziv učitanog sempla. Neispravan ili nedostupan spremljeni sempl ostaje tih uz poruku greške; povuci zamjenu ili klikni **x**.

Bez uvezenog sempla svaki kanal zadržava jedan početni ugrađeni zvuk. Učitani stari projekti mogu zadržati svoj prijašnji izvor do zamjene semplom. Tvornički izbor i novi paket tvorničkih WAV pregleda nisu dio verzije 0.5.

Ako nema zvuka, provjeri monitoring, MIDI ulaz/raspored, M/S gumbe i audio izlaz. Za uklanjanje zatvori DAW i izbriši kopiranu `.vst3` mapu.
