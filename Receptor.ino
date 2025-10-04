#include <WiFi.h>
#include <PubSubClient.h>
#include <LoRa.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// -----------------------------
// WiFi y MQTT
// -----------------------------
const char* ssid = "Claro_FliaOcampo";
const char* password = "0201457504";
const char* server = "192.168.100.19";
const int port = 1883;
const char *MQTT_CLIENT_NAME = "LoRa";
const char *mqtt_user = "esp8266_user";
const char *mqtt_pass = "P0rG4R4Pi!";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// -----------------------------
// LoRa
// -----------------------------
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26
#define RF_FREQUENCY 915E6

// -----------------------------
// OLED
// -----------------------------
#define ANCHOPANTALLA 128
#define ALTOPANTALLA 64
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16

Adafruit_SSD1306 display(ANCHOPANTALLA, ALTOPANTALLA, &Wire, OLED_RST);

// -----------------------------
// Variables de sensores
// -----------------------------
String lastLux = "--";
String lastTemp = "--";
String lastHum = "--";

// -----------------------------
// Funciones WiFi y MQTT
// -----------------------------
void wifiInit() {
  WiFi.begin(ssid, password);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Conectando WiFi...");
  display.display();

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi conectado, IP: " + WiFi.localIP().toString());

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("WiFi Conectado");
  display.setCursor(0,16);
  display.println(WiFi.localIP().toString());
  display.display();
}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Intentando conectarse MQTT...");
    if (mqttClient.connect(MQTT_CLIENT_NAME, mqtt_user, mqtt_pass)) {
      Serial.println("Conectado");
      display.clearDisplay();
      display.setCursor(0,0);
      display.println("MQTT Conectado");
      display.display();
    } else {
      Serial.println("Fallo, rc=" + String(mqttClient.state()) + " intentar de nuevo en 5seg");
      delay(5000);
    }
  }
}

// -----------------------------
// Setup
// -----------------------------
void setup() {
  Serial.begin(115200);

  // Inicializar pantalla OLED
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) {
    Serial.println("Fallo iniciando SSD1306");
    while(1);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Iniciando Gateway LoRa...");
  display.display();

  // WiFi y MQTT
  wifiInit();
  mqttClient.setServer(server, port);

  // Inicializar LoRa
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(RF_FREQUENCY)) {
    Serial.println("Error iniciando LoRa");
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Error iniciando LoRa");
    display.display();
    while(1);
  }
  Serial.println("Gateway LoRa iniciado");
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Gateway LoRa listo");
  display.display();
}

// -----------------------------
// Loop
// -----------------------------
void loop() {
  if (!mqttClient.connected()) reconnect();
  mqttClient.loop();

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String received = "";
    while (LoRa.available()) {
      received += (char)LoRa.read();
    }
    Serial.println("Recibido: " + received);

    // Separar valores: lux,temp,hum
    int firstComma = received.indexOf(',');
    int secondComma = received.indexOf(',', firstComma + 1);

    if (firstComma > 0 && secondComma > firstComma) {
      lastLux = received.substring(0, firstComma);
      lastTemp = received.substring(firstComma + 1, secondComma);
      lastHum = received.substring(secondComma + 1);

      // Publicar MQTT
      mqttClient.publish("LoRa/lux", lastLux.c_str());
      mqttClient.publish("LoRa/temp", lastTemp.c_str());
      mqttClient.publish("LoRa/hum", lastHum.c_str());

      // Mostrar OLED
      display.clearDisplay();
      display.setCursor(0,0);
      display.println("LoRa Gateway");
      display.setCursor(0,16);
      display.println("Lux: " + lastLux + " lx");
      display.setCursor(0,32);
      display.println("Temp: " + lastTemp + " C");
      display.setCursor(0,48);
      display.println("Hum: " + lastHum + " %");
      display.display();
    }
  }
}
