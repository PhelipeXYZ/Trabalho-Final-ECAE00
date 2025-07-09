#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char* ssidAP = "EstufaESP32";     // Nome da rede criada pelo ESP
const char* passwordAP = "12345678";    // Senha da rede

WebServer server(80);

const int Pino_servo_1 = 18;
const int Pino_servo_2 = 25;
const int led_vermelho = 21;
const int led_azul = 5;
const int led_amarelo = 14;
const int Pino_LDR = 32;
const int Pino_POT_umid = 35;
const int Pino_POT_temp = 34;

Servo irrigador;
Servo cobertura;

bool plantaCoberta = false;
int temperatura = 0;
int umidade = 0;
int luminosidade = 0;

void setup() {
  Serial.begin(115200);

  pinMode(led_vermelho, OUTPUT);
  pinMode(led_azul, OUTPUT);
  pinMode(led_amarelo, OUTPUT);

  irrigador.attach(Pino_servo_1);
  irrigador.write(0);
  cobertura.attach(Pino_servo_2);
  cobertura.write(0);

  WiFi.softAP(ssidAP, passwordAP);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Rede criada. IP do AP: ");
  Serial.println(IP);

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", paginaHTML());
  });

  server.on("/dados", HTTP_GET, []() {
    String irrigStatus = (umidade < 30) ? "Ligado" : "Desligado";
    String coberturaStatus = (temperatura > 25 || plantaCoberta) ? "Aberta" : "Fechada";
    String json = "{";
    json += "\"temp\":" + String(temperatura) + ",";
    json += "\"umid\":" + String(umidade) + ",";
    json += "\"luz\":" + String(luminosidade) + ",";
    json += "\"irrig\":\"" + irrigStatus + "\",";
    json += "\"coberta\":\"" + coberturaStatus + "\"";
    json += "}";
    server.send(200, "application/json", json);
  });

  server.on("/toggle", HTTP_GET, []() {
    plantaCoberta = !plantaCoberta;

    if (plantaCoberta) {
      cobertura.write(90);
    } else {
      cobertura.write(0);
    }

    server.send(200, "text/plain", "OK");
  });

  server.begin();
}

void loop() {
  server.handleClient();

  int valorTemp = analogRead(Pino_POT_temp);
  temperatura = map(valorTemp, 0, 4095, -10, 50);

  int valorUmid = analogRead(Pino_POT_umid);
  umidade = map(valorUmid, 0, 4095, 0, 100);

  int lux = analogRead(Pino_LDR);
  luminosidade = map(lux, 0, 4095, 0, 100);

  // Controle da cobertura
  if (temperatura > 25 || plantaCoberta) {
    cobertura.write(0);
  } else {
    cobertura.write(90);
  }

  // Controle do irrigador automático
  if (umidade < 60) {
    irrigador.write(90);
    digitalWrite(led_azul, HIGH);
  } else {
    irrigador.write(0);
    digitalWrite(led_azul, LOW);
  }

  // LEDs de status
  digitalWrite(led_vermelho, (temperatura > 25) ? HIGH : LOW);
  digitalWrite(led_amarelo, (luminosidade < 30) ? HIGH : LOW);

  // Status para serial monitor
  String irrigStatus = (umidade < 60) ? "Ligado" : "Desligado";
  String coberturaStatus = (temperatura > 25 || plantaCoberta) ? "Aberta" : "Fechada";

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.print(" °C | Umidade: ");
  Serial.print(umidade);
  Serial.print(" % | Luminosidade: ");
  Serial.print(luminosidade);
  Serial.print(" % | Estufa: ");
  Serial.print(coberturaStatus);
  Serial.print(" | Irrigador: ");
  Serial.println(irrigStatus);
  
  delay(1000);
}

String paginaHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Estufa Inteligente</title>
  <style>
    body {
      margin: 0;
      padding: 0;
      font-family: 'Segoe UI', sans-serif;
      background-color: #121212;
      color: #ffffff;
      text-align: center;
    }
    h1 {
      margin-top: 20px;
      color: #00e676;
    }
    .container {
      display: flex;
      flex-wrap: wrap;
      justify-content: center;
      margin: 20px;
    }
    .card {
      background-color: #1e1e1e;
      border-radius: 12px;
      box-shadow: 0 4px 10px rgba(0, 0, 0, 0.5);
      padding: 50px;
      margin: 10px;
      width: 90%;
      max-width: 220px;
    }
    .card h2 {
      margin-bottom: 10px;
      font-size: 1.2em;
      color: #80cbc4;
    }
    .card p {
      font-size: 1.5em;
      margin: 0;
    }
    button {
      background-color: #00e676;
      color: #000000;
      padding: 12px 24px;
      border: none;
      border-radius: 8px;
      font-size: 1em;
      margin: 20px 0;
      cursor: pointer;
      transition: background-color 0.3s;
    }
    button:hover {
      background-color: #00c853;
    }
    @media (max-width: 600px) {
      .card {
        width: 100%;
      }
    }
  </style>
</head>
<body>
  <h1>Estufa Inteligente - DOMATECH AUTOMATION</h1>
  <div class="container">
    <div class="card">
      <h2>🌡️ Temperatura</h2>
      <p id="temp">--</p>
    </div>
    <div class="card">
      <h2>💧 Umidade</h2>
      <p id="umid">--</p>
    </div>
    <div class="card">
      <h2>🔆 Luminosidade</h2>
      <p id="luz">--</p>
    </div>
    <div class="card">
      <h2>🚿 Irrigador</h2>
      <p id="irrig">--</p>
    </div>
    <div class="card">
      <h2>🌱 Estufa</h2>
      <p id="coberta">--</p>
    </div>
  </div>
  <button onclick="toggleCobertura()">Abrir/Fechar Estufa</button>

  <script>
    function atualizar() {
      fetch('/dados').then(r => r.json()).then(d => {
        document.getElementById("temp").innerText = d.temp + ' °C';
        document.getElementById("umid").innerText = d.umid + ' %';
        document.getElementById("luz").innerText = d.luz + ' %';
        document.getElementById("irrig").innerText = d.irrig;
        document.getElementById("coberta").innerText = d.coberta;
      });
    }

    function toggleCobertura() {
      fetch('/toggle');
    }

    setInterval(atualizar, 2000);
    atualizar();
  </script>
</body>
</html>
  )rawliteral";

}
