## 📖 OUTLINE BUKU: "LoRa Master-Slave untuk Pemula"

### Judul Lengkap

**Komunikasi LoRa Multi-Node dengan Arduino Uno & Dragino Shield: Dari Polling Sederhana hingga Keamanan Data**

### Target Pembaca

- Mahasiswa teknik elektro/informatika
- Hobiis IoT dan embedded systems
- Peneliti yang membutuhkan komunikasi jarak jauh dengan biaya rendah

### Level Kesulitan

Pemula → Menengah (diasumsikan sudah tahu dasar Arduino)

------

## Daftar Isi (Outline Lengkap)

### Bagian 1: Dasar Teori (3 Bab)

#### Bab 1: Pengantar LoRa dan SX1276

| Sub-bab                                        | Halaman Estimasi | Poin Penting                                       |
| :--------------------------------------------- | :--------------- | :------------------------------------------------- |
| 1.1 Apa itu LoRa?                              | 4                | Chirp Spread Spectrum, jangkauan vs bandwidth      |
| 1.2 Perbandingan LoRa dengan WiFi, BLE, ZigBee | 3                | Tabel perbandingan jarak, kecepatan, konsumsi daya |
| 1.3 Arsitektur SX1276                          | 5                | Blok diagram: FIFO, IRQ, PA, LNA                   |
| 1.4 Mode Operasi                               | 4                | Sleep, Standby, TX, RX, CAD                        |
| 1.5 Parameter LoRa                             | 6                | SF, BW, CR, CR – hubungan dengan airtime           |

#### Bab 2: Hardware Dragino LoRa Shield v1.2

| Sub-bab                    | Halaman | Poin Penting                     |
| :------------------------- | :------ | :------------------------------- |
| 2.1 Pin Mapping dan Jumper | 4       | R9 ke D10, DIO0 ke D2, RST ke D9 |
| 2.2 Antena dan Impedansi   | 3       | Jangan hidupkan tanpa antena!    |
| 2.3 Power Management       | 3       | 3.3V vs 5V, konsumsi arus TX/RX  |
| 2.4 LED Indikator          | 2       | D13 sharing dengan SPI SCK       |

#### Bab 3: Instalasi dan Persiapan

| Sub-bab                                  | Halaman | Poin Penting                      |
| :--------------------------------------- | :------ | :-------------------------------- |
| 3.1 Install Arduino IDE                  | 2       | Download, install driver USB      |
| 3.2 Install Library LoRa (sandeepmistry) | 2       | `arduino-cli lib install "LoRa"`  |
| 3.3 Verifikasi Shield                    | 3       | Program scan I2C/SPI, cek koneksi |
| 3.4 Mengenal COM Port                    | 2       | Windows vs Linux vs Mac           |

------

### Bagian 2: Komunikasi Dasar (4 Bab)

#### Bab 4: Single Node – LoRa UART Loopback

| Sub-bab                             | Halaman | Poin Penting                                       |
| :---------------------------------- | :------ | :------------------------------------------------- |
| 4.1 Inisialisasi LoRa               | 4       | `LoRa.begin()`, `LoRa.setPins()`                   |
| 4.2 TX: Mengirim Data               | 5       | `beginPacket()`, `print()`, `endPacket()` blocking |
| 4.3 RX: Menerima Data               | 5       | `parsePacket()`, `available()`, `read()`           |
| 4.4 Program Lengkap Sender-Receiver | 6       | Kode lengkap dengan penjelasan baris per baris     |
| 4.5 Eksperimen: Jarak vs RSSI       | 4       | Tabel hasil pengukuran                             |

**Kode Program yang Disertakan:**

- `01-sender.ino`
- `01-receiver.ino`

#### Bab 5: Peer-to-Peer 2 Node

| Sub-bab                         | Halaman | Poin Penting                                 |
| :------------------------------ | :------ | :------------------------------------------- |
| 5.1 Topologi Peer-to-Peer       | 3       | Tidak ada master, kedua node bisa TX/RX      |
| 5.2 Masalah Collision           | 4       | Apa yang terjadi jika dua node TX bersamaan  |
| 5.3 Simple ALOHA                | 5       | Kirim, tunggu random, kirim ulang jika gagal |
| 5.4 Program 2-Way Communication | 6       | Kode dengan tombol dan LED notifikasi        |

**Kode Program:**

- `02-peer-to-peer-nodeA.ino`
- `02-peer-to-peer-nodeB.ino`

