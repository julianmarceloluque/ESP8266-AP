#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#ifndef APSSID
#define APSSID "ESPap"
#define APPSK "thereisnospoon"
#endif

/* Set these to your desired credentials. */
const char *ssid = APSSID;
const char *password = APPSK;

ESP8266WebServer server(80);

bool ledState = false; // Estado actual del LED

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
   connected to this access point to see it.
*/

void handleRoot() {
  String estado = ledState ? "Encendido" : "Apagado";
  String html = R"rawliteral(
  <html>
  <head>
    <style>
      .boton {
        font-size: 20px;
        padding: 15px 30px;
        margin: 10px;
        background-color: #ff5733;
        color: white;
        border: none;
        border-radius: 10px;
      }
      .boton:hover {
        background-color: #df4a2a;
      }
    </style>
  </head>
  <body>
    <h1>Control de LED</h1>
    <p>Estado actual: )rawliteral" + estado + R"rawliteral(</p>
    <a href="/on"><button class="boton">Encender</button></a>
    <a href="/off"><button class="boton">Apagar</button></a>
    <a href="/toggle"><button class="boton">TOGGLE</button></a>
  </body>
  </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

void handleOn() {
  digitalWrite(LED_BUILTIN, LOW); // Encender (inverso en ESP8266)
  ledState = true;
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleOff() {
  digitalWrite(LED_BUILTIN, HIGH); // Apagar
  ledState = false;
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleToggle() {
  ledState = !ledState;
  digitalWrite(LED_BUILTIN, ledState ? LOW : HIGH); // Toggle LED
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");


//  WiFi.softAP("ESP8266","12345678");
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("IP local: ");
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Apagar inicialmente (HIGH en ESP8266)

  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/toggle", handleToggle);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
