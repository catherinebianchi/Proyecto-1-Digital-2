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
#include "config.h"

//******************************************/
// Definiciones
//******************************************/
//Botón
#define boton 35

//LED RGB
#define LEDrojo 13
#define LEDverde 27
#define LEDazul 25

#define canalRojo 0
#define canalVerde 1
#define canalAzul 2

//Servomotor
#define pinServo 22
#define canalServo 3

//Sensor de temperatura
#define LM35 15
#define voltajeRef 3.3
#define resolutionACD 4095

//Condiciones 
#define freqPWM 50
#define resolution 12
#define debounce 200

//Display
#define pinA 16
#define pinB 17
#define pinC 32
#define pinD 14
#define pinE 33
#define pinF 23
#define pinG 4
#define pinDP 26
#define display1 21
#define display2 19
#define display3 18

uint8_t pinesDisplay[8]={pinA, pinB, pinC, pinD, pinE, pinF, pinG, pinDP};

//Adafruit IO
#define IO_LOOP_DELAY 5000

#define potPin 34


//******************************************/
// Prototipos de funciones
//******************************************/
void initRGB(void);
void color(int rojo, int verde, int azul);

void initServo(void);
void initDisplay(void);
void displayPunto(uint8_t punto);
void displayNumero(uint8_t numero);

void IRAM_ATTR lectura();
void configTimer(void);

void handleMessage(AdafruitIO_Data *data);

//******************************************/
// Variables globales
//******************************************/
float potValue = 0;
float temperatura = 0;
volatile unsigned long ultimo = 0;
volatile bool flag = false;
int leerEstado = 0;
int temp;
volatile int decenas; 
volatile int unidades; 
volatile int decimal;
volatile uint8_t displayActivo = 0;         // 0, 1, o 2

//Temporizador para Display
hw_timer_t *Timer0_Display = NULL;

//******************************************/
// Adafruit IO
//******************************************/
unsigned long lastUpdate = 0;
// set up the 'counter' feed
AdafruitIO_Feed *canalTemperatura = io.feed("temperatura");

//******************************************/
// ISRs Rutinas de Interrupcion
//******************************************/
void IRAM_ATTR lectura(){
  unsigned long ahora = millis();
  if (ahora - ultimo > debounce){
    flag = true;
    ultimo = ahora;
  }}

void IRAM_ATTR Timer0_ISR(){
  switch(displayActivo){
    case 0:
      digitalWrite(display1, HIGH); digitalWrite(display2, LOW); digitalWrite(display3, LOW);
      displayNumero(decenas);
      displayPunto(0);
      break;
    case 1:
      digitalWrite(display1, LOW); digitalWrite(display2, HIGH); digitalWrite(display3, LOW);
      displayNumero(unidades);
      displayPunto(1);
      break;
    case 2:
      digitalWrite(display1, LOW); digitalWrite(display2, LOW); digitalWrite(display3, HIGH);
      displayNumero(decimal);
      displayPunto(0);
      break;
  }

  displayActivo = (displayActivo + 1) % 3; // avanza al siguiente display
}

//******************************************/
// Configuración SETUP
//******************************************/
void setup() {
Serial.begin(115200);
pinMode(LEDrojo, OUTPUT);
pinMode(LEDazul, OUTPUT);
pinMode(LEDverde, OUTPUT);

pinMode(boton, INPUT);
pinMode(potPin, INPUT);
pinMode(pinServo, OUTPUT);

initRGB();
initServo();
initDisplay();


pinMode(display1, OUTPUT);
pinMode(display2, OUTPUT);
pinMode(display3, OUTPUT);
digitalWrite(display1, HIGH);
digitalWrite(display2, HIGH);
digitalWrite(display3, HIGH);

attachInterrupt(digitalPinToInterrupt(boton), lectura, FALLING);
configTimer();


io.connect();
// set up a message handler for the count feed.
// the handleMessage function (defined below)
// will be called whenever a message is
// received from adafruit io.
canalTemperatura->onMessage(handleMessage);

// wait for a connection
while (io.status() < AIO_CONNECTED) {
  Serial.print(".");
  delay(500);
}

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  canalTemperatura->get();
}



