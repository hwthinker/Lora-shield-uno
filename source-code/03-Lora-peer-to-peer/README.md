# 03 — LoRa Peer-to-Peer (Ping-Pong Dua Arah)

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
- [Cara Kerja Detail](#cara-kerja-detail)
- [Format Payload](#format-payload)
- [Konfigurasi LoRa](#konfigurasi-lora)
- [Library yang Digunakan](#library-yang-digunakan)
- [Cara Penggunaan](#cara-penggunaan)
- [Contoh Output Serial Monitor](#contoh-output-serial-monitor)
- [Troubleshooting](#troubleshooting)

---

## Pendahuluan

Subproject `03-Lora-peer-to-peer` adalah implementasi komunikasi LoRa **dua arah (bidirectional)** antara dua board dalam pola Ping-Pong.

Terdapat **dua file terpisah** — satu per board, langsung upload tanpa konfigurasi manual:

| File | Board | Port | Peran |
|---|---|---|---|
| `03a-Lora-peer-to-peer.ino` | Arduino Uno + Shield A | COM8 | **INITIATOR** — memulai Ping |
| `03b-Lora-peer-to-peer.ino` | Arduino Uno + Shield B | COM9 | **RESPONDER** — membalas |

Fitur utama:
- TX **blocking** (`endPacket()`) — andal di AVR, tidak bergantung pada ISR TX-done
- RX **polling** via `LoRa.parsePacket()` — tidak perlu ISR, bebas race condition
- Device A **auto-retry** tiap 5 detik jika tidak ada balasan — sistem tidak stuck permanen

---

## Tujuan Program

- Memperagakan komunikasi LoRa dua arah antara dua node
- Membuktikan fungsionalitas TX dan RX pada board yang sama secara bergantian
- Mendemonstrasikan pola polling yang sederhana dan andal di AVR

---

## Hardware yang Digunakan

| Komponen | Detail |
|---|---|
| **Board utama** | Arduino Uno (ATmega328P) |
| **Shield LoRa** | Dragino LoRa Shield v1.2 |
| **Modul LoRa** | SX1276 (onboard shield) |
| **Frekuensi** | **433 MHz** |
| **LED** | LED built-in Arduino pada **D13** (shared dengan SPI SCK) |
| **Antena** | Antena LoRa eksternal via konektor SMA (wajib dipasang pada kedua board) |
| **Jumlah board** | **2 set** — masing-masing upload file yang berbeda |

---

## Pin Definition

| Sinyal | Arduino Pin | Keterangan |
|---|---|---|
| NSS / CS | **D10** | SPI Chip Select (R9 = 0 ohm terpasang) |
| DIO0 | **D2** | Digunakan `parsePacket()` secara internal |
| RST | **D9** | Reset SX1276 |
| SCK | **D13** | SPI Clock (juga LED built-in) |
| MOSI | **D11** | SPI MOSI |
| MISO | **D12** | SPI MISO |

> D13 shared dengan SPI SCK — LED berkedip saat SPI aktif. Pasang LED eksternal di **D3** untuk notifikasi bersih.

---

## Struktur File

```
03-Lora-peer-to-peer/
├── README.md
├── 03a-Lora-peer-to-peer/
│   └── 03a-Lora-peer-to-peer.ino   ← Upload ke COM8 (INITIATOR)
└── 03b-Lora-peer-to-peer/
    └── 03b-Lora-peer-to-peer.ino   ← Upload ke COM9 (RESPONDER)
```

---

## Topologi Komunikasi

```
┌──────────────────────────┐                      ┌──────────────────────────┐
│   DEVICE A (COM8)        │    LoRa 433 MHz       │   DEVICE B (COM9)        │
│   03a-...ino             │ ◄──────────────────► │   03b-...ino             │
│   Peran: INITIATOR       │                      │   Peran: RESPONDER       │
│   Boot → kirim Ping      │                      │   Boot → tunggu RX       │
│   Auto-retry tiap 5 dtk  │                      │   Balas setiap ada paket │
└──────────────────────────┘                      └──────────────────────────┘
```

- **Topologi:** Point-to-Point (satu lawan satu)
- **Mode:** Half-duplex, turn-based
- **Recovery:** Device A kirim ulang Ping otomatis jika tidak ada balasan dalam 5 detik

---

## Alur Kerja Program

### Device A — Initiator (`03a`)

```
setup()
  ├─ Init Serial, LED, LoRa
  ├─ delay(1000)              ← beri Device B waktu siap
  └─ transmit("DeviceA:Ping") ← TX blocking, mulai percakapan

loop()
  ├─ Jika sudah > 5 detik tanpa balasan:
  │    └─ transmit("DeviceA:Ping")   ← auto-retry
  │
  └─ LoRa.parsePacket()
       └─ Jika ada paket:
            ├─ Baca + cetak (pesan, RSSI, SNR)
            ├─ delay(50)
            └─ transmit("DeviceA:Pong/Ping")
```

### Device B — Responder (`03b`)

```
setup()
  └─ Init Serial, LED, LoRa   ← langsung siap terima

loop()
  └─ LoRa.parsePacket()
       └─ Jika ada paket:
            ├─ Baca + cetak (pesan, RSSI, SNR)
            ├─ delay(50)
            └─ transmit("DeviceB:Pong/Ping")
```

---

## Cara Kerja Detail

### TX — Blocking

```cpp
void transmit(const String& msg) {
  LoRa.beginPacket();
  LoRa.print(msg);
  LoRa.endPacket();  // tunggu sampai TX selesai, baru return
}
```

- Setelah `endPacket()` return, SX1276 otomatis kembali ke STANDBY
- Tidak perlu ISR TX-done — lebih sederhana dan andal di AVR

### RX — Polling `parsePacket()`

```cpp
int ps = LoRa.parsePacket();
if (ps > 0) { /* ada paket */ }
```

- Setiap panggilan: cek IRQ register, jika ada paket kembalikan ukurannya
- Jika tidak ada, radio masuk mode RX_SINGLE dan coba lagi di iterasi berikutnya
- Tidak perlu interrupt, tidak ada race condition, tidak perlu `volatile` flag

### Logika Balas

```cpp
// Terima "Ping" → balas "Pong", terima "Pong" → balas "Ping"
String reply = "DeviceA:" + String(received.indexOf("Ping") >= 0 ? "Pong" : "Ping");
```

### Auto-Retry (Device A saja)

```cpp
if (millis() - lastTxMs > PING_INTERVAL) {
  transmit("DeviceA:Ping");  // kirim ulang jika tidak ada balasan
  lastTxMs = millis();
}
```

---

## Format Payload

| Arah | Format | Contoh |
|---|---|---|
| Device A → Device B | `DeviceA:Ping` atau `DeviceA:Pong` | `DeviceA:Ping` |
| Device B → Device A | `DeviceB:Pong` atau `DeviceB:Ping` | `DeviceB:Pong` |

---

## Konfigurasi LoRa

| Parameter | Nilai |
|---|---|
| Frekuensi | **433 MHz** |
| Bandwidth | **125 kHz** |
| Spreading Factor | **SF7** |
| Coding Rate | **4/5** |
| TX Power | **17 dBm** |

> Konfigurasi **harus identik** pada kedua board.

---

## Library yang Digunakan

| Library | Versi | Fungsi |
|---|---|---|
| `LoRa` (sandeepmistry) | 0.8.x | Driver SX1276: TX, RX polling, RSSI, SNR |
| `SPI.h` | bawaan Arduino | Komunikasi SPI hardware ke SX1276 |

```bash
arduino-cli lib install "LoRa"
```

---

## Cara Penggunaan

> **Upload Device B dulu**, baru Device A — agar Device B sudah siap menerima ketika Device A kirim Ping pertama.

### Upload Device B → COM9

```bash
arduino-cli compile --fqbn arduino:avr:uno 03b-Lora-peer-to-peer
arduino-cli upload -p COM9 --fqbn arduino:avr:uno 03b-Lora-peer-to-peer
```

### Upload Device A → COM8

```bash
arduino-cli compile --fqbn arduino:avr:uno 03a-Lora-peer-to-peer
arduino-cli upload -p COM8 --fqbn arduino:avr:uno 03a-Lora-peer-to-peer
```

Buka Serial Monitor kedua port (baud **9600**). Ping-Pong akan berjalan otomatis.

> Tutup Serial Monitor sebelum upload — port COM tidak bisa digunakan bersamaan.

---

## Contoh Output Serial Monitor

### Device A — COM8

```
=== DEVICE A — INITIATOR ===
Init LoRa ... OK
Freq: 433.00 MHz

[TX] DeviceA:Ping

================================
[RX] Pesan  : DeviceB:Pong
[RX] RSSI   : -52 dBm
[RX] SNR    : 8.25 dB
================================
[TX] DeviceA:Ping

================================
[RX] Pesan  : DeviceB:Pong
...
```

### Device B — COM9

```
=== DEVICE B — RESPONDER ===
Init LoRa ... OK
Freq: 433.00 MHz
Menunggu paket dari Device A...

================================
[RX] Pesan  : DeviceA:Ping
[RX] RSSI   : -50 dBm
[RX] SNR    : 9.00 dB
================================
[TX] DeviceB:Pong

================================
[RX] Pesan  : DeviceA:Ping
...
```

Jika Device A tidak menerima balasan dalam 5 detik:
```
[RETRY] Tidak ada balasan, kirim ulang Ping...
[TX] DeviceA:Ping
```

---

## Troubleshooting

| Masalah | Kemungkinan Penyebab | Solusi |
|---|---|---|
| `Init LoRa ... GAGAL!` | R9 tidak terpasang / shield tidak terpasang sempurna | Cek R9 (0 ohm), lepas-pasang shield |
| Device A terus `[RETRY]`, Device B tidak cetak apa-apa | Antena tidak terpasang / jarak terlalu jauh | Pasang antena, perkecil jarak |
| Ping-Pong berjalan lalu berhenti | Paket hilang sementara | Device A akan auto-retry dalam 5 detik |
| LED tidak berkedip | D13 shared SPI SCK | Normal; pasang LED eksternal di D3 jika perlu |
| Upload error: `Access is denied` | Serial Monitor masih terbuka di port tersebut | Tutup Serial Monitor, upload ulang |
