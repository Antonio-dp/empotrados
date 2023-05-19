/**
   ESP32 + DHT22 Example for Wokwi

   https://wokwi.com/arduino/projects/322410731508073042
*/

#include <NoDelay.h>
#include <string.h>
#include <NewPing.h>
#include <Servo.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <ESPAsyncWebSrv.h>
#include <ArduinoJson.h>
#include "paginaWeb.h"
#include <NTPClient.h>
#include "time.h"

//Constantes de red
const char* ssid = "A75B0C";
const char* password = "K2T76D2C03543";
String contrasena = "ea";
AsyncWebServer server(80);

//Constantes del sensor ultrasonico
const unsigned int PIN_TRIGGER = 5;
const unsigned int PIN_ECHO = 18;
const unsigned int BAUD_RATE = 115200;
const int DISTANCIA_MAX = 100;
const int DISTANCIA_ALERTA = 10;
long limiteInferior = 10;
long limiteSuperior = 11;

//Constantes del servomotor
const unsigned int PIN_SERVO = 13;
const long PAUSA_SERVO = 250;

//Constante de potenciometro
const unsigned int PIN_POT = 34;

//Constantes de los leds
const int PIN_LED_ROJO = 2;
const int PIN_LED_VERDE = 4;
const int PIN_LED_BLANCO = 17;
int brillo;
int pot = 0;

unsigned long tiempoActual = 0;
unsigned long tiempoAnterior = 0;
unsigned long intervalo = 10;
const long PAUSA = 500;
//python
char salidaJson[128];

typedef enum {
  DESOCUPADO,
  OCUPADO,
  OCUPADOYESPERANDO
} estadoSala;

estadoSala edo;

noDelay pausa(PAUSA);

//Variables del sensor ultrasonico
NewPing sonar(PIN_TRIGGER, PIN_ECHO, DISTANCIA_MAX);

//Variables del servomotor
Servo servo;

//Variables del zumbador
int frecuencia;

//variables de tiempo
// URL de un servidor NTP (Network Time Protocol)
const char* ntpServer = "pool.ntp.org";
// Offset en segundos de la zona horaria local con respecto a GMT
const long gmtOffset_sec = -25200;  // -7*3600
// Offset en segundos del tiempo, en el caso de horario de verano
const int daylightOffset_sec = 0;
// Estructura con la informacion de la fecha/hora actual
struct tm timeinfo;

void apagarAlarma();
void controlarLuzLedBlanco();
void encenderOcupado();
void encenderDesocupado();
void servoBloqueado();
void servoDesbloqueado();
long distanciaUnPing();
String processor(const String& var);
String sacarEstado();

//python
void peticionesPython();

noDelay pausaAbierto(3000, servoBloqueado, false);
//Servidor ESP32
void conectaRedWifi(const char* ssid, const char* password);
void configuraServidor();
void noHallada(AsyncWebServerRequest* request);

void setup() {
  servo.attach(PIN_SERVO);
  Serial.begin(BAUD_RATE);

  //wifi
  conectaRedWifi(ssid, password);
  configuraServidor();
  server.begin();
  Serial.println("Servidor web iniciado");

  //Hora y fecha
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  //leds
  pinMode(PIN_LED_ROJO, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_BLANCO, OUTPUT);

  encenderDesocupado();
  servoDesbloqueado();
  edo = DESOCUPADO;
}

void loop() {
  tiempoActual = millis();
  if (tiempoActual - tiempoAnterior >= intervalo) {
    tiempoAnterior = tiempoActual;
    controlarLuzLedBlanco();
  }
  if ((intptr_t)distanciaUnPing() < limiteInferior && (intptr_t)distanciaUnPing() != 0) {
    if (edo == OCUPADO) {
      edo = OCUPADOYESPERANDO;
    }
  } else {
    if (edo == OCUPADOYESPERANDO) {
      edo = OCUPADO;
    }
  }
  if (pausaAbierto.update()) {
    pausaAbierto.stop();
  }
}


void servoBloqueado() {
  servo.write(60);
}

void servoDesbloqueado() {
  servo.write(180);
}

void encenderOcupado() {
  digitalWrite(PIN_LED_ROJO, HIGH);
  digitalWrite(PIN_LED_VERDE, LOW);
}

