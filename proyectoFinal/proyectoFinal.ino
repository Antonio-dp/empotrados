/**
   ESP32 + DHT22 Example for Wokwi

   https://wokwi.com/arduino/projects/322410731508073042
*/
/** Estas son las librerías que se están incluyendo en el código. 
    Las librerías son conjuntos de funciones predefinidas que facilitan la 
    programación al proporcionar implementaciones de funciones comunes y útiles. 
    En este caso, estas librerías son necesarias para diversas funcionalidades del programa, 
    como la comunicación en red, control de periféricos y manejo de tiempo.
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

/* Aquí se definen algunas constantes relacionadas con la configuración de la red Wi-Fi. 
  ssid y password representan el nombre y la contraseña de la red Wi-Fi a la que se desea conectar el dispositivo. 
  contrasena es una variable de tipo String que almacena la contraseña. 
  AsyncWebServer es un objeto que se utiliza para configurar y manejar un servidor web asíncrono en el puerto 80.
*/
const char* ssid = "A75B0C";
const char* password = "K2T76D2C03543";
//Contraseña de acceso por defecto
String contrasena = "ea";
//Inicia el servidor web en el puerto 80
AsyncWebServer server(80);

/*
  Aquí se definen constantes relacionadas con un sensor ultrasónico. 
  PIN_TRIGGER y PIN_ECHO representan los pines utilizados para conectar el sensor ultrasónico. 
  BAUD_RATE indica la velocidad de comunicación en baudios. DISTANCIA_MAX y DISTANCIA_ALERTA 
  son valores límite para la distancia medida por el sensor ultrasónico. 
  limiteInferior y limiteSuperior son límites utilizados para ciertas operaciones en el código.
*/
const unsigned int PIN_TRIGGER = 5;
const unsigned int PIN_ECHO = 18;
//BAUD_RATE indica la velocidad de comunicación en baudios
const unsigned int BAUD_RATE = 115200;
//Limite máximo en centimetros de detección
const int DISTANCIA_MAX = 100;
//Limite inferior en cm de deteccion del sensor ultrasonico
long limiteInferior = 10;
//Limite superior en cm de deteccion del sensor ultrasonico
long limiteSuperior = 11;

//Pin de informacion del servo
const unsigned int PIN_SERVO = 13;

//Constante de potenciometro
const unsigned int PIN_POT = 34;

//Pin led rojo
const int PIN_LED_ROJO = 2;
//Pin led verde
const int PIN_LED_VERDE = 4;
//Pin led blanco
const int PIN_LED_BLANCO = 17;

//Variable que permite regular la intensidad del brillo de el led blanco
int brillo;
//Variable que guarda el valor de resistencia del potenciometro
int pot = 0;

//Permite guardar el tiempo actual en ms
unsigned long tiempoActual = 0;
//Permite guardar el tiempo anterior en ms
unsigned long tiempoAnterior = 0;
//Permite guardar el intervalo en ms
unsigned long intervalo = 10;
//Pausa en ms
const long PAUSA = 500;
//Permite guardar el objeto json a enviar en el POST para python
char salidaJson[128];

//Enum de los estados del sistema
typedef enum {
  DESOCUPADO,
  OCUPADO,
  OCUPADOYESPERANDO
} estadoSala;
//Variable de estado del sistema
estadoSala edo;
//Inicializado de la variable de pausa
noDelay pausa(PAUSA);

//Inicializado de la variable que permite el funcionamiento del sensor ultrasonico
NewPing sonar(PIN_TRIGGER, PIN_ECHO, DISTANCIA_MAX);

//Variables del servomotor
Servo servo;

//variables de tiempo
// URL de un servidor NTP (Network Time Protocol)
const char* ntpServer = "pool.ntp.org";
// Offset en segundos de la zona horaria local con respecto a GMT
const long gmtOffset_sec = -25200;  // -7*3600
// Offset en segundos del tiempo, en el caso de horario de verano
const int daylightOffset_sec = 0;
// Estructura con la informacion de la fecha/hora actual
struct tm timeinfo;


//Permite regular la intensidad de la luz del led blanco
void controlarLuzLedBlanco();

//Enciende el led rojo y apaga el LED Verde
void encenderOcupado();

//Enciende el led verde y apaga el LED Rojo
void encenderDesocupado();

//Gira el servomotor 60 grados para bloquear la puerta
void servoBloqueado();

//Gira el servomotor 180 grados para desbloquear la puerta
void servoDesbloqueado();

//Permite obtener la distancia del objeto con el sensor ultrasonico
long distanciaUnPing();

//Permite insertar el estado que se utilizara en la pagina web
String processor(const String& var);
//Devuelve el estado actual en string
String sacarEstado();

//Permite enviar el estado atual y la fecha con un método POST
void peticionesPython();

//Permite desbloquear el servomotor despues de 3 segundos
noDelay pausaAbierto(3000, servoBloqueado, false);

//Permite conectarse a la red wifi
void conectaRedWifi(const char* ssid, const char* password);

//Servidor web del ESP32
void configuraServidor();

//Envia una respuesta si ocurre un fallo en la petición
void noHallada(AsyncWebServerRequest* request);

void setup() {
  //Inicializa el servomotor
  servo.attach(PIN_SERVO);
  //Inicia la velocidad de comunicación en baudios
  Serial.begin(BAUD_RATE);

  //Se conecta a la red
  conectaRedWifi(ssid, password);
  //Inicializa el servidor y habilita los endpoints
  configuraServidor();
  server.begin();
  Serial.println("Servidor web iniciado");

  //Hora y fecha
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  //Configura el PIN y los LEDS
  pinMode(PIN_LED_ROJO, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_BLANCO, OUTPUT);

  //Inicia sin seguro la puerta y con el LED Verde encendida
  encenderDesocupado();
  servoDesbloqueado();
  edo = DESOCUPADO;
}

