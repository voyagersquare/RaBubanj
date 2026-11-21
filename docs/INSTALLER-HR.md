# RaBubanj 0.16.1 — Windows installer (VST3 + Standalone)

1. Zatvori DAW i prethodno otvorenu aplikaciju RaBubanj.
2. Raspakiraj preuzeti installer ZIP i pokreni **RaBubanj-0.16.1-Setup-x64.exe**. Instalacija u Program Files i zajedničku VST3 mapu traži administratorske ovlasti.
3. Odaberi **VST3 + Standalone**, samo **VST3 plugin** ili samo **Standalone application**. Ako želiš, uključi dodatni prečac na radnoj površini.
4. Dovrši instalaciju, ponovno pokreni DAW i po potrebi ponovno skeniraj VST3 instrumente. RaBubanj Standalone pronaći ćeš u Start izborniku, ako je odabran.

Podržani su Windows 10/11 x64. **VST** u ovom paketu znači **VST3**; stariji VST2 nije uključen.

## Mape

- VST3 paket: `C:\Program Files\Common Files\VST3\RaBubanj.vst3`.
- Samostalna aplikacija: `C:\Program Files\Voyagersquare\RaBubanj\RaBubanj.exe`, osim ako odabereš drugu mapu aplikacije.
- Upute, licence i izvorni kod: u mapi aplikacije. Mapa **Source** sadrži točan izvorni kod RaBubnja i pripadajući JUCE izvor. Instaliraju se uz sve odabrane komponente.

VST3 putanja ostaje standardna zajednička Windows mapa, neovisno o mapi koju odabereš za samostalnu aplikaciju. Za instalaciju nije potreban zaseban VC++ runtime paket jer su aplikacija i plugin izgrađeni sa statičkim runtimeom.

## Ponovna instalacija i deinstalacija

Ponovno pokretanje instalera osvježava odabrane komponente. Ako želiš ukloniti cijeli paket, koristi **Settings → Apps → Installed apps → RaBubanj → Uninstall**, ili **Uninstall RaBubanj** iz Start izbornika. Prethodno zatvori DAW i samostalnu aplikaciju. Instalacijske datoteke, VST3 i instalirani prečaci uklanjaju se kroz deinstalaciju; korisnički projekti i semplovi ne brišu se.

Izbor komponenata određuje što se instalira ili osvježava. Za uklanjanje prethodno instalirane komponente deinstaliraj paket, pa ponovno instaliraj samo željenu komponentu.

## Provjera preuzimanja

Uz Setup EXE nalazi se **SHA256SUMS.txt**. U PowerShellu možeš provjeriti datoteku naredbom:

```powershell
Get-FileHash .\RaBubanj-0.16.1-Setup-x64.exe -Algorithm SHA256
```

Installer trenutačno nema Authenticode digitalni potpis. Vrijednost SHA256 služi provjeri da se preuzeta datoteka podudara s objavljenom datotekom.

U samostalnoj aplikaciji odaberi audio i MIDI uređaj u Options. Step sekvencer prati transport DAW-a kada koristiš VST3; samostalna aplikacija nema DAW transport. Potpune upute za kontrole i sekvencer nalaze se u **PROCITAJ-ME.md** u instaliranoj mapi aplikacije.