void encenderDesocupado() {
  digitalWrite(PIN_LED_ROJO, LOW);
  digitalWrite(PIN_LED_VERDE, HIGH);
}

void controlarLuzLedBlanco() {
  pot = analogRead(PIN_POT);
  brillo = map(pot, 0, 4095, 0, 255);
  analogWrite(PIN_LED_BLANCO, brillo);
}

long distanciaUnPing() {
  int distancia = sonar.ping_cm();
  return distancia;
}

void conectaRedWifi(const char* ssid, const char* password) {
  Serial.print("Conectando..");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    noDelay(500);
  }
  Serial.print("Conectado con exito, mi IP es:");
  Serial.println(WiFi.localIP());
}

void configuraServidor() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", inicio, processor);
  });
  server.on("/bloqueo", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", bloqueo, processor);
  });
  server.on("/contra", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", contra, processor);
  });
  server.on("/bloquear", HTTP_GET, [](AsyncWebServerRequest* request) {
    encenderOcupado();
    servoBloqueado();
    edo = OCUPADO;
    peticionesPython();
    const char content[] PROGMEM = "todo fine";
    request->send_P(200, "text/plain", content);
  });
  server.on("/desbloquear", HTTP_GET, [](AsyncWebServerRequest* request) {
    encenderDesocupado();
    servoDesbloqueado();
    edo = DESOCUPADO;
    peticionesPython();
    const char content[] PROGMEM = "todo fine";
    request->send_P(200, "text/plain", content);
  });
  server.on("/code", HTTP_GET, [](AsyncWebServerRequest* request) {
    String codigo = request->getParam("codigo")->value();
    if (codigo == contrasena) {
      if (edo == OCUPADOYESPERANDO) {
        peticionesPython();
        servoDesbloqueado();
        pausaAbierto.start();
      }
    } else {
      Serial.print("Comando no valido: ");
      Serial.println(codigo);
    }
    const char content[] PROGMEM = "todo fine";
    request->send_P(200, "text/plain", content);
  });
  server.on("/password", HTTP_GET, [](AsyncWebServerRequest* request) {
    String passw = request->getParam("password")->value();
    contrasena = passw;
    const char content[] PROGMEM = "todo fine";
    request->send_P(200, "text/plain", content);
  });
  server.onNotFound(noHallada);
}

void noHallada(AsyncWebServerRequest* request) {
  // Le envia al cliente el mensaje de respuesta. Recibe como
  // argumentos el codigo de estado HTTP: 200 (indica exito al
  // obtener la solicitud), el tipo del contenido del mensaje:
  // texto plano y el cuerpo del mensaje de respuesta:
  // una cadena con la pagina con el mensaje "URL no encontrada".
  request->send(404, "text/plain", "URL no encontrada");
}

void peticionesPython() {
  time_t now;
  HTTPClient http;
  http.begin("https://lock-system-production.up.railway.app/datos");
  http.addHeader("Content-Type", "application/json");
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No se pudo obtener la fecha/hora");
  }
  time(&now);
  const size_t capacity = JSON_OBJECT_SIZE(2);
  StaticJsonDocument<capacity> doc;
  switch (edo) {
    case OCUPADO:
      doc["estado"] = "OCUPADO";
      break;
    case DESOCUPADO:
      doc["estado"] = "DESOCUPADO";
      break;
    case OCUPADOYESPERANDO:
      doc["estado"] = "ENTRANDING";
      break;
  }
  doc["fecha"] = now;
  serializeJson(doc, salidaJson);
  int httpCode = http.POST(String(salidaJson));
  if (httpCode > 0) {
    Serial.println("Codigo HTTP" + String(httpCode));
    if (httpCode == 200) {
      String cuerpo_respuesta = http.getString();
      Serial.println("Servidor respondio");
      Serial.println(cuerpo_respuesta);
    }
  }
}

String processor(const String& var) {
  if (var == "ESTADO")
    return sacarEstado();
  return String();
}

String sacarEstado() {
  switch (edo) {
    case OCUPADO:
      return "Ocupado";
    case DESOCUPADO:
      return "Desocupado";
    case OCUPADOYESPERANDO:
      return "Esperanding";
  }
}