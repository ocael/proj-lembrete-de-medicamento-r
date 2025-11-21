#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// ------------------- Configurações do OLED -------------------
#define OLED_LARGURA 128
#define OLED_ALTURA 64
#define OLED_RST -1
Adafruit_SSD1306 tela(OLED_LARGURA, OLED_ALTURA, &Wire, OLED_RST);

// ------------------- Pinos -------------------
#define PINO_BUZZER 25
#define POTENCIOMETRO 34
#define LED_PODER 26      // LED verde, sempre ligado
#define LED_ALERTA 27     // LED vermelho, liga na hora do alerta

// ------------------- Conexão Wi-Fi e MQTT -------------------
const char* SSID = "Wokwi-GUEST";
const char* SENHA = "";
const char* BROKER_MQTT = "test.mosquitto.org";

WiFiClient clienteWiFi;
PubSubClient mqtt(clienteWiFi);
WiFiUDP udp;
NTPClient horarioNTP(udp, "pool.ntp.org", -10800, 60000); // GMT-3

// ------------------- Variáveis de tempo -------------------
int horaAlerta = 14;
int minutoAlerta = 30;
int ultimaHora = -1;
int ultimoMinuto = -1;

unsigned long tempoBuzzer = 0;
bool estadoAlerta = false;
const unsigned int intervaloBuzzer = 500;

unsigned long ultimaPublicacao = 0;
const unsigned long intervaloPublicacao = 1000;

// ------------------- Funções -------------------
void conectarRede() {
  Serial.print("Conectando à rede Wi-Fi: ");
  Serial.println(SSID);
  WiFi.begin(SSID, SENHA);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWi-Fi conectado com sucesso!");
}

void reconectarMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Tentando reconectar ao broker MQTT...");
    if (mqtt.connect("ESP32_LembreteReform")) {
      Serial.println("Conectado ao broker MQTT!");
    } else {
      Serial.print("Erro, código=");
      Serial.println(mqtt.state());
      delay(3000);
    }
  }
}

void publicarMQTT(const char* topico, const char* mensagem) {
  if (!mqtt.connected()) reconectarMQTT();
  mqtt.publish(topico, mensagem);
  Serial.print("Mensagem publicada em ");
  Serial.print(topico);
  Serial.print(": ");
  Serial.println(mensagem);
}

void atualizarTela(int horaAtual, int minutoAtual) {
  tela.clearDisplay();
  tela.setTextSize(1);
  tela.setCursor(0, 0);
  tela.printf("Hora Atual: %02d:%02d\n", horaAtual, minutoAtual);

  tela.setCursor(0, 16);
  tela.print("Proximo Lembrete:");
  tela.setCursor(0, 26);
  tela.printf("%02d:%02d", horaAlerta, minutoAlerta);

  tela.setCursor(0, 46);
  tela.print("Estado: ");
  tela.println(estadoAlerta ? "Alertando" : "Aguardando");

  tela.display();
}

// ------------------- Setup -------------------
void setup() {
  Serial.begin(115200);

  pinMode(PINO_BUZZER, OUTPUT);
  pinMode(LED_PODER, OUTPUT);
  pinMode(LED_ALERTA, OUTPUT);

  digitalWrite(LED_PODER, HIGH);  // LED verde sempre aceso
  noTone(PINO_BUZZER);

  if (!tela.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Erro ao inicializar OLED"));
    while (true);
  }

  tela.clearDisplay();
  tela.setTextSize(1);
  tela.setTextColor(SSD1306_WHITE);
  tela.setCursor(10, 25);
  tela.println("Sistema de Lembretes");
  tela.setCursor(10, 40);
  tela.println("Inicializando...");
  tela.display();
  delay(2000);

  conectarRede();
  mqtt.setServer(BROKER_MQTT, 1883);
  horarioNTP.begin();
  horarioNTP.update();
}

// ------------------- Loop -------------------
void loop() {
  if (!mqtt.connected()) reconectarMQTT();
  mqtt.loop();
  horarioNTP.update();

  int horaAtual = horarioNTP.getHours();
  int minutoAtual = horarioNTP.getMinutes();

  // Ajuste do horário de alerta via potenciômetro
  int leituraPot = analogRead(POTENCIOMETRO);
  int minutosTotais = map(leituraPot, 0, 4095, 0, 1439);
  horaAlerta = minutosTotais / 60;
  minutoAlerta = minutosTotais % 60;

  // Publicação MQTT do horário de alerta
  if (horaAlerta != ultimaHora || minutoAlerta != ultimoMinuto) {
    ultimaHora = horaAlerta;
    ultimoMinuto = minutoAlerta;
    char msg[10];
    sprintf(msg, "%02d:%02d", horaAlerta, minutoAlerta);
    publicarMQTT("medicamento/horario", msg);
  } else if (millis() - ultimaPublicacao >= intervaloPublicacao) {
    char msg[10];
    sprintf(msg, "%02d:%02d", horaAlerta, minutoAlerta);
    publicarMQTT("medicamento/horario", msg);
    ultimaPublicacao = millis();
  }

  // Verifica se é hora do alerta
  if (horaAtual == horaAlerta && minutoAtual == minutoAlerta) {
    estadoAlerta = true;
    if (millis() - tempoBuzzer >= intervaloBuzzer) {
      bool liga = !digitalRead(PINO_BUZZER);
      if (liga) {
        tone(PINO_BUZZER, 1000);
        digitalWrite(LED_ALERTA, HIGH);
        publicarMQTT("medicamento/alerta", "Hora de tomar o medicamento!");
      } else {
        noTone(PINO_BUZZER);
        digitalWrite(LED_ALERTA, LOW);
      }
      tempoBuzzer = millis();
    }
  } else {
    estadoAlerta = false;
    noTone(PINO_BUZZER);
    digitalWrite(LED_ALERTA, LOW);
  }

  atualizarTela(horaAtual, minutoAtual);

  delay(100);
}
