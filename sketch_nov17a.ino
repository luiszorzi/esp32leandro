#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Update.h>
#include <ArduinoJson.h>

#define LED_PIN 2
String currentVersion = "1.0.0";

// --- CONFIG WiFi ---
#define WIFI_SSID  "keci"
#define WIFI_PASS  "keci123456"

// --- CONFIG GitHub ---
const char* githubUser = "luiszorzi";
const char* githubRepo = "esp32leandro";

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi conectado!");
}

bool performOTA(String binURL) {
  Serial.println("Baixando: " + binURL);
  WiFiClient client;
  HTTPClient http;

  http.begin(client, binURL);
  int code = http.GET();
  if (code != 200) {
    Serial.println("Erro HTTP: " + String(code));
    return false;
  }

  int len = http.getSize();
  if (!Update.begin(len)) return false;

  WiFiClient *stream = http.getStreamPtr();
  Update.writeStream(*stream);

  if (Update.end() && Update.isFinished()) {
    Serial.println("Atualização concluída! Reiniciando...");
    ESP.restart();
    return true;
  }
  return false;
}

void checkForUpdate() {
  String url = "https://api.github.com/repos/";
  url += githubUser;
  url += "/";
  url += githubRepo;
  url += "/releases/latest";

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  http.begin(client, url);
  http.addHeader("User-Agent", "ESP32");

  int code = http.GET();
  if (code != 200) {
    Serial.println("Falha ao buscar release.");
    return;
  }

  DynamicJsonDocument doc(4096);
  deserializeJson(doc, http.getString());

  String latest = doc["tag_name"].as<String>();
  String binURL = doc["assets"][0]["browser_download_url"].as<String>();

  Serial.println("Atual: " + currentVersion);
  Serial.println("Disponível: " + latest);

  if (latest != currentVersion) {
    Serial.println("Atualização encontrada! Atualizando...");
    performOTA(binURL);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  connectWiFi();
  checkForUpdate();
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);
}
