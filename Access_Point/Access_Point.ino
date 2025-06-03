#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#ifndef APSSID
#define APSSID "ESPap"
#define APPSK "thereisnospoon"
#endif

const char *ssid = APSSID;
const char *password = APPSK;
int brillo = 842; // Valor PWM inicial (0 a 1023)
int brilloPorcentual = 0;
const int ledPin = LED_BUILTIN;

ESP8266WebServer server(80);

bool ledState = false; // Estado actual del LED

void handleRoot() {
  String estado = ledState ? "Encendido" : "Apagado";
  brilloPorcentual = map(brillo, 764, 1023, 0, 100); // Se actualiza correctamente el porcentaje antes de mostrarlo

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

    <p>Brillo: <span id="brillo_val">)rawliteral" + String(brilloPorcentual) + R"rawliteral(%</span></p>

    <!-- ✅ CORREGIDO: el value ahora usa brillo (PWM) directamente -->
    <input type="range" min="764" max="1023" value=")rawliteral" + String(brillo) + R"rawliteral("
      onchange="ajustarBrillo(this.value)">

    <script>
      function ajustarBrillo(val) {
        let porcentaje = Math.round((val - 764) * 100 / (1023 - 764));
        document.getElementById('brillo_val').innerText = porcentaje + "%";
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
    brilloPorcentual = map(brillo, 764, 1023, 0, 100);

    actualizarPWM();
    Serial.print("Brillo actualizado: ");
    Serial.println(brilloPorcentual); // Ahora muestra el valor correcto
  }
  server.send(200, "text/plain", "OK");
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");

  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
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
