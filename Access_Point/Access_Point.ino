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
int brillo = 842; // Valor PWM inicial (0 a 1023)
const int ledPin = LED_BUILTIN;

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
    <p>Brillo: <span id="brillo_val">)rawliteral" + String(brillo) + R"rawliteral(</span></p>
    <input type="range" min="764" max="1023" value=")rawliteral" + String(brillo) + R"rawliteral(" 
      onchange="ajustarBrillo(this.value)">
    
    <script>
      function ajustarBrillo(val) {
        document.getElementById('brillo_val').innerText = val;
        fetch('/brillo?valor=' + val);
      }
    </script>
  </body>
  </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

void actualizarPWM() {
  if (ledState) {
    analogWrite(ledPin, 1023 - brillo); // Inverso porque LED va de HIGH a LOW
  } else {
    analogWrite(ledPin, 1023); // Apagar
  }
}

void handleOn() {
  digitalWrite(ledPin, LOW); // Encender (inverso en ESP8266)
  ledState = true;
  actualizarPWM();
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleOff() {
  digitalWrite(ledPin, HIGH); // Apagar
  ledState = false;
  actualizarPWM();
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleToggle() {
  ledState = !ledState;
  digitalWrite(ledPin, ledState ? LOW : HIGH); // Toggle LED
  actualizarPWM();
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleBrillo() {
    if (server.hasArg("valor")) {
    brillo = server.arg("valor").toInt();
    actualizarPWM();
    Serial.print("Brillo actualizado: ");
    Serial.println(brillo);
  }
  server.send(200, "text/plain", "OK");
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

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH); // Apagar inicialmente (HIGH en ESP8266)

  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/toggle", handleToggle);
  server.on("/brillo", handleBrillo);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
