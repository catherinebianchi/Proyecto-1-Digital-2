//******************************************/
// Universidad del Valle de Guatemala
// BE3029 - Electronica Digital 2
// Catherine Bianchi 24080
// Proyecto 1: Sistema de monitoreo para cadena de frío
// MCU: ESP32 dev kit 1.0
//******************************************/
//******************************************/
// Librerias
//******************************************/
#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>
#include <driver/gpio.h>
#include <driver/ledc.h>

//******************************************/
// Definiciones
//******************************************/
//Botón
#define boton 22

//LED RGB
#define LEDrojo 13
#define LEDverde 27
#define LEDazul 25

#define canalRojo 0
#define canalVerde 1
#define canalAzul 2

//Servomotor
#define pinServo 33
#define canalServo 3

//Sensor de temperatura
#define LM35 15
#define voltajeRef 3.3
#define resolutionACD 4095

//Condiciones 
#define freqPWM 50
#define resolution 12
#define debounce 200

//Displays
const int segPins[8] = {4, 5, 18, 19, 21, 22, 23, 33}; //a, b, c, d, e, f, g, dp

const int digitPins[3] = {27, 26, 14};

const byte digitMap[10][8] ={
  {1,1,1,1,1,1,0,0}, //0
  {0,1,1,0,0,0,0,0}, //1
  {1,1,0,1,1,0,1,0}, //2
  {1,1,1,1,0,0,1,0}, //3
  {0,1,1,0,0,1,1,0}, //4
  {1,0,1,1,0,1,1,0}, //5
  {1,0,1,1,1,1,1,0}, //6
  {1,1,1,0,0,0,0,0}, //7
  {1,1,1,1,1,1,1,0}, //8
  {1,1,1,1,0,1,1,0}, //9
};

#define potPin 34

//******************************************/
// Prototipos de funciones
//******************************************/
void initRGB(void);
void color(int rojo, int verde, int azul);

void initServo(void);

void initDisplay(void);
void actualizarDisplay(float temperatura);

void IRAM_ATTR lectura();
void IRAM_ATTR refrescarDisplay();

//******************************************/
// Variables globales
//******************************************/
int potValue = 0;
int temperatura = 0;
volatile unsigned long ultimo = 0;
volatile bool flag = false;
int leerEstado = 0;

//Display
volatile int digitosDisplay[3] = {0,0,0};
volatile bool puntoDecimal = true;

hw_timer_t *timerDisplay = NULL;
volatile int digitoActual = 0;

//******************************************/
// ISRs Rutinas de Interrupcion
//******************************************/
void IRAM_ATTR lectura(){
  unsigned long ahora = millis();
  if (ahora - ultimo > debounce){
    flag = true;
    ultimo = ahora;
  }}

void IRAM_ATTR refrescarDisplay(){
  for (int i=0; i<3; i++){
    digitalWrite(digitPins[i],LOW);}

  int valor = digitosDisplay[digitoActual];
  for (int i=0; i<7; i++){
    digitalWrite(segPins[i], digitMap[valor][i]);  }

  digitalWrite(segPins[7], (digitoActual == 1 && puntoDecimal) ? HIGH:LOW);

  digitalWrite(digitPins[digitoActual], HIGH);

  digitoActual++;
  if (digitoActual >= 3){
    digitoActual=0;  }
}

//******************************************/
// Configuración
//******************************************/
void setup() {
Serial.begin(115200);

pinMode(LEDrojo, OUTPUT);
pinMode(LEDazul, OUTPUT);
pinMode(LEDverde, OUTPUT);

pinMode(boton, INPUT);

pinMode(potPin, INPUT);

initRGB();
initServo();

attachInterrupt(digitalPinToInterrupt(boton), lectura, RISING);

}

//******************************************/
// Loop Principal
//******************************************/
void loop() {
//potValue = analogRead(potPin);
//temperatura = map(potValue, 0, 4095, 21, 29);

//Lectura del sensor de temperatura
float lectura = analogRead(LM35);
float voltaje = (lectura*voltajeRef)/resolutionACD;
float temperatura = voltaje*100;
/*Serial.print("Voltaje: ");
Serial.print(voltaje);
Serial.print(" V  |  Temperatura: ");
Serial.print(temp);
Serial.println(" °C");*/

if(flag){
  flag = false;
  leerEstado = 1;
}

//RGB y servo para temperatura
if((!flag)&&(leerEstado==1)){
  leerEstado = 0;

if ((temperatura < 23)){
  color(0,0,255);}
else if ((temperatura>=23)&&(temperatura<25)){
  color(0,255,0);}
else if ((temperatura>=25)&&(temperatura<27)){
  color(255,255,0);}
else if ((temperatura>27)){
  color(255,0,0);}

if ((temperatura < 23)){
  ledcWrite(canalServo, 205);} //0°
else if ((temperatura>=23)&&(temperatura<25)){
  ledcWrite(canalServo, 256);} //45°
else if ((temperatura>=25)&&(temperatura<27)){
  ledcWrite(canalServo, 256);} //45°
else if ((temperatura>27)){
  ledcWrite(canalServo, 307);} //90°
}
}



//******************************************/
// Funciones para LED RGB (PWM)
//******************************************/
void initRGB(void){
  ledcSetup(canalAzul, freqPWM, resolution);
  ledcSetup(canalRojo, freqPWM, resolution);
  ledcSetup(canalVerde, freqPWM, resolution);

  ledcAttachPin(LEDazul, canalAzul);
  ledcAttachPin(LEDrojo, canalRojo);
  ledcAttachPin(LEDverde, canalVerde);
}

void color(int rojo, int verde, int azul){
  ledcWrite(canalRojo, rojo);
  ledcWrite(canalVerde, verde);
  ledcWrite(canalAzul, azul);
}

//******************************************/
// Funciones para Servomotor (PWM)
//******************************************/
void initServo(void){
  ledcSetup(canalServo, freqPWM, resolution);
  ledcAttachPin(pinServo, canalServo);
  ledcWrite(canalServo, 205); //Comienza desde 0°
}

//******************************************/
// Funciones para Displays
//******************************************/
void initDisplays(){
  for(int i=0; i<8; i++) pinMode(segPins[i],OUTPUT);
  for(int i=0; i<3; i++) pinMode(digitPins[i], OUTPUT);

  timerDisplay = timerBegin(0, 80, true);
  timerAttachInterrupt(timerDisplay, &refrescarDisplay, true);
  timerAlarmWrite(timerDisplay, 3000, true);
  timerAlarmEnable(timerDisplay);
}

void actualizarDisplay(float temperatura){
  int tempEntera = (int)temperatura;
  int decimal = (int)round((temperatura-tempEntera)*10);

  digitosDisplay[0] = tempEntera/10;
  digitosDisplay[1] = tempEntera%10;
  digitosDisplay[2] = decimal;
}