#### Bab 6: Master-Slave 3 Node (Program Anda)

| Sub-bab                    | Halaman | Poin Penting                        |
| :------------------------- | :------ | :---------------------------------- |
| 6.1 Mengapa Master-Slave?  | 3       | Tidak ada collision, deterministik  |
| 6.2 Round-Robin Polling    | 4       | Algoritma giliran                   |
| 6.3 Addressing dan ID      | 3       | Slave hanya respon ke ID sendiri    |
| 6.4 Timeout Handling       | 4       | Mendeteksi slave mati               |
| 6.5 Program Lengkap 3 Node | 8       | master.ino, slave1.ino, slave2.ino  |
| 6.6 Analisis Timing        | 4       | Hitung airtime dan delay per siklus |

**Kode Program:** (Sudah Anda miliki)

#### Bab 7: Ekspansi ke N Node

| Sub-bab                       | Halaman | Poin Penting                                    |
| :---------------------------- | :------ | :---------------------------------------------- |
| 7.1 Membuat Slave Generic     | 4       | Menggunakan `#define SLAVE_ID`                  |
| 7.2 Master dengan Array Slave | 5       | Loop untuk N slave, konfigurasi dinamis         |
| 7.3 Batasan Jumlah Node       | 4       | 10 node → siklus 1.3 detik, 20 node → 2.6 detik |
| 7.4 Optimasi Waktu            | 4       | Kurangi delay, pakai SF yang lebih kecil        |

**Kode Program:**

- `master-n-node.ino`
- `slave-generic.ino`

------

### Bagian 3: Keandalan dan Deteksi Error (3 Bab)

#### Bab 8: CRC dan Deteksi Error

| Sub-bab                               | Halaman | Poin Penting                                |
| :------------------------------------ | :------ | :------------------------------------------ |
| 8.1 Teori CRC                         | 4       | Cyclic Redundancy Check, polynomial         |
| 8.2 CRC di SX1276                     | 5       | Hardware CRC, register IRQ_FLAGS            |
| 8.3 Mendeteksi CRC Error              | 4       | Baca `REG_IRQ_FLAGS` untuk tahu paket rusak |
| 8.4 Statistik Packet Error Rate (PER) | 3       | Hitung persentase paket hilang/corrupt      |

**Kode Program:**

- `03-crc-detector.ino` (baru perlu dibuat)
- `03-per-statistic.ino`

#### Bab 9: Acknowledgment dan Retransmission

| Sub-bab                        | Halaman | Poin Penting                                   |
| :----------------------------- | :------ | :--------------------------------------------- |
| 9.1 Jenis ACK                  | 3       | ACK kosong, ACK dengan data, ACK dengan status |
| 9.2 Implementasi ACK Eksplisit | 6       | Master: POLL → Slave: ACK → Master: REQ_DATA   |
| 9.3 Retransmission Strategy    | 5       | Maks 3 kali retry, exponential backoff         |
| 9.4 Timeout Adaptif            | 4       | Sesuaikan timeout dengan jarak/RSSI            |

**Kode Program:**

- `04-master-ack.ino`
- `04-slave-ack.ino`

#### Bab 10: Menghitung dan Mengoptimasi Timing

| Sub-bab                                  | Halaman | Poin Penting                                              |
| :--------------------------------------- | :------ | :-------------------------------------------------------- |
| 10.1 Rumus Airtime LoRa                  | 6       | Perhitungan lengkap dengan contoh                         |
| 10.2 Tabel Airtime (SF7-SF12, BW125-500) | 4       | Tabel referensi cepat                                     |
| 10.3 Menentukan Timeout Optimal          | 4       | 2× airtime + processing overhead                          |
| 10.4 Menentukan CYCLE_INTERVAL           | 4       | Rumus: `(target_update_rate / N_slave) - total_slot_time` |

**Kode Program:**

- `05-timing-calculator.ino` (calculator via Serial Monitor)

------

### Bagian 4: Keamanan (4 Bab)

#### Bab 11: Ancaman pada Komunikasi LoRa

| Sub-bab                    | Halaman | Poin Penting                            |
| :------------------------- | :------ | :-------------------------------------- |
| 11.1 Snooping (Mendengar)  | 3       | Siapa pun di frekuensi sama bisa dengar |
| 11.2 Spoofing (Memalsukan) | 3       | Hacker kirim `POLL:1` palsu             |
| 11.3 Replay Attack         | 4       | Rekam lalu ulang pesan yang valid       |
| 11.4 Jamming (Macetkan)    | 3       | Kirim noise di frekuensi yang sama      |

