# 04 — LoRa dengan Sistem ACK (Acknowledgement)

> **Target Hardware:** Dragino LoRa Shield v1.2 &nbsp;·&nbsp; MCU: Arduino Uno (ATmega328P) &nbsp;·&nbsp; LoRa: SX1276

[← Kembali ke README Utama](../../README.md)

---

## Tujuan Program

Implementasi komunikasi LoRa dengan mekanisme **konfirmasi pengiriman (ACK)**. Sender mengirim data dan menunggu balasan ACK dari receiver. Jika ACK tidak diterima dalam batas waktu (3 detik), transmisi dianggap gagal.

---

## Hardware yang Digunakan

| Komponen | Detail |
|---|---|
| **Board** | Arduino Uno (ATmega328P) |
| **Shield** | Dragino LoRa Shield v1.2 |
| **LoRa Chip** | SX1276 |
| **Frekuensi** | 433 MHz |
| **LED** | LED built-in D13 — menyala singkat saat ACK diterima / paket RX |

---

## Struktur File

```
04-lora-ack/
├── README.md
├── sender-ack/
│   └── sender-ack.ino     ← Upload ke COM8 — kirim DATA, tunggu ACK
└── receiver-ack/
    └── receiver-ack.ino   ← Upload ke COM9 — terima DATA, balas ACK
```

---

## Pin Definition

| Sinyal | Arduino Pin | Keterangan |
|---|---|---|
| NSS / CS | **D10** | SPI Chip Select (R9 0-ohm terpasang) |
| DIO0 | **D2** | Interrupt RX done (INT0) |
| RST | **D9** | Reset SX1276 |
| SCK | **D13** | SPI Clock (juga LED built-in) |
| MOSI | **D11** | SPI MOSI |
| MISO | **D12** | SPI MISO |

---

## Library

| Library | Fungsi | Install via |
|---|---|---|
| `LoRa` (sandeepmistry) | Driver SX1276: TX, RX, interrupt | `arduino-cli lib install "LoRa"` |
| `SPI.h` | SPI hardware | Bawaan Arduino |

---

## Parameter LoRa

| Parameter | Nilai |
|---|---|
| Frekuensi | **433 MHz** |
| Spreading Factor | **SF7** |
| Bandwidth | **125 kHz** |
| Coding Rate | **4/5** |
| TX Power | **17 dBm** |
| ACK Timeout | **3000 ms** |

> Parameter harus **identik** antara sender dan receiver.

---

## Format Payload

| Arah | Format | Contoh |
|---|---|---|
| Sender → Receiver | `DATA:[nomor]` | `DATA:42` |
| Receiver → Sender | `ACK:[nomor]` | `ACK:42` |

Nomor pada ACK harus cocok dengan nomor DATA untuk dianggap valid.

---

## Topologi Komunikasi

```
[SENDER — COM8]                        [RECEIVER — COM9]
      │                                       │
      ├─── DATA:N ──────────────────────────►│
      │                                       ├── filter: harus "DATA:"
      │                                       ├── LED nyala, cetak info
      │                                       ├── TX ACK:N (blocking)
      │◄──────────────────────── ACK:N ───────┤
      │                                       ├── kembali ke RX
      ├── cocok? YES → ackOK++, LED nyala    │
      └── cocok? NO / timeout → ackFail++    │
```

---

## Cara Kerja Program

### Sender (`sender-ack.ino`)

```
setup():
  LoRa.onReceive(onAckReceived)  ← ISR untuk terima ACK

loop():
  1. TX blocking: LoRa.endPacket()       ← kirim "DATA:N", tunggu selesai
  2. LoRa.receive()                      ← masuk RX mode, tunggu ACK
  3. Polling ackFlag max 3000 ms:
       - Jika ackFlag: baca reply, cek cocok "ACK:N"
       - Cocok → gotACK = true, break
       - Tidak cocok → tetap tunggu sampai timeout
  4. Evaluasi:
       - gotACK → ackOK++, LED blink
       - Timeout → ackFail++
  5. counter++, delay 3000 ms
```

### Receiver (`receiver-ack.ino`)

```
setup():
  LoRa.onReceive(onReceive)    ← ISR saat paket tiba
  LoRa.receive()               ← masuk RX mode

loop():
  1. Jika rxFlag:
       a. Baca paket via LoRa.read()
       b. Filter: abaikan jika bukan "DATA:"
       c. Cetak data + RSSI + SNR ke Serial
       d. LED blink
       e. TX blocking: kirim "ACK:N"     ← LoRa.endPacket()
       f. LoRa.receive()                 ← kembali RX
```

---

## Kenapa TX Blocking?

Program menggunakan `LoRa.endPacket()` (blocking) bukan async, karena:

- Pada AVR (ATmega328P), async TX bergantung pada ISR `onTxDone` yang berbagi DIO0 dengan ISR `onReceive`
- Dua `attachInterrupt` bergantian pada pin yang sama dapat menyebabkan race condition
- TX blocking lebih deterministik: setelah `endPacket()` return, TX pasti sudah selesai dan radio langsung bisa masuk RX
- Durasi TX pendek (~20–30 ms untuk SF7/125 kHz) — blocking di sini tidak merugikan

---

## Contoh Output Serial Monitor

### Sender (COM8, 9600 baud)

```
=== LoRa ACK SENDER ===
Init LoRa ... OK
Freq: 433.00 MHz | SF7 | ACK timeout: 3000 ms

[TX] Kirim: DATA:0 ... selesai
[RX] Balasan: ACK:0
[OK] ACK diterima! | OK: 1 | FAIL: 0

[TX] Kirim: DATA:1 ... selesai
[FAIL] Tidak ada ACK! | OK: 1 | FAIL: 1
```

### Receiver (COM9, 9600 baud)

```
=== LoRa ACK RECEIVER ===
Init LoRa ... OK
Freq: 433.00 MHz
Menunggu paket DATA...

=== PAKET DITERIMA ===
  Data  : DATA:0
  RSSI  : -48 dBm
  SNR   : 9.50 dB
  Total : 1
=====================
[TX] ACK: ACK:0
[RX] Menunggu paket berikutnya...
```

---

## Cara Upload

```bash
# Receiver dulu (agar sudah siap di RX mode saat sender mulai)
arduino-cli compile --fqbn arduino:avr:uno receiver-ack
arduino-cli upload -p COM9 --fqbn arduino:avr:uno receiver-ack

# Lalu sender
arduino-cli compile --fqbn arduino:avr:uno sender-ack
arduino-cli upload -p COM8 --fqbn arduino:avr:uno sender-ack
```

> Tutup Serial Monitor sebelum upload.

---

## Troubleshooting

| Gejala | Kemungkinan Penyebab | Solusi |
|---|---|---|
| Selalu `NO ACK` | Jarak terlalu jauh / frekuensi tidak cocok | Perkecil jarak, pastikan kedua pakai `433E6` |
| `Init LoRa ... GAGAL` | R9 shield tidak terpasang / SPI error | Periksa jumper R9, lepas-pasang shield |
| Receiver tidak balas | `LoRa.receive()` tidak dipanggil di setup | Pastikan ada di `setup()` |
| ACK salah nomor | Ada paket sisa / ghost packet | Normal jika ada interferensi — sender akan tunggu dan retry cycle berikutnya |
| Upload error `Access is denied` | Serial Monitor masih terbuka | Tutup Serial Monitor, upload ulang |
