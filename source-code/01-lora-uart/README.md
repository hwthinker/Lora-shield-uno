# 01 — LoRa UART (Sender → Receiver)

> **Target Hardware:** Dragino LoRa Shield v1.2 &nbsp;·&nbsp; MCU: Arduino Uno (ATmega328P) &nbsp;·&nbsp; LoRa: SX1276

[← Kembali ke README Utama](../../README.md)

---

## Daftar Isi

- [Pendahuluan](#pendahuluan)
- [Tujuan Program](#tujuan-program)
- [Hardware yang Digunakan](#hardware-yang-digunakan)
- [Pin Definition](#pin-definition)
- [Struktur File](#struktur-file)
- [Topologi Komunikasi](#topologi-komunikasi)
- [Alur Kerja Program](#alur-kerja-program)
- [Konfigurasi LoRa](#konfigurasi-lora)
- [Library yang Digunakan](#library-yang-digunakan)
- [Cara Penggunaan](#cara-penggunaan)
- [Contoh Output Serial Monitor](#contoh-output-serial-monitor)
- [Troubleshooting](#troubleshooting)

---

## Pendahuluan

Subproject `01-lora-uart` adalah contoh komunikasi LoRa **paling dasar** — satu arah dari sender ke receiver. Tidak ada LED, tidak ada ACK, tidak ada interrupt. Cocok sebagai titik awal untuk memverifikasi bahwa hardware dan library berfungsi.

---

## Tujuan Program

- Memverifikasi bahwa Dragino LoRa Shield v1.2 terhubung dengan benar ke Arduino Uno
- Mengirimkan string teks via LoRa dan menerimanya di board lain
- Menampilkan kualitas link (RSSI, SNR) ke Serial Monitor

---

## Hardware yang Digunakan

| Komponen | Detail |
|---|---|
| **Board utama** | Arduino Uno (ATmega328P) |
| **Shield LoRa** | Dragino LoRa Shield v1.2 |
| **Modul LoRa** | SX1276 (onboard shield) |
| **Frekuensi** | **920 MHz** |
| **Antena** | Antena LoRa eksternal via konektor SMA (wajib dipasang pada kedua board) |
| **Jumlah board** | **2 set** — satu sender (COM8), satu receiver (COM9) |

---

## Pin Definition

| Sinyal | Arduino Pin | Keterangan |
|---|---|---|
| NSS / CS | **D10** | SPI Chip Select (R9 = 0 ohm terpasang default) |
| DIO0 | **D2** | Digunakan library secara internal |
| RST | **D9** | Reset SX1276 |
| SCK | **D13** | SPI Clock (juga LED built-in) |
| MOSI | **D11** | SPI MOSI |
| MISO | **D12** | SPI MISO |

> Program ini tidak menggunakan interrupt atau LED — semua pin selain yang di atas bebas dipakai.

---

## Struktur File

```
01-lora-uart/
├── README.md
├── sender/
│   └── sender.ino       ← Upload ke COM8 — kirim "Hello LoRa #N" tiap 2 detik
└── receiver/
    └── receiver.ino     ← Upload ke COM9 — terima dan tampilkan ke Serial
```

---

## Topologi Komunikasi

```
┌──────────────────────────┐                      ┌──────────────────────────┐
│   SENDER (COM8)          │    LoRa 920 MHz       │   RECEIVER (COM9)        │
│   sender.ino             │ ─────────────────►    │   receiver.ino           │
│   Kirim tiap 2 detik     │   "Hello LoRa #N"     │   Polling parsePacket()  │
│   TX blocking            │                       │   Cetak data + RSSI + SNR│
└──────────────────────────┘                       └──────────────────────────┘
```

- **Topologi:** Point-to-Point (satu arah)
- **Mode TX:** Blocking (`LoRa.endPacket()`)
- **Mode RX:** Polling (`LoRa.parsePacket()`) — loop cek setiap iterasi
- **Tidak ada ACK / retry**

---

## Alur Kerja Program

### Sender (`sender.ino`)

```
setup()
  ├─ Serial.begin(9600)
  ├─ LoRa.setPins(10, 9, 2)
  ├─ LoRa.begin(920E6)
  └─ Set SF7, BW=125kHz, CR=4/5, Power=17dBm

loop()
  ├─ Buat string: "Hello LoRa #N"
  ├─ LoRa.beginPacket()
  ├─ LoRa.print(msg)           ← Masukkan payload ke buffer TX
  ├─ LoRa.endPacket()          ← TX blocking — tunggu hingga selesai
  ├─ Serial.println("terkirim")
  ├─ counter++
  └─ delay(2000)               ← Interval 2 detik
```

### Receiver (`receiver.ino`)

```
setup()
  ├─ Serial.begin(9600)
  ├─ LoRa.setPins(10, 9, 2)
  ├─ LoRa.begin(920E6)
  └─ Set SF7, BW=125kHz, CR=4/5

loop()
  ├─ int ps = LoRa.parsePacket()   ← Cek apakah ada paket (polling)
  └─ Jika ps > 0:
       ├─ Baca karakter: LoRa.read() sampai available() habis
       ├─ Cetak pesan ke Serial
       ├─ Cetak RSSI: LoRa.packetRssi()
       └─ Cetak SNR:  LoRa.packetSnr()
```

---

## Konfigurasi LoRa

| Parameter | Nilai |
|---|---|
| Frekuensi | **920 MHz** |
| Bandwidth | **125 kHz** |
| Spreading Factor | **SF7** |
| Coding Rate | **4/5** |
| TX Power | **17 dBm** (sender saja) |

> Konfigurasi **harus identik** antara sender dan receiver. Ketidakcocokan parameter menyebabkan paket tidak terbaca.

---

## Library yang Digunakan

| Library | Versi | Fungsi | Install |
|---|---|---|---|
| `LoRa` (sandeepmistry) | 0.8.x | Driver SX1276: init, TX blocking, RX polling, RSSI, SNR | `arduino-cli lib install "LoRa"` |
| `SPI.h` | bawaan Arduino | Komunikasi SPI hardware ke SX1276 | Sudah tersedia |

---

## Cara Penggunaan

### Persiapan

1. Pasang Dragino LoRa Shield v1.2 ke atas Arduino Uno
2. Pasang antena SMA pada **kedua** shield
3. Pastikan **R9 (0 ohm) terpasang** di shield (CS → D10)
4. Install library LoRa:

```bash
arduino-cli lib install "LoRa"
```

### Upload Receiver → COM9

```bash
arduino-cli compile --fqbn arduino:avr:uno receiver
arduino-cli upload -p COM9 --fqbn arduino:avr:uno receiver
```

### Upload Sender → COM8

```bash
arduino-cli compile --fqbn arduino:avr:uno sender
arduino-cli upload -p COM8 --fqbn arduino:avr:uno sender
```

Buka Serial Monitor pada kedua port (baud **9600**). Sender langsung mengirim begitu boot, receiver langsung mencetak paket yang diterima.

> **Tutup Serial Monitor sebelum upload** — port COM tidak bisa digunakan bersamaan oleh IDE dan upload.

---

## Contoh Output Serial Monitor

### Sender — COM8

```
=== LoRa SENDER ===
Init LoRa ... OK
Frekuensi : 920.00 MHz
SF=7, BW=125kHz, CR=4/5, Power=17dBm
Kirim tiap 2 detik...

[TX] "Hello LoRa #0" ... terkirim
[TX] "Hello LoRa #1" ... terkirim
[TX] "Hello LoRa #2" ... terkirim
```

### Receiver — COM9

```
=== LoRa RECEIVER ===
Init LoRa ... OK
Frekuensi : 920.00 MHz
SF=7, BW=125kHz, CR=4/5
Menunggu paket...

--- Paket Diterima ---
  Data  : "Hello LoRa #0"
  RSSI  : -52 dBm
  SNR   : 9.25 dB

--- Paket Diterima ---
  Data  : "Hello LoRa #1"
  RSSI  : -51 dBm
  SNR   : 9.50 dB
```

---

## Troubleshooting

| Masalah | Kemungkinan Penyebab | Solusi |
|---|---|---|
| `Init LoRa ... GAGAL!` | R9 tidak terpasang / shield tidak terpasang sempurna | Cek R9 (0 ohm), lepas-pasang shield |
| Sender cetak "terkirim" tapi receiver tidak mencetak apa-apa | Antena tidak terpasang | Pasang antena SMA pada kedua board |
| Receiver tidak mencetak apa-apa | Frekuensi atau parameter tidak cocok | Pastikan sender dan receiver pakai `920E6`, SF7, BW 125kHz |
| Upload error: `Access is denied` | Serial Monitor masih terbuka | Tutup Serial Monitor, upload ulang |
| Beberapa paket hilang | Jarak terlalu jauh atau interferensi | Perkecil jarak; naikkan SF (`setSpreadingFactor(9)`) untuk jangkauan lebih jauh |
