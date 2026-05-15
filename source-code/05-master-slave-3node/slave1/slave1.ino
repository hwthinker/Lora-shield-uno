/*
  LoRa Master-Slave 3 Node - Dragino LoRa Shield v1.2 + Arduino Uno (ATmega328P)
  Library : LoRa by sandeepmistry v0.8.x
  Upload to: COM9 (SLAVE 1)

  Slave 1 hanya merespon ketika Master mengirim "POLL:1".
  Balasan: "S1:DATA:N" — N adalah counter lokal yang naik setiap kali dipoll.

  Mekanisme:
    TX : blocking (endPacket) — sederhana dan andal di AVR
    RX : polling via LoRa.parsePacket() — tidak perlu ISR

  Pin Mapping Dragino Shield v1.2:
    NSS/CS -> D10, DIO0 -> D2, RST -> D9
    SCK -> D13, MOSI -> D11, MISO -> D12
*/

#include <SPI.h>
#include <LoRa.h>

#define NSS_PIN          10
#define RST_PIN           9
#define DIO0_PIN          2
#define LED_PIN          LED_BUILTIN

#define FREQUENCY        433E6
#define BANDWIDTH        125E3
#define SPREADING_FACTOR 7
#define CODING_RATE      5
#define TX_POWER         17

#define SLAVE_ID         1

int dataCounter = 0;
int rxCount = 0;

void transmit(const String& msg) {
  LoRa.beginPacket();
  LoRa.print(msg);
  LoRa.endPacket();
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.print(F("=== LoRa SLAVE "));
  Serial.print(SLAVE_ID);
  Serial.println(F(" ==="));
  Serial.print(F("Init LoRa ... "));

  LoRa.setPins(NSS_PIN, RST_PIN, DIO0_PIN);
  if (!LoRa.begin(FREQUENCY)) {
    Serial.println(F("GAGAL! Cek shield/kabel."));
    while (true);
  }

  LoRa.setSpreadingFactor(SPREADING_FACTOR);
  LoRa.setSignalBandwidth(BANDWIDTH);
  LoRa.setCodingRate4(CODING_RATE);
  LoRa.setTxPower(TX_POWER);

  Serial.println(F("OK"));
  Serial.print(F("Freq: ")); Serial.print(FREQUENCY / 1E6); Serial.println(F(" MHz"));
  Serial.println(F("Menunggu POLL:1 dari Master...\n"));
}

void loop() {
  int ps = LoRa.parsePacket();
  if (ps == 0) return;

  String received = "";
  while (LoRa.available()) {
    received += (char)LoRa.read();
  }

  if (!received.equals("POLL:" + String(SLAVE_ID))) {
    Serial.print(F("[IGNORE] ")); Serial.println(received);
    return;
  }

  rxCount++;
  dataCounter++;
  digitalWrite(LED_PIN, HIGH);

  Serial.print(F("[RX] ")); Serial.print(received);
  Serial.print(F(" | RSSI: ")); Serial.print(LoRa.packetRssi());
  Serial.print(F(" dBm | SNR: ")); Serial.print(LoRa.packetSnr());
  Serial.print(F(" dB | RX#: ")); Serial.println(rxCount);

  String reply = "S" + String(SLAVE_ID) + ":DATA:" + String(dataCounter);
  Serial.print(F("[TX] ")); Serial.println(reply);

  transmit(reply);
  digitalWrite(LED_PIN, LOW);
  Serial.println();
}