void loop() {
  //Permite controlar la luz
  tiempoActual = millis();
  if (tiempoActual - tiempoAnterior >= intervalo) {
    tiempoAnterior = tiempoActual;
    controlarLuzLedBlanco();
  }
  //Permite saber si hay una persona cerca mientras el estado sea OCUPADO
  if ((intptr_t)distanciaUnPing() < limiteInferior && (intptr_t)distanciaUnPing() != 0) {
    if (edo == OCUPADO) {
      edo = OCUPADOYESPERANDO;
    }
  } else {
    if (edo == OCUPADOYESPERANDO) {
      edo = OCUPADO;
    }
  }
  //Permite cerrar la puerta despues de abrirla con contraseña
  if (pausaAbierto.update()) {
    pausaAbierto.stop();
  }
}

//Gira el servomotor 60 grados para bloquear la puerta
void servoBloqueado() {
  servo.write(60);
}
//Gira el servomotor 180 grados para desbloquear la puerta
void servoDesbloqueado() {
  servo.write(180);
}
//Enciende el led rojo y apaga el LED Verde
void encenderOcupado() {
  digitalWrite(PIN_LED_ROJO, HIGH);
  digitalWrite(PIN_LED_VERDE, LOW);
}
//Enciende el led verde y apaga el LED Rojo
void encenderDesocupado() {
  digitalWrite(PIN_LED_ROJO, LOW);
  digitalWrite(PIN_LED_VERDE, HIGH);
}
//Permite regular la intensidad de la luz del led blanco
void controlarLuzLedBlanco() {
  pot = analogRead(PIN_POT);
  brillo = map(pot, 0, 4095, 0, 255);
  analogWrite(PIN_LED_BLANCO, brillo);
}
//Permite obtener la distancia del objeto con el sensor ultrasonico
long distanciaUnPing() {
  int distancia = sonar.ping_cm();
  return distancia;
}
//Permite conectarse a la red wifi
void conectaRedWifi(const char* ssid, const char* password) {
  Serial.print("Conectando..");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    noDelay(500);
  }
  Serial.print("Conectado con exito, mi IP es:");
  Serial.println(WiFi.localIP());
}

//Servidor web del ESP32
void configuraServidor() {
  //EndPoint get que lanza la pagina de inicio en donde puedes introducir la contraseña para desbloquear la puerta
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", inicio, processor);
  });
  //EndPoint get que lanza la pagina de inicio en donde puedes cambiar entre el estado de OCUPADO y DESOCUPADO
  server.on("/bloqueo", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", bloqueo, processor);
  });
  //EndPoint get que lanza la pagina de inicio en donde puedes cambiar la contraseña con la que desbloqueas la puerta
  server.on("/contra", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", contra, processor);
  });
  //Permite activar las funciones para bloquear la puerta y encender el led rojo
  server.on("/bloquear", HTTP_GET, [](AsyncWebServerRequest* request) {
    encenderOcupado();
    servoBloqueado();
    edo = OCUPADO;
    peticionesPython();
    const char content[] PROGMEM = "todo fine";
    request->send_P(200, "text/plain", content);
  });
  //Permite activar las funciones para desbloquear la puerta y encender el led verde
  server.on("/desbloquear", HTTP_GET, [](AsyncWebServerRequest* request) {
    encenderDesocupado();
    servoDesbloqueado();
    edo = DESOCUPADO;
    peticionesPython();
    const char content[] PROGMEM = "todo fine";
    request->send_P(200, "text/plain", content);
  });
  //Permite abrir la puerta con el codigo dado en la pagina
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
  //Permite cambiar la contraseña para desbloquear la puerta por la dada en la petición
  server.on("/password", HTTP_GET, [](AsyncWebServerRequest* request) {
    String passw = request->getParam("password")->value();
    contrasena = passw;
    const char content[] PROGMEM = "todo fine";
    request->send_P(200, "text/plain", content);
  });
  server.onNotFound(noHallada);
}
//Devuelva una respuesta en caso de algun error en la petición
void noHallada(AsyncWebServerRequest* request) {
  // Le envia al cliente el mensaje de respuesta. Recibe como
  // argumentos el codigo de estado HTTP: 200 (indica exito al
  // obtener la solicitud), el tipo del contenido del mensaje:
  // texto plano y el cuerpo del mensaje de respuesta:
  // una cadena con la pagina con el mensaje "URL no encontrada".
  request->send(404, "text/plain", "URL no encontrada");
}
//Cliente que permite hacer POST desde el ESP32 a una api hosteada.
void peticionesPython() {
  //Variable de fecha
  time_t now;
  //Cliente http
  HTTPClient http;
  //URL de la api hosteada
  http.begin("https://lock-system-production.up.railway.app/datos");
  //header
  http.addHeader("Content-Type", "application/json");
  //Permite obtener la fecha y hora actual
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No se pudo obtener la fecha/hora");
  }
  //Setea la fecha y hora actual en a variable now
  time(&now);
  //Establece el tamaño de objetos en el json document
  const size_t capacity = JSON_OBJECT_SIZE(2);
  //Json document
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
  //Serializacion de los datos a enviar
  serializeJson(doc, salidaJson);
  //Peticion POST
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
//Permite insertar el estado que se utilizara en la pagina web
String processor(const String& var) {
  if (var == "ESTADO")
    return sacarEstado();
  return String();
}
//Devuelve el estado actual en string
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