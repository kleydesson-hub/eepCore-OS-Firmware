#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// --- CONFIGURAÇÕES DO WI-FI ---
const char* ssid = "##########";
const char* password = "######";

// --- CONFIGURAÇÕES DO BROKER MQTT (Batendo com seu HTML) ---
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;

// --- TÓPICOS AJUSTADOS PARA O SEU SITE ---
const char* topico_temperatura = "deepcore/status/temp"; // Igual ao seu JS
const char* topico_bomba = "deepcore/status/rele";      // Igual ao seu JS

// --- CONFIGURAÇÕES DO DISPLAY OLED ---
#define LARGURA_TELA 128
#define ALTURA_TELA 64
Adafruit_SSD1306 display(LARGURA_TELA, ALTURA_TELA, &Wire, -1);

// --- SENSOR E RELÉS ---
const int PINO_ONE_WIRE = 4; 
const int PINO_BOMBA = 25;      
const int PINO_VENTOINHA = 17;  

OneWire oneWire(PINO_ONE_WIRE);
DallasTemperature sensors(&oneWire);
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long tempoAnterior = 0;
const long intervalo = 2000; 

float temperaturaC = 0.0;
String statusHTML = "DESLIGADO"; // Seu site espera "LIGADO" ou "DESLIGADO"

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWi-Fi Conectado!");
}

void reconnect() {
  while (!client.connected()) {
    String clientId = "DeepCore_ESP32_" + String(random(0, 0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado ao Broker EMQX!");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  sensors.begin();
  pinMode(PINO_BOMBA, OUTPUT);
  pinMode(PINO_VENTOINHA, OUTPUT);
  digitalWrite(PINO_BOMBA, HIGH); 
  digitalWrite(PINO_VENTOINHA, HIGH);

  if(display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    display.clearDisplay();
    display.display();
  }

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  unsigned long tempoAtual = millis();
  if (tempoAtual - tempoAnterior >= intervalo) {
    tempoAnterior = tempoAtual;

    sensors.requestTemperatures();
    temperaturaC = sensors.getTempCByIndex(0);

    // Lógica de Resfriamento
    if (temperaturaC >= 34.0) {
      digitalWrite(PINO_VENTOINHA, LOW); 
      digitalWrite(PINO_BOMBA, LOW);     
      statusHTML = "LIGADO";
    } 
    else if (temperaturaC >= 28.0 && temperaturaC < 34.0) {
      digitalWrite(PINO_VENTOINHA, LOW);  
      digitalWrite(PINO_BOMBA, HIGH);     
      statusHTML = "LIGADO"; // Para o site mostrar animação
    } 
    else if (temperaturaC < 24.0 && temperaturaC > 0) {
      digitalWrite(PINO_VENTOINHA, HIGH); 
      digitalWrite(PINO_BOMBA, HIGH);     
      statusHTML = "DESLIGADO";
    }

    // Atualiza Display OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("DEEPCORE OS");
    display.setTextSize(2);
    display.setCursor(0,20);
    display.print(temperaturaC, 1); display.println(" C");
    display.setTextSize(1);
    display.setCursor(0,45);
    display.print("Status: "); display.print(statusHTML);
    display.display();

    // ENVIO PARA O SITE
    client.publish(topico_temperatura, String(temperaturaC, 1).c_str());
    client.publish(topico_bomba, statusHTML.c_str());
    
    Serial.print("Temp enviada: "); Serial.println(temperaturaC);
  }
}