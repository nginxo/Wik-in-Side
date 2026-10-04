# GeoOS ESP32 - Wikipedia & GPS Explorer

Sistema Operativo embedded per ESP32 che unifica display ST7789, joystick analogico, ricevitore GPS, chiamate alle API di Wikipedia (GeoSearch), generatore di codici QR e feedback combinato audio e vibrazione aptica.

---

## 📁 Struttura del Progetto

```text
OS_ARDUINO/
│
├── OS_ARDUINO.ino          # Orchestratore principale dell'OS (setup, loop, gestione viste)
├── config.h                # Configurazione centrale dei PIN e dei parametri di sistema
│
├── DisplayManager.h/.cpp   # Gestione display ST7789 (interfaccia a 5 viste, animazioni e radar)
├── JoystickManager.h/.cpp  # Calibrazione e lettura assi/pulsante su ADC1
├── FeedbackManager.h/.cpp  # Motore audio PWM e vibrazione con curve di inviluppo (audiovibration.c)
├── GPSManager.h/.cpp       # Parsing non bloccante TinyGPS++, calcolo distanze e modalità simulazione
├── WikiClient.h/.cpp       # Connessione Wi-Fi, chiamate HTTPS e parsing MediaWiki API (ArduinoJson 7)
├── QRManager.h/.cpp        # Generazione ed emissione QR code su TFT e Seriale (qrcodegen)
├── TestManager.h/.cpp      # Suite diagnostica interattiva attivabile inviando "TEST" su Seriale
│
├── qrcodegen.h             # Libreria QR Code (Nayuki)
└── qrcodegen.c
```

---

## 🔌 Pinout Hardware Consigliato (Zero Conflitti)

Tutti i pin sono stati riassegnati per evitare conflitti e aggirare il limite critico dell'ESP32 che **disabilita l'ADC2 quando il Wi-Fi è attivo**:

| Periferica | Segnale / Funzione | Pin ESP32 | Note Tecniche |
| :--- | :--- | :--- | :--- |
| **Display ST7789** | MOSI (SDA) | **GPIO 23** | SPI Hardware (VSPI) |
| | SCLK (SCL) | **GPIO 18** | SPI Hardware (VSPI) |
| | CS | **GPIO 5** | Chip Select |
| | DC | **GPIO 27** | Data / Command |
| | RST | **GPIO 4** | Reset |
| **Modulo GPS** | RX (da TX GPS) | **GPIO 16** | Hardware UART 1/2 |
| | TX (da RX GPS) | **GPIO 17** | Hardware UART 1/2 |
| **Joystick** | VRX (Asse X) | **GPIO 34** | **ADC1_CH6** (compatibile con Wi-Fi) |
| | VRY (Asse Y) | **GPIO 35** | **ADC1_CH7** (compatibile con Wi-Fi) |
| | SW (Pulsante) | **GPIO 32** | Con pull-up interno |
| **Feedback** | Buzzer (Audio) | **GPIO 25** | Canale LEDC PWM (2000 Hz) |
| | Vibro (Aptico) | **GPIO 26** | Canale LEDC PWM (1000 Hz) |

*(Tutti i pin sono facilmente modificabili all'interno di `config.h`)*.

---

## 🖥️ Interfaccia a 5 Viste dell'OS

Navigazione fluida tramite **Joystick**:
- **Stick Sinistra / Destra**: Scorri tra le 5 viste principali.
- **Stick Su / Giù**: Scorri i testi lunghi (con scrollbar dinamica) nella vista Articolo.
- **Pressione Stick (Click SW)**: Azione rapida contestuale:
  - *In Esplora*: Apri la scheda articolo (o forza una nuova ricerca se nessun monumento).
  - *In Articolo*: Mostra il QR Code.
  - *In QR*: Passa al Radar.
  - *In Impostazioni*: Attiva/Disattiva la modalità simulazione coordinate indoor.

### Le 5 Schermate:
1. **[1/5] ESPLORA**: Mostra il monumento più vicino rilevato, la distanza in metri/chilometri, le coordinate GPS attuali e un'anteprima della descrizione.
2. **[2/5] ARTICOLO**: Visualizzatore di testo per leggere l'estratto completo di Wikipedia del luogo rilevato, con scorrimento verticale continuo.
3. **[3/5] QR CODE**: Genera e disegna in alta definizione sul display il codice QR che punta direttamente all'articolo Wikipedia da inquadrare con lo smartphone.
4. **[4/5] RADAR**: Bussola/Radar grafico con cerchi di portata e indicatore del Nord che mostra graficamente la direzione e la posizione relativa del monumento rispetto all'utente.
5. **[5/5] IMPOSTAZIONI**: Stato della connessione Wi-Fi, raggio di ricerca configurabile, stato audio/vibrazione e toggle simulazione GPS.

---

## 🧪 Modalità Diagnostica TEST via Seriale

In qualunque momento durante il funzionamento dell'OS, aprendo il **Monitor Seriale** (115200 baud) e digitando:

```text
TEST
```

l'OS sospende le normali attività e apre il **Menu Diagnostico Hardware** dedicato, implementando punto per punto i riferimenti di test originali:

```text
======================================================================
               GeoOS ESP32 - MENU TEST E DIAGNOSTICA
   (Ispirato ai moduli di test: wikipedia, audiovibration, qrGen)
======================================================================
 1. Test Display ST7789 & Joystick        (Rif: wikipedia.ino)
 2. Test Audio & Vibrazione Aptica        (Rif: audiovibration.c)
 3. Test Generatore QR Code               (Rif: qrGen.c)
 4. Test Modulo GPS Hardware              (Rif: wikishootme.cpp)
 5. Test Wi-Fi & Chiamata API Wikipedia   (Rif: wikishootme.cpp)
 6. Diagnostica Hardware Completa (Self-Test automatico)
 X. Esci dalla modalita Test e torna all'OS
----------------------------------------------------------------------
```

- **Opzione 1 (wikipedia.ino)**: Monitoraggio live degli assi analogici VRX, VRY e dello stato del pulsante sia sulla Seriale che sullo schermo con mirino grafico.
- **Opzione 2 (audiovibration.c)**: Esecuzione delle frequenze 888 Hz, 1142 Hz e 1333 Hz con le curve di attacco, sustain e rilascio sia per l'altoparlante che per il motorino vibrazionale.
- **Opzione 3 (qrGen.c)**: Generazione del QR code con stampa a blocchi ASCII (`##` / `  `) su console seriale e disegno sul display.
- **Opzione 4 (wikishootme.cpp - GPS)**: Ricezione e decodifica live delle sentenze NMEA del GPS.
- **Opzione 5 (wikishootme.cpp - WiFi/API)**: Test di connessione ed esecuzione della query GeoSearch su Roma/Colosseo con recupero testo ed URL.
- **Opzione 6**: Test sequenziale automatico con tabella di PASS/FAIL.
- **Opzione X**: Ritorno immediato all'OS e ripristino dell'interfaccia grafica.

---

## ⚙️ Come Compilare e Caricare

1. Apri la cartella `OS_ARDUINO` con **Arduino IDE** (oppure apri direttamente `OS_ARDUINO.ino`).
2. Modifica in `config.h` le credenziali del tuo hotspot Wi-Fi (`DEFAULT_WIFI_SSID` e `DEFAULT_WIFI_PASS`).
3. Seleziona la tua scheda ESP32 (es. `ESP32 Dev Module`).
4. Premi **Carica**.
