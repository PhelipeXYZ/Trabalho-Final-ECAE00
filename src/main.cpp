#include <ESP32Servo.h>

const int Pino_servo_1 = 18;
const int Pino_servo_2 = 25;
const int led_vermelho = 21; 
const int led_azul = 5; 
const int led_amarelo = 14; 
const int Pino_LDR = 32; 
const int ntcPin = 34; // ADC do NTC
const int potPin = 35; // ADC do potenciômetro

const float R_FIXED = 10000.0;
const float BETA = 3950.0;
const float T_0 = 298.15; // 25 ºC em Kelvin
const float R0 = 10000.0;

Servo irrigador;
Servo cobertura;

void setup() {
  Serial.begin(115200);
  pinMode(led_vermelho, OUTPUT);
  pinMode(led_azul, OUTPUT);
  pinMode(led_amarelo, OUTPUT);

  irrigador.attach(Pino_servo_1);
  irrigador.write(0);
  cobertura.attach(Pino_servo_2); // Inicialização da cobertura adicionada
  cobertura.write(0);
}

float ler_temperatura() {
  int adcValue = analogRead(ntcPin);
  float voltage = (adcValue / 4095.0) * 3.3;

  float rNTC = (voltage * R_FIXED) / (3.3 - voltage);
  float tempK = 1.0 / (1.0 / T_0 + (1.0 / BETA) * log(rNTC / R0));
  float tempC = tempK - 273.15;

  Serial.print("Temperatura: ");
  Serial.print(tempC);
  Serial.println(" °C");

  return tempC;
}

int ler_potenciometro() {
  int valorBruto = analogRead(potPin);
  int umidade = map(valorBruto, 0, 4095, 0, 100);

  return umidade;
}

int ler_ldr() {
  int lux = analogRead(Pino_LDR);
  int luminosidade = map(lux, 0, 4095, 0, 100);

  return luminosidade;
}


void loop() {
  int tempC = ler_temperatura();
  int umidade = ler_potenciometro();
  int luminosidade = ler_ldr();

  if (tempC > 28) {
    cobertura.write(0);
    digitalWrite(led_vermelho, HIGH);
    Serial.println("Status: Planta protegida");
  } else {
    cobertura.write(90);
    digitalWrite(led_vermelho, LOW);
    Serial.println("Status: Planta exposta");
  }
  

  Serial.print("Umidade: ");
  Serial.print(umidade);
  Serial.println(" %");

  if (umidade < 60) {
    irrigador.write(0);
    digitalWrite(led_azul, HIGH);
    Serial.println("Status: Irrigador ON");
  } else {
    irrigador.write(90);
    digitalWrite(led_azul, LOW);
    Serial.println("Status: Irrigador OFF");
  }

  Serial.print("Luminosidade: ");
  Serial.print(luminosidade);
  Serial.println(" %");

  if (luminosidade < 30) {
    digitalWrite(led_amarelo, HIGH);
  } else {
    digitalWrite(led_amarelo, LOW);
  }

  Serial.println("-----------------");
  delay(1000);
}
