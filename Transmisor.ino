#include "LoRaWan_APP.h"
#include "Arduino.h"

#define RF_FREQUENCY 915000000  // 915 MHz para América
#define TX_OUTPUT_POWER 20
#define LORA_BANDWIDTH 0        // 125 kHz
#define LORA_SPREADING_FACTOR 7
#define LORA_CODINGRATE 1
#define LORA_PREAMBLE_LENGTH 8
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false

static RadioEvents_t RadioEvents;
char txpacket[64];

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Iniciando CubeCell TX...");

  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);
}

void loop() {
  String msg = "lux:123,temp:25,hum:60";
  msg.toCharArray(txpacket, 64);

  Serial.println("Enviando: " + msg);
  Radio.Send((uint8_t*)txpacket, strlen(txpacket));
  
  delay(5000); // cada 5 segundos
}
