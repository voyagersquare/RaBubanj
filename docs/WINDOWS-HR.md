# RaBubanj 0.1 — Windows x64

## Pokretanje bez instalera

1. Preuzmi ZIP `RaBubanj-0.1.0-Windows-x64` iz uspješnog GitHub Actions builda i raspakiraj ga.
2. Zatvori DAW pa kopiraj cijelu mapu **RaBubanj.vst3** u `C:\Program Files\Common Files\VST3\`. Windows može tražiti administratorsko dopuštenje za kopiranje u tu mapu. Nemoj izdvajati samo datoteku iz unutrašnjosti VST3 paketa.
3. Otvori svoj 64-bitni DAW i ponovno skeniraj VST3 plugine.
4. Dodaj **RaBubanj** kao instrument na MIDI/instrument traku. Uključi monitoring trake i odaberi svoj MIDI ulaz.
5. Klikni padove ili pošalji MIDI note 36, 38, 42, 46, 45, 48, 49 i 51. Nije potrebno preuzimati zvukove.

Alternativa je vlastita VST3 mapa, ako je DAW podržava. `RaBubanj.exe` je samostalna aplikacija za preslušavanje i MIDI sviranje: odaberi audio izlaz i MIDI uređaj u audio postavkama aplikacije. Za aranžiranje i snimanje koristi DAW.

## Kontrole

- **Padovi:** preslušavanje pojedinog instrumenta; crtica ispod pada pokazuje MIDI/preview udarce.
- **Level:** glasnoća pojedinog instrumenta u dB. Dvoklik vraća 0 dB.
- **Pan:** -1 lijevo, 0 sredina, +1 desno. Dvoklik centrira panoramu.
- **Mute:** utišavanje instrumenta. Prethodno nastali room rep prirodno se stišava.
- **Master:** ukupna glasnoća; početno -8 dB radi prostora za istovremene udarce.
- **Room:** količina stereo reverba.

Mikser podržava automatizaciju; DAW sprema postavke s projektom. Svira na svim MIDI kanalima. Bubnjevi se odsvire do kraja bez obzira na note-off. Zatvoreni hi-hat prekida otvoreni. MIDI CC120 ili CC123 prekida sve glasove i reverb.

Ova početna verzija koristi vlastite generirane zvukove s tri velocity sloja i tri varijacije. Još nema biblioteku realističnih snimljenih bubnjeva, uvoz WAV datoteka, preglednik grooveova ni zasebne audio izlaze. Plugin ne sadrži vlastiti sekvencer: ritam šalje DAW ili MIDI kontroler.

Ako nema zvuka, provjeri da je plugin na instrument traci, da je monitoring uključen i da izlaz DAW-a ide na ispravan audio uređaj. Oktavne oznake razlikuju se među DAW-ovima; koristi MIDI brojeve iz README-a. Paket je za 64-bitni Windows i 64-bitni VST3 host; nije VST2 ni 32-bitni plugin.

Za uklanjanje zatvori DAW i izbriši kopiranu `.vst3` mapu. Nema deinstalera ni registra koji treba čistiti.
