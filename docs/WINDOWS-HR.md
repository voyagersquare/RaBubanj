# RaBubanj 0.16.0 — Windows x64, bez instalera

1. Preuzmi ZIP `RaBubanj-0.16.0-Windows-x64` iz uspješnog GitHub Actions builda i raspakiraj ga.
2. Zatvori DAW i kopiraj cijelu mapu **RaBubanj.vst3** u `C:\Program Files\Common Files\VST3\`. Zamijeni cijeli prethodni paket.
3. Ponovno skeniraj VST3 instrumente u 64-bitnom DAW-u, dodaj RaBubanj na MIDI traku i uključi arm/monitoring.
4. `RaBubanj.exe` je samostalna aplikacija, bez instalera; u njoj odaberi audio i MIDI uređaj.

## PAD nazivi i veći natpisi

Osam zvučnih kanala sada su **PAD 1–PAD 8**. Isti nazivi prikazuju se u kanalnom EQ-u i uz PITCH/DRIVE/COMP. Zaokruženi natpisi imaju veći, podebljani font. Prijašnji zvukovi i svi uvezeni semplovi ostaju na svojim kanalima.

## Step sekvencer

1. Klikni **STEP SEQ** za otvaranje zasebnog prikaza koji zamjenjuje mikser ispod zaglavlja. Gornji gumb tada piše **MIXER**; on ili **crveni X** vraćaju mikser. Izbornikom **32 / 64 / 128 STEPS** biraš duljinu svakog patterna zasebno; prikaz ima **8 redova**, s oznakama **PAD 1–PAD 8 lijevo**. Koraci su šesnaestinke; obrazac traje **2, 4 ili 8 taktova u 4/4**.
2. U padajućem izborniku odaberi **Pattern 1, Pattern 2, Pattern 3 ili Pattern 4**. Svaki pamti zasebne korake. Promjena se čuje od sljedećeg audio bloka i zadržava položaj prema DAW-u.
3. **Lijevi klik upisuje najjači udarac.** Na istoj noti drugi klik daje srednji, treći slabiji, četvrti najslabiji, a peti ponovno najjači udarac: **127 → 88 → 48 → 32 → 127**. Jačina se vidi po visini i svjetlini ispunjenog polja. Gumbi SLABA/SREDNJA/JAKA su uklonjeni.
4. **Desni klik briše korak.** Drži desnu tipku i povlači za brisanje više koraka. Brzo povlačenje briše i korake između položaja miša. **Drži lijevu tipku i povlači** za upis najjačih nota u prazne ćelije. Brzo povlačenje popunjava i preskočene ćelije. Postojeće note zadržavaju jačinu i pomak; prelazak preko njih ne mijenja velocity.
5. Uz **SEQ ON**, DAW razmaknicom pokrećeš kao i prije. Sekvencer prati njegov tempo, položaj, Play/Stop i loop. Uključi monitoring/arm instrument trake i audio engine DAW-a.
6. Povuci **DRAG MIDI** na MIDI traku za izvoz odabranog patterna. Izvezeni klip zadržava sve četiri jačine i trenutačni MIDI raspored i odabranu duljinu patterna. Pri sviranju izvezenog klipa postavi **SEQ OFF** da ne sviraju istodobno klip i interni obrazac.

**Skrolanje:** iznad mreže odaberi 64 ili 128 STEPS. U mreži ostaju vidljiva 32 koraka; kotačićem miša (prema dolje za kasnije korake) ili donjim vodoravnim klizačem dolaziš do posljednjeg koraka. Oznake PAD ostaju lijevo. VIEW pokazuje vidljivi raspon, primjerice 97–128 / 128.

**Skraćivanje čuva note:** pri promjeni sa 128 na 32 koraka kasnije note i njihovi pomaci ostaju spremljeni. Dok su izvan odabrane duljine ne sviraju i ne izvoze se; produljivanjem ih vraćaš. Svaki pattern pamti vlastitu duljinu. Stariji projekti zadržavaju 32 koraka.

Klik na oznaku **PAD 1–PAD 8** lijevo jednom reproducira trenutačni sempl i odabire taj pad. Odabrana oznaka je svjetlija i odabir se prikazuje i u glavnom mikseru/EQ-u.

Na dnu povećanog prozora nalazi se **živi waveform odabranog pada**: otprilike posljednja sekunda stereo signala nakon Pitch/Drive/Comp/EQ-a, prije glasnoće i panorame. Prati udarce sekvence, MIDI-ja i preslušavanja. Klik na drugi PAD briše stari prikaz i prati novi zvuk. Dok nema audio obrade, prikaz zadržava posljednji primljeni signal. Svjetlije skupine po četiri koraka ostaju. Razdjelnici svakih osam koraka te jači razdjelnici svakih 16 koraka i oznake taktova olakšavaju praćenje ritma. Waveform, mreža i EQ zasloni imaju jednobojnu tamnu unutrašnjost.

**Dugi pritisak na notu (0,65 sekundi), bez pomicanja miša**, otvara **TIMING** kliznik. Negativna vrijednost aktivira udarac ranije, pozitivna kasnije, a 0% ostaje na mreži. Raspon je **−50% do +50% jednog koraka**, uz prikaz milisekundi prema trenutačnom tempu (na 120 BPM: ±62,5 ms). Dvoklik na kliznik vraća 0%. Dugi pritisak ne mijenja velocity; na praznoj ćeliji najprije upisuje najjaču notu. X ili klik na mrežu zatvara mali panel. Oznaka na noti pokazuje pomak ranije/kasnije.

Pomak se sprema za svaku notu i pattern te prenosi u MIDI izvoz. Rani prvi korak kružno prelazi na kraj dvotaktnog obrasca; prije početka reprodukcije DAW-a zvuk se ne može čuti unaprijed. Pri aktivnom DAW loopu pomak prati rubove tog loopa. Projekti iz 0.8/0.9 zadržavaju sve pattern-e s početnim pomakom 0%.

Sva četiri patterna, njihove duljine, pomaci nota, odabrani pattern i SEQ ON/OFF spremaju se u projekt. Novi plugin počinje četirima praznim patternima. Pri otvaranju projekta iz 0.7 njegov obrazac prelazi u Pattern 1 uz očuvane stare jačine 48/88/127, dok ostali ostaju prazni. Stariji projekti bez sekvencera također se otvaraju prazni. Zatvaranje prozora ne briše obrazac niti ga isključuje. Stop zaustavlja nove udarce; repovi semplova i reverba mogu prirodno odjeknuti.

MIDI izvoz je datoteka jednog tracka, 960 PPQ, kanal 10, duljine 8, 16 ili 32 beata, prema odabranih 32, 64 ili 128 koraka. Privremene datoteke ostaju u OS temp mapi `RaBubanj-MIDI`. Samostalni EXE nema DAW transport za sekvencer, ali zadržava preslušavanje padova i vanjski MIDI ulaz.

## Drveni paneli i metalne kontrole

Kućište, kanalni paneli, EQ okviri te Reverb/FX i sekvencer koriste izvornu drvenu teksturu iz poslane arhive. JPEG je ugrađen bez izmjene datoteke; prilagodba prikaza zadržava omjer slike, uz odvojeno osvjetljenje reljefa. Drvena tekstura u verziji 0.16 mirnija je, s manje izraženim godovima zahvaljujući mat crvenosmeđem sloju pri iscrtavanju; izvorni JPEG ostaje nepromijenjen.

Zaokružene metalne tipke zamijenjene su **ravnim bijelim natpisima na tamnim podlogama**, poput REVERB/PAD naslova: STEP SEQ, MIDI/KEYS, EQ BYPASS/RESET, nazivi kanala, M/S i PLAY. Isto vrijedi u sekvenceru za SEQ ON/OFF, DRAG MIDI, izbor patterna, PAD oznake i X. Klikovi i sve dosadašnje funkcije ostaju; PLAY pri pritisku svijetli žuto, X je crven, aktivna stanja i kanalni metri i dalje se prikazuju.

Kružni metalni gumbi s obojenim gornjim plohama **povećani su za 15% u odnosu na 0.12**, uz zadržane proporcije. Boje: SIZE tirkizna, DAMPING bijela, WIDTH bordo, PITCH plava, DRIVE crvena i COMP ljubičasta. Profilirani faderi ostaju prema primjeru konzole. Veliki natpisi iznad kontrola ostaju podebljani i kontrastni.

Ćelije sekvencera vraćene su na **jednostavan ravan izgled**: prve četiri su tamnije, sljedeće četiri svjetlije i tako do kraja odabrane duljine. Četiri jačine udarca, pomaci nota, uređivanje i MIDI izvoz ostaju kao prije.

## Veći crni natpisi i novi naziv

Natpisi otisnuti na drvenim površinama sada su veći i crni: EQ zaglavlja, 6-BAND PARAMETRIC, naslov miksera, nazivi semplova/Drop sample, PAN/dB/SEND, WET RETURN/STEREO OUT, POST SENDS/OUT 1/2 i donje upute. Bijeli natpisi na tamnim kontrolama i zaslonima ostaju čitljivi. Uklonjen je prekriženi natpis C1-G1 MIDI / SAMPLE PLAYER dolje desno; MIDI funkcije ostaju.

Naslov sekvencera, tempo/status, upute i natpisi prozora za pomak note također imaju veća crna slova na drvu. Mikser i EQ nisu vidljivi iza sekvencera.

Naziv RaBubanj prikazan je tamnocrvenom kaligrafijom prema primjeru. Ugrađen je font Great Vibes, pa nije potrebna instalacija fonta na računalu.

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
