# 02 — LoRa UART dengan Notifikasi LED (Sender + Non-Blocking Receiver)

> **Target Hardware:** Dragino LoRa Shield v1.2 &nbsp;·&nbsp; MCU: Arduino Uno (ATmega328P) &nbsp;·&nbsp; LoRa: SX1276

[← Kembali ke README Utama](../../README.md)

---

## Daftar Isi

- [Pendahuluan](#pendahuluan)
- [Tujuan Program](#tujuan-program)
- [Hardware yang Digunakan](#hardware-yang-digunakan)
- [Pin Definition](#pin-definition)
- [Catatan LED pada Pin D13](#catatan-led-pada-pin-d13)
- [Deskripsi File Source Code](#deskripsi-file-source-code)
- [Alur Kerja — Sender (02a)](#alur-kerja--sender-02a)
- [Alur Kerja — Receiver (02b)](#alur-kerja--receiver-02b)
- [Cara Kerja Detail](#cara-kerja-detail)
- [Topologi Komunikasi LoRa](#topologi-komunikasi-lora)
- [Konfigurasi LoRa](#konfigurasi-lora)
- [Library yang Digunakan](#library-yang-digunakan)
- [Cara Penggunaan](#cara-penggunaan)
- [Contoh Output Serial Monitor](#contoh-output-serial-monitor)
- [Troubleshooting](#troubleshooting)

---

## Pendahuluan

Subproject `02-Lora-UART-LED_notif` adalah pengembangan dari subproject 01, menambahkan dua fitur utama:

1. **Sender (02a):** Node transmitter yang mengirim pesan berurutan secara periodik setiap 2 detik, disertai kedipan LED setelah setiap transmisi berhasil
2. **Receiver Non-Blocking (02b):** Node receiver yang menggunakan **interupsi hardware via DIO0 (D2)** agar CPU tidak terblokir saat menunggu paket, dilengkapi indikator **LED** yang menyala sesaat setiap kali paket berhasil diterima

Perbedaan utama dibanding subproject 01:
- Receiver menggunakan **interrupt-driven** (`LoRa.onReceive()`), bukan polling blocking
- LED memberikan indikasi visual langsung saat paket diterima / berhasil dikirim
- Sender juga dilengkapi indikator LED setelah TX selesai

Seluruh kode ditulis untuk dan diuji pada **Dragino LoRa Shield v1.2 + Arduino Uno**.

---

## Tujuan Program

- Memperagakan komunikasi LoRa satu arah (TX → RX) secara lengkap
- Menunjukkan teknik penerimaan **non-blocking** menggunakan interupsi DIO0 (D2 / INT0)
- Memberikan feedback visual via LED saat paket dikirim dan diterima
- Menampilkan informasi link quality (RSSI, SNR) ke Serial Monitor

---

## Hardware yang Digunakan

| Komponen | Detail |
|---|---|
| **Board utama** | Arduino Uno (ATmega328P) |
| **Shield LoRa** | Dragino LoRa Shield v1.2 |
| **Modul LoRa** | SX1276 (onboard shield) |
| **Frekuensi** | **920 MHz** |
| **LED** | LED built-in Arduino pada **D13** (shared dengan SPI SCK) |
| **Antena** | Antena LoRa eksternal via konektor SMA (wajib dipasang) |
| **Jumlah board** | **2 set** — satu sebagai sender (COM8), satu sebagai receiver (COM9) |

---

## Pin Definition

### SPI Bus — Sama untuk 02a dan 02b

SPI menggunakan **hardware SPI Arduino Uno** melalui ICSP header (otomatis dikelola library).

| Sinyal | Arduino Pin | Keterangan |
|---|---|---|
| SCK | **D13** | Serial Clock (juga built-in LED) |
| MOSI | **D11** | Master Out Slave In |
| MISO | **D12** | Master In Slave Out |
| NSS / CS | **D10** | Chip Select SX1276 (R9 = 0 ohm terpasang) |

### Modul LoRa SX1276 — Sama untuk 02a dan 02b

| Sinyal | Arduino Pin | Keterangan |
|---|---|---|
| NSS (CS) | **D10** | SPI Chip Select |
| DIO0 | **D2** | Interrupt — RX done / TX done (INT0) |
| RST | **D9** | Reset modul SX1276 |
| DIO1 | **D6** | (terhubung di shield, tidak dipakai program ini) |
| DIO2 | **D7** | (terhubung di shield, tidak dipakai program ini) |
| DIO5 | **D8** | (terhubung di shield, tidak dipakai program ini) |

### LED

| Peripheral | Arduino Pin | Keterangan |
|---|---|---|
| LED built-in | **D13** | Nyala 150 ms setelah TX (sender) / 200 ms saat RX (receiver) |

> Lihat [Catatan LED pada Pin D13](#catatan-led-pada-pin-d13) untuk penjelasan keterbatasan dan alternatifnya.

### Jumper Resistor CS (penting)

| Resistor | Kondisi Default | Efek |
|---|---|---|
| **R9** | 0 ohm, **terpasang** | LoRa CS → **D10** (yang dipakai) |
| R10 | Tidak terpasang | (CS ke D5 jika dipasang) |
| R11 | Tidak terpasang | (CS ke D4 jika dipasang) |

---

## Catatan LED pada Pin D13

Pin D13 pada Arduino Uno berfungsi ganda: **built-in LED** sekaligus **SPI SCK (Serial Clock)**. Ketika SPI aktif (saat LoRa sedang kirim atau baca data), D13 akan berkedip mengikuti sinyal clock — ini normal dan tidak bisa dihindari di hardware.

**Dampak di program ini:**
- **Sender (02a):** LED blink dipicu *setelah* `LoRa.endPacket()` selesai (SPI sudah idle) — hasilnya bersih dan terkontrol.
- **Receiver (02b):** LED blink dipicu di `loop()` saat flag dari ISR aktif, *setelah* data dibaca — cukup bersih, sedikit kedip saat pembacaan SPI.

**Alternatif lebih bersih:** Pasang LED eksternal (LED + resistor 220Ω) di **D3** (pin bebas di Dragino Shield), lalu ganti di source code:
```cpp
#define LED_PIN  LED_BUILTIN   // sebelumnya
#define LED_PIN  3             // ganti ke ini untuk LED eksternal di D3
```

---

## Deskripsi File Source Code

```
02-Lora-UART-LED_notif/
├── README.md                                                    ← Dokumen ini
├── 02a-sender-dragino-LEDNotif/
│   └── *.ino                                                   ← SENDER (TX periodik, LED blink)
└── 02b-receiver-dragino-nonblocking-LEDNotif/
    └── *.ino                                                   ← RECEIVER non-blocking + LED
```

| File | Peran | Mode TX/RX | LED |
|---|---|---|---|
| `02a-sender-dragino-LEDNotif` | **Transmitter** | TX blocking tiap 2 detik | Blink 150 ms setelah TX |
| `02b-receiver-dragino-nonblocking-LEDNotif` | **Receiver** | RX non-blocking via interrupt DIO0 | Nyala 200 ms saat RX |

---

## Alur Kerja — Sender (02a)

```
setup()
  │
  ├─ Serial.begin(9600)
  ├─ pinMode(LED_PIN, OUTPUT)
  ├─ LoRa.setPins(NSS=10, RST=9, DIO0=2)
  ├─ LoRa.begin(920E6)
  ├─ LoRa.setSpreadingFactor(7)
  ├─ LoRa.setSignalBandwidth(125E3)
  └─ LoRa.setCodingRate4(5)

loop()
  │
  ├─ Buat pesan: "Hello #N"
  ├─ LoRa.beginPacket()
  ├─ LoRa.print(msg)
  ├─ LoRa.endPacket()           ← TX blocking (tunggu hingga selesai kirim)
  ├─ digitalWrite(LED, HIGH)    ← LED nyala (SPI sudah idle)
  ├─ delay(150)
  ├─ digitalWrite(LED, LOW)
  ├─ Serial.println("OK")
  ├─ counter++
  └─ delay(1850)                ← Tunggu sisa interval 2 detik
```

---

## Alur Kerja — Receiver (02b)

```
setup()
  │
  ├─ Serial.begin(9600)
  ├─ pinMode(LED_PIN, OUTPUT)
  ├─ LoRa.setPins(NSS=10, RST=9, DIO0=2)
  ├─ LoRa.begin(920E6)
  ├─ LoRa.setSpreadingFactor(7)
  ├─ LoRa.setSignalBandwidth(125E3)
  ├─ LoRa.setCodingRate4(5)
  ├─ LoRa.onReceive(onReceive)  ← Pasang callback ISR pada DIO0 (D2/INT0)
  └─ LoRa.receive()             ← Masuk mode RX non-blocking

ISR onReceive(packetSize)       ← Dipanggil hardware saat paket tiba
  └─ rxFlag = true

loop()
  │
  └─ if (rxFlag)                ← Cek flag dari ISR (NON-BLOCKING)
       │
       ├─ rxFlag = false
       ├─ digitalWrite(LED, HIGH)
       ├─ Baca data: LoRa.read() sampai available() habis
       ├─ Cetak pesan + RSSI + SNR ke Serial
       ├─ delay(200)             ← LED nyala 200 ms
       ├─ digitalWrite(LED, LOW)
       └─ LoRa.receive()        ← Kembali ke mode RX
```

---

## Cara Kerja Detail

### Sender (02a)

Sender mengirim pesan teks berurutan format `"Hello #N"` setiap ~2 detik. `LoRa.endPacket()` bersifat **blocking** — program menunggu hingga transmisi selesai sebelum melanjutkan. Setelah SPI idle, LED dinyalakan 150 ms sebagai konfirmasi visual TX berhasil.

**TX Power:** 17 dBm — nilai maksimum yang disarankan untuk SX1276.

### Receiver Non-Blocking (02b)

Receiver menggunakan mekanisme **interrupt-driven** yang efisien:

1. `LoRa.onReceive(onReceive)` mendaftarkan fungsi `onReceive()` sebagai callback ISR pada pin DIO0 (D2 = INT0 Arduino Uno)
2. `LoRa.receive()` menempatkan SX1276 dalam mode receive kontinyu — CPU bebas melakukan hal lain
3. Saat SX1276 mendeteksi paket, hardware memicu interrupt pada DIO0
4. Callback `onReceive()` dijalankan — set flag `rxFlag = true`
5. Di `loop()`, cek flag `rxFlag`; jika aktif, baca data via `LoRa.read()`
6. LED menyala 200 ms sebagai indikator visual paket diterima
7. `LoRa.receive()` dipanggil kembali untuk siap menerima paket berikutnya

> **Catatan AVR:** Berbeda dengan ESP32 (yang memerlukan `IRAM_ATTR`), pada ATmega328P semua kode berjalan dari flash secara sinkron — tidak ada kebutuhan atribut khusus pada fungsi ISR.

---

## Topologi Komunikasi LoRa

```
┌────────────────────────────┐        LoRa 920 MHz         ┌──────────────────────────────┐
│   BOARD A — Sender (COM8)  │ ──────────────────────────► │  BOARD B — Receiver (COM9)   │
│   02a-sender-dragino.ino   │     "Hello #0", "Hello #1"  │  02b-receiver-dragino.ino    │
│   LED blink tiap TX        │        setiap 2 detik       │  LED blink saat RX           │
└────────────────────────────┘                             └──────────────────────────────┘
```

- **Topologi:** Point-to-Point (satu sender, satu receiver)
- **Arah data:** Unidirectional (satu arah — hanya TX ke RX)
- **Format payload:** String teks `"Hello #N"`
- **Tidak ada ACK / retry** — fire and forget

---

## Konfigurasi LoRa

| Parameter | Nilai | Berlaku di |
|---|---|---|
| Frekuensi | **920 MHz** | 02a + 02b |
| Bandwidth | **125 kHz** | 02a + 02b |
| Spreading Factor | **SF7** | 02a + 02b |
| Coding Rate | **4/5** | 02a + 02b |
| TX Power | **17 dBm** | 02a saja |

> Parameter **harus identik** antara sender dan receiver agar komunikasi berhasil.

---

## Library yang Digunakan

| Library | Versi | Fungsi |
|---|---|---|
| `LoRa` (sandeepmistry) | 0.8.x | Driver SX1276: init, TX, RX non-blocking, interrupt, RSSI, SNR |
| `SPI.h` | bawaan Arduino | Komunikasi SPI hardware ke modul SX1276 |

Install via Arduino CLI:
```bash
arduino-cli lib install "LoRa"
```

---

## Cara Penggunaan

### Persiapan

- Siapkan **2 set** Arduino Uno + Dragino LoRa Shield v1.2 (versi 920 MHz)
- Pasang antena SMA pada kedua shield
- Pastikan **R9 (0 ohm) terpasang** di shield (CS ke D10) — cek jumper resistor
- Install library **LoRa** via Arduino CLI atau Library Manager

### Upload ke Board A (Sender → COM8)

```bash
arduino-cli compile --fqbn arduino:avr:uno 02a-sender-dragino-LEDNotif
arduino-cli upload -p COM8 --fqbn arduino:avr:uno 02a-sender-dragino-LEDNotif
```

Buka Serial Monitor **COM8** (9600 baud) — tampil status pengiriman tiap 2 detik.

### Upload ke Board B (Receiver → COM9)

```bash
arduino-cli compile --fqbn arduino:avr:uno 02b-receiver-dragino-nonblocking-LEDNotif
arduino-cli upload -p COM9 --fqbn arduino:avr:uno 02b-receiver-dragino-nonblocking-LEDNotif
```

Buka Serial Monitor **COM9** (9600 baud) — tampil pesan diterima + RSSI + SNR, LED berkedip tiap paket.

> **Tutup Serial Monitor sebelum upload** — port COM tidak bisa dipakai bersamaan oleh dua aplikasi.

---

## Contoh Output Serial Monitor

### Board A — Sender (COM8)

```
=== LoRa SENDER (Dragino) ===
Init LoRa ... OK
Freq: 920.00 MHz | BW: 125 kHz | SF7 | Power: 17 dBm
Mulai kirim tiap 2 detik...

[TX] Kirim: "Hello #0" ... OK
[TX] Kirim: "Hello #1" ... OK
[TX] Kirim: "Hello #2" ... OK
```

### Board B — Receiver (COM9)

```
=== LoRa RECEIVER (Dragino) ===
Init LoRa ... OK
Freq: 920.00 MHz | BW: 125 kHz | SF7
Menunggu paket (non-blocking)...

================================
[RX] Pesan : Hello #0
[RX] RSSI  : -52 dBm
[RX] SNR   : 9.50 dB
================================

================================
[RX] Pesan : Hello #1
[RX] RSSI  : -51 dBm
[RX] SNR   : 9.75 dB
================================
```

---

## Troubleshooting

| Masalah | Kemungkinan Penyebab | Solusi |
|---|---|---|
| `Init LoRa ... GAGAL!` | SPI tidak terhubung / CS salah | Pastikan R9 (0 ohm) terpasang di shield; cek `NSS_PIN = 10` |
| `Init LoRa ... GAGAL!` | Shield tidak terpasang sempurna | Lepas dan pasang ulang shield ke Arduino |
| Sender OK tapi receiver tidak menerima | Frekuensi tidak cocok | Pastikan kedua sketch pakai `920E6` |
| Sender OK tapi receiver tidak menerima | Antena tidak terpasang | Pasang antena SMA pada kedua shield |
| LED tidak menyala / tidak berkedip | Pin D13 masih menjadi SCK aktif | Normal saat SPI aktif; LED akan blink setelah SPI idle |
| LED tidak berfungsi sama sekali | `LED_PIN` salah | Pastikan `#define LED_PIN LED_BUILTIN` atau ganti ke pin eksternal |
| Upload error: `Access is denied` | Serial Monitor sedang terbuka di port tersebut | Tutup Serial Monitor, lalu upload ulang |
| Receiver kehilangan beberapa paket | Jarak terlalu jauh / interferensi | Perkecil jarak; coba naikkan SF (`setSpreadingFactor(9)`) |