#### Bab 12: Network ID dan Address Filtering

| Sub-bab                                 | Halaman | Poin Penting                     |
| :-------------------------------------- | :------ | :------------------------------- |
| 12.1 Keterbatasan Address Filtering     | 3       | Hacker bisa sniff ID lalu pakai  |
| 12.2 Implementasi Network ID            | 4       | Format `"NET123:POLL:1"`         |
| 12.3 Multi-network dalam Satu Frekuensi | 4       | ID berbeda → tidak saling ganggu |

**Kode Program:**

- `06-master-network-id.ino`
- `06-slave-network-id.ino`

#### Bab 13: Rolling Code dan Autentikasi

| Sub-bab                      | Halaman | Poin Penting                                  |
| :--------------------------- | :------ | :-------------------------------------------- |
| 13.1 Konsep Rolling Code     | 4       | Seperti remote mobil, counter naik tiap kirim |
| 13.2 Implementasi dengan XOR | 5       | `hash = counter XOR secret_key`               |
| 13.3 Sinkronisasi Counter    | 5       | Jika slave restart? Master dan slave resync   |
| 13.4 Kelemahan Rolling Code  | 3       | Masih bisa di-replay dalam window kecil       |

**Kode Program:**

- `07-master-rolling.ino`
- `07-slave-rolling.ino`

#### Bab 14: Enkripsi Sederhana (XOR/AES)

| Sub-bab                          | Halaman | Poin Penting                            |
| :------------------------------- | :------ | :-------------------------------------- |
| 14.1 XOR Cipher                  | 4       | `cipher = plain XOR key`                |
| 14.2 Implementasi XOR di Arduino | 4       | Key 16 byte, loop per karakter          |
| 14.3 Pengenalan AES-128          | 5       | Library `AES.h`, ukuran 16 byte block   |
| 14.4 AES dengan LoRa             | 5       | Enkripsi payload sebelum `LoRa.print()` |

**Kode Program:**

- `08-master-xor.ino`
- `08-slave-xor.ino`
- `09-master-aes.ino` (perlu library)

------

### Bagian 5: Aplikasi dan Proyek (3 Bab)

#### Bab 15: Sensor Monitoring (DHT11 + LoRa)

| Sub-bab                    | Halaman | Poin Penting                          |
| :------------------------- | :------ | :------------------------------------ |
| 15.1 Wiring DHT11 ke Slave | 3       | Pin data ke D4, pull-up resistor      |
| 15.2 Format Payload Sensor | 4       | `"S1:TEMP:25.3,HUM:60.2"`             |
| 15.3 Master sebagai Logger | 4       | Tampilkan ke Serial atau simpan ke SD |

**Kode Program:**

- `10-slave-dht11.ino`
- `10-master-logger.ino`

#### Bab 16: Dynamic Time Slot (Slave Request)

| Sub-bab                      | Halaman | Poin Penting                            |
| :--------------------------- | :------ | :-------------------------------------- |
| 16.1 Konsep Request-Response | 4       | Slave: `"S1:NEED_TIME:1000"`            |
| 16.2 Master Scheduler        | 5       | Array waktu per slave, beri slot ekstra |
| 16.3 Prioritas Data Penting  | 4       | Alarm, emergency → potong giliran       |

**Kode Program:**

- `11-master-dynamic.ino`
- `11-slave-dynamic.ino`

#### Bab 17: Proyek Final – Sistem Monitoring 10 Node

| Sub-bab                       | Halaman | Poin Penting                          |
| :---------------------------- | :------ | :------------------------------------ |
| 17.1 Desain Topologi          | 4       | 1 Master + 10 Slave, jarak 500m       |
| 17.2 Konfigurasi Parameter    | 3       | SF9, BW125, CR4/5 untuk jangkauan     |
| 17.3 Perhitungan Kapasitas    | 4       | 10 slave × 100ms ≈ 1 detik per siklus |
| 17.4 Troubleshooting Lapangan | 5       | Masalah umum dan solusi               |

**Kode Program:**

- `final-master.ino`
- `final-slave.ino`

------

### Bagian 6: Lampiran (4 Lampiran)

#### Lampiran A: Tabel Referensi Cepat

