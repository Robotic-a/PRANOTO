#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

// ======= GLOBAL VARIABEL =======
int speed1 = 150;
int speed2 = 150;
int speed3 = 150;
int speed4 = 150;

// ======= WIFI AP CONFIG =======
const char* ssid = "ESP32_AP";
const char* password = "12345678";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// ======= FUNGSI: KIRIM JSON =======
void sendMotorSpeedData(AsyncWebSocketClient *client) {
  StaticJsonDocument<200> doc;
  doc["speed1"] = speed1;
  doc["speed2"] = speed2;
  doc["speed3"] = speed3;
  doc["speed4"] = speed4;

  String jsonStr;
  serializeJson(doc, jsonStr);
  client->text(jsonStr);
}

// ======= HANDLER EVENT WEBSOCKET =======
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("Client %u connected\n", client->id());
    sendMotorSpeedData(client);  // Kirim data saat client connect
  }
}

// ======= SETUP =======
void setup() {
  Serial.begin(115200);

  // Setup WiFi Access Point
  WiFi.softAP(ssid, password);
  Serial.print("ESP32 AP IP: ");
  Serial.println(WiFi.softAPIP());

  // WebSocket setup
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);

  // Web root minimal
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "ESP32 WebSocket JSON Server");
  });

  // Start server
  server.begin();
}

// ======= LOOP =======
void loop() {
  // Di sini kamu bisa ubah nilai speedX kapan saja
  // Misal dari sensor, tombol, atau logika lainnya
}