//******************************************/
// Loop Principal
//******************************************/
void loop() {
//ADAFRUIT IO
  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop
  // function. it keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  io.run();

  if (millis() > (lastUpdate + IO_LOOP_DELAY)) {
    // save count to the 'counter' feed on Adafruit IO
    Serial.print("sending -> ");
    Serial.println(temperatura);
    canalTemperatura->save(temperatura);

    // after publishing, store the current time
    lastUpdate = millis();
  }

potValue = analogRead(potPin);
temperatura = map(potValue, 0, 4095, 21, 29);

//Lectura del sensor de temperatura
/*float lectura = analogRead(LM35);
float voltaje = (lectura*voltajeRef)/resolutionACD;
float temperatura = voltaje*100;*/


/*Serial.print("Voltaje: ");
Serial.print(voltaje);
Serial.print(" V  |  Temperatura: ");
Serial.print(temp);
Serial.println(" °C");*/

//Si se presiona el botón
if(flag){
  flag = false;
  leerEstado = 1;
  Serial.print("Botón disparado");
}

//RGB y servo para temperatura
if((!flag)&&(leerEstado==1)){
  leerEstado = 0;

if ((temperatura < 23)){
  color(0,0,4095);} //Azul
else if ((temperatura>=23)&&(temperatura<25)){
  color(0,4095,0);} //Verde
else if ((temperatura>=25)&&(temperatura<27)){
  color(4095,4095,0);} //Amarillo
else if ((temperatura>27)){
  color(4095,0,0);} //Rojo

if ((temperatura < 23)){
  ledcWrite(canalServo, 102);} //0°
else if ((temperatura>=23)&&(temperatura<25)){
  ledcWrite(canalServo, 205);} //45°
else if ((temperatura>=25)&&(temperatura<27)){
  ledcWrite(canalServo, 205);} //45°
else if ((temperatura>27)){
  ledcWrite(canalServo, 307);} //90°  

//Cálculos para decenas, unidades y decimal
temp = temperatura*10; 
decenas = temp/100;
temp = temp - (decenas*100);
unidades = temp/10;
temp = temp - (unidades*10);
decimal = temp;
}


/*Serial.print("Temperatura: ");
Serial.print(temperatura);
Serial.print(" °C |  Decenas: ");
Serial.print(decenas);
Serial.print(" |  Unidades: ");
Serial.print(unidades);
Serial.print(" |  Decimal: ");
Serial.println(decimal);*/
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

void color(int rojo, int verde, int azul){//Utiliza valores entre 0 y 4095 por la resolución de 12 bits
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
  ledcWrite(canalServo, 102); //Comienza desde 0°
}

//******************************************/
// Funciones para Displays
//******************************************/
void initDisplay(void){ //Configuración de segmentos y displays como OUTPUT y apagados
  for(size_t i=0; i<8; i++){
    pinMode(pinesDisplay[i], OUTPUT);
    digitalWrite(pinesDisplay[i], HIGH);
  }
}

void IRAM_ATTR displayPunto(uint8_t punto){ //Se enciende el punto decimal (IRAM_ATTR porque se llama en el timer)
  if(punto == 1){
    digitalWrite(pinDP, LOW); //encendido 
  }else{
    digitalWrite(pinDP, HIGH); //apagado
  }
}

void IRAM_ATTR displayNumero(uint8_t numero){ //Define el número en el display (IRAM_ATTR porque se llama en el timer)
  for(int i=0; i<7; i++){
    digitalWrite(pinesDisplay[i], HIGH); //Se apagan todos los segmentos para borrar el número pasado
  }

  switch(numero){ //Como el display es de ánodo común, LOW representa encendido y HIGH representa apagado
    case 0: digitalWrite(pinA,LOW); digitalWrite(pinB,LOW); digitalWrite(pinC,LOW); digitalWrite(pinD,LOW); digitalWrite(pinE,LOW); digitalWrite(pinF,LOW); break;
    case 1: digitalWrite(pinB,LOW); digitalWrite(pinC,LOW); break;
    case 2: digitalWrite(pinA,LOW); digitalWrite(pinB,LOW); digitalWrite(pinG,LOW); digitalWrite(pinD,LOW); digitalWrite(pinE, LOW); break;
    case 3: digitalWrite(pinA,LOW); digitalWrite(pinB,LOW); digitalWrite(pinG,LOW); digitalWrite(pinC,LOW); digitalWrite(pinD,LOW); break;
    case 4: digitalWrite(pinF,LOW); digitalWrite(pinG,LOW); digitalWrite(pinB,LOW); digitalWrite(pinC,LOW); break;
    case 5: digitalWrite(pinA,LOW); digitalWrite(pinF,LOW); digitalWrite(pinG,LOW); digitalWrite(pinC,LOW); digitalWrite(pinD,LOW); break;
    case 6: digitalWrite(pinA,LOW); digitalWrite(pinF,LOW); digitalWrite(pinG,LOW); digitalWrite(pinE,LOW); digitalWrite(pinC,LOW); digitalWrite(pinD,LOW); break;
    case 7: digitalWrite(pinA,LOW); digitalWrite(pinB,LOW); digitalWrite(pinC,LOW); break;
    case 8: digitalWrite(pinA,LOW); digitalWrite(pinB,LOW); digitalWrite(pinC,LOW); digitalWrite(pinD,LOW); digitalWrite(pinE,LOW); digitalWrite(pinF,LOW); digitalWrite(pinG,LOW); break;
    case 9: digitalWrite(pinA,LOW); digitalWrite(pinF,LOW); digitalWrite(pinG,LOW); digitalWrite(pinB,LOW); digitalWrite(pinC,LOW); digitalWrite(pinD,LOW); break;
  }
}

void configTimer(){ //Configuración del temporizador
  Timer0_Display = timerBegin(0, 80, true);
  timerAttachInterrupt(Timer0_Display, &Timer0_ISR, true);
  timerAlarmWrite(Timer0_Display, 3000, true);
  timerAlarmEnable(Timer0_Display);
}

//******************************************/
// Funciones para Adafruit IO
//******************************************/
void handleMessage(AdafruitIO_Data *data) {

  Serial.print("received <- ");
  Serial.println(data->value());
}