| Isi                                         | Halaman |
| :------------------------------------------ | :------ |
| A.1 Pin Mapping Dragino Shield              | 1       |
| A.2 Tabel Airtime (SF7-SF12, BW125/250/500) | 2       |
| A.3 Kode Error dan Solusi                   | 1       |
| A.4 Perintah Arduino-CLI untuk Upload       | 1       |

#### Lampiran B: Library yang Digunakan

| Library               | Fungsi         | Link Download           |
| :-------------------- | :------------- | :---------------------- |
| LoRa by sandeepmistry | Driver SX1276  | GitHub                  |
| SPI.h                 | Komunikasi SPI | Bawaan Arduino          |
| AES.h (opsional)      | Enkripsi       | Arduino Library Manager |

#### Lampiran C: Glossary Istilah

| Istilah               | Definisi |
| :-------------------- | :------- |
| SF (Spreading Factor) | ...      |
| BW (Bandwidth)        | ...      |
| Airtime               | ...      |

#### Lampiran D: Troubleshooting

| Masalah            | Solusi                        |
| :----------------- | :---------------------------- |
| LoRa.begin() gagal | Cek jumper R9                 |
| Tidak terima paket | Cek antena, jarak, SF/BW sama |
| Upload gagal       | Tutup Serial Monitor          |

------

## 📊 Ringkasan Statistik Buku

| Bagian                     | Jumlah Bab | Estimasi Halaman |
| :------------------------- | :--------- | :--------------- |
| Bagian 1: Dasar Teori      | 3          | 36               |
| Bagian 2: Komunikasi Dasar | 4          | 56               |
| Bagian 3: Keandalan        | 3          | 42               |
| Bagian 4: Keamanan         | 4          | 56               |
| Bagian 5: Aplikasi         | 3          | 38               |
| Bagian 6: Lampiran         | 4          | 20               |
| **Total**                  | **21 Bab** | **~248 halaman** |

------

## 🎯 Program yang Perlu Dibuat (Selain yang Sudah Anda Miliki)

| No   | Nama Program                             | Status         | Bab  |
| :--- | :--------------------------------------- | :------------- | :--- |
| 1    | `01-sender.ino`, `01-receiver.ino`       | ❌ Perlu dibuat | 4    |
| 2    | `02-peer-to-peer-*.ino`                  | ❌ Perlu dibuat | 5    |
| 3    | `master-n-node.ino`, `slave-generic.ino` | ❌ Perlu dibuat | 7    |
| 4    | `03-crc-detector.ino`                    | ❌ Perlu dibuat | 8    |
| 5    | `04-master-ack.ino`, `04-slave-ack.ino`  | ❌ Perlu dibuat | 9    |
| 6    | `05-timing-calculator.ino`               | ❌ Perlu dibuat | 10   |
| 7    | `06-master-network-id.ino`               | ❌ Perlu dibuat | 12   |
| 8    | `07-master-rolling.ino`                  | ❌ Perlu dibuat | 13   |
| 9    | `08-master-xor.ino`                      | ❌ Perlu dibuat | 14   |
| 10   | `10-slave-dht11.ino`                     | ❌ Perlu dibuat | 15   |
| 11   | `11-master-dynamic.ino`                  | ❌ Perlu dibuat | 16   |
| 12   | `final-master.ino`                       | ❌ Perlu dibuat | 17   |

**Total program baru yang perlu dibuat: 20+ file `.ino`**

------

## 💬 Rekomendasi Saya untuk Anda

### Jika Anda Ingin Menulis Buku Ini:

1. **Mulai dari program yang sudah Anda miliki** (Bab 6) sebagai inti buku
2. **Buat program pendukung** untuk Bab 4, 5, 7 dulu (yang paling mendasar)
3. **Tambahkan keamanan** (Bab 11-14) sebagai bab lanjutan yang membedakan buku ini dari buku LoRa lain
4. **Sertakan screenshot** Serial Monitor untuk setiap program
5. **Sertakan foto** wiring dan setup hardware

### Prioritas Pembuatan Program (Rekomendasi Urutan):

text

```
Minggu 1: Bab 4 (Sender-Receiver)
Minggu 2: Bab 5 (Peer-to-peer)
Minggu 3: Bab 7 (N node) ← dari program Anda yang sudah ada
Minggu 4: Bab 8 (CRC) + Bab 9 (ACK)
Minggu 5: Bab 12 (Network ID) + Bab 13 (Rolling)
Minggu 6: Bab 15 (Sensor) + Bab 16 (Dynamic slot)
```