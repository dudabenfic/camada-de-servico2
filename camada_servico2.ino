#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_now.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 1
#define CS_PIN 5
#define LED_VERDE 2
#define LED_VERMELHO 4

//Estrutura dos dados (ESPNOW)
typedef struct struct_message {
  float nivel_tinta;
  float temperatura;
  float umidade;
  int luminosidade;
  int presenca;
  unsigned long timestamp;
} struct_message;

const unsigned long TIMEOUT_MS = 5000;
#define SCROLL_SPEED 75

// Variáveis globais
struct_message data;
unsigned long lastReceivedTime = 0;
bool dataReceivedFlag = false; // Flag para indicar que o ESP-NOW recebeu dados
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
int currentScreen = 0;
char displayBuffer[50];
bool redLedStatus = false;
bool greenLedStatus = false;

// Variáveis de controle de envio HTTP
unsigned long lastSendTime = 0;
const long SEND_INTERVAL_MS = 10000; // Enviar dados a cada 10 segundos


const char* SERVER_URL = "http://10.55.46.131:8000/dados";
const char* ssid = "Galaxy J8F22C";
const char* password = "snhm6588";


void conectarWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("🔌 Conectando ao WiFi");
  int tentativas = 0;

  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi conectado!");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ Falha ao conectar ao WiFi!");
  }
}

// Função para enviar dados ao FASTAPI
void enviarParaServidor(struct_message d) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi desconectado. Tentando reconectar...");
    conectarWiFi();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("❌ Não foi possível reconectar ao WiFi.");
      return;
    }
  }

  HTTPClient http;
  http.begin(SERVER_URL);

  // Adicionar o Content-Type Header
  http.addHeader("Content-Type", "application/json");

  String json = "{";
  json += "\"nivel\": " + String(d.nivel_tinta, 1) + ",";
  json += "\"temperatura\": " + String(d.temperatura, 2) + ",";
  json += "\"umidade\": " + String(d.umidade, 2) + ",";
  json += "\"luminosidade\": " + String(d.luminosidade) + ",";
  json += "\"presenca\": " + String(d.presenca) + ",";
  json += "\"timestamp\": " + String(d.timestamp);
  json += "}";

  Serial.print("\n➡️ Enviando JSON: ");
  Serial.println(json);

  // Usar http.POST
  int httpResponseCode = http.POST(json);
  
  Serial.print(" Código HTTP: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print(" Resposta: ");
    Serial.println(response);
  } else {
    Serial.print(" Erro HTTP: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

void updateLEDs() {
  unsigned long tempo_atual = millis();
  bool green = greenLedStatus;
  bool red = redLedStatus;

  // A lógica de LED está ligada ao recebimento ESP-NOW
  if (dataReceivedFlag) { 
    green = true;
    red = false;
    lastReceivedTime = tempo_atual;
  } else if (tempo_atual - lastReceivedTime > TIMEOUT_MS) {
    green = false;
    red = true;
  } else {
    green = true;
    red = false;
  }

  if (green != greenLedStatus) {
    digitalWrite(LED_VERDE, green ? HIGH : LOW);
    greenLedStatus = green;
  }

  if (red != redLedStatus) {
    digitalWrite(LED_VERMELHO, red ? HIGH : LOW);
    redLedStatus = red;
  }
}

// Função Trocar Tela
void nextScreen() {
  currentScreen = (currentScreen + 1) % 5;

  switch (currentScreen) {
    case 0:
      snprintf(displayBuffer, sizeof(displayBuffer), "NVL %.1f%% ", data.nivel_tinta);
      break;
    case 1:
      snprintf(displayBuffer, sizeof(displayBuffer), "TMP %.1f C ", data.temperatura);
      break;
    case 2:
      snprintf(displayBuffer, sizeof(displayBuffer), "UMD %.1f%% ", data.umidade);
      break;
    case 3:
      snprintf(displayBuffer, sizeof(displayBuffer), "LUX %d ", data.luminosidade);
      break;
    case 4:
      snprintf(displayBuffer, sizeof(displayBuffer), "PRS %s ", data.presenca ? "ON" : "OFF");
      break;
  }

  Serial.print("🖥️ Tela ");
  Serial.print(currentScreen + 1);
  Serial.print(": ");
  Serial.println(displayBuffer);

  P.displayText(displayBuffer, PA_LEFT, SCROLL_SPEED, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
}

// ===================================
// --- CALLBACK: Recepção ESP-NOW (APENAS SETA A FLAG) ---
// ===================================
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *data_recv, int len) {
  if (len == sizeof(struct_message)) {
    memcpy(&data, data_recv, sizeof(data));
    dataReceivedFlag = true; // Apenas seta a flag

    Serial.println("\n✅ Dados Recebidos via ESP-NOW");
    // Removida a chamada enviarParaServidor(data); daqui
  } else {
    Serial.println("❌ Pacote recebido inválido!");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println(WiFi.macAddress());

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);

  conectarWiFi();

  // Inicia ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ Erro ao iniciar ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("ESP-NOW iniciado e aguardando dados...");

  // Inicia display
  P.begin();
  P.setIntensity(0, 4);
  P.displayClear();

  lastReceivedTime = millis();
  updateLEDs();
  nextScreen();
}

void loop() {
  unsigned long currentTime = millis();

  // 1. ROTINA DE ENVIO ESP-NOW (PRIORIDADE)
  // Verifica se o ESP-NOW recebeu dados e envia no loop (seguro)
  if (dataReceivedFlag) {
    Serial.println("Acionando envio HTTP por dados ESP-NOW...");
    enviarParaServidor(data); 
    
    // Reseta a flag e o timer de envio agendado
    dataReceivedFlag = false;
    lastSendTime = currentTime; 
  }

  // 2. ROTINA DE ENVIO AGENDADO (A cada 10 segundos, se não foi acionado pelo ESP-NOW)
  if (currentTime - lastSendTime >= SEND_INTERVAL_MS) {
    if (WiFi.status() == WL_CONNECTED) {
      // Envia os dados
      enviarParaServidor(data);
      // Reseta o contador
      lastSendTime = currentTime;
    } else {
      Serial.println("WiFi desconectado. Tentando reconectar...");
      conectarWiFi();
    }
  }

  // Animação do Display e Troca de Tela
  if (P.displayAnimate()) {
    nextScreen();
  }

  // Atualização dos LEDs
  updateLEDs();
}