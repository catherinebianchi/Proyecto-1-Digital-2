/******************************************/
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
uint8_t numero = 0;

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

//******************************************/
// Variables globales
//******************************************/
int potValue = 0;
int temperatura = 0;
volatile unsigned long ultimo = 0;
volatile bool flag = false;
int leerEstado = 0;

//******************************************/
// ISRs Rutinas de Interrupcion
//******************************************/
void IRAM_ATTR lectura(){
  unsigned long ahora = millis();
  if (ahora - ultimo > debounce){
    flag = true;
    ultimo = ahora;
  }

}

//******************************************/
// Configuracion
//******************************************/
void setup() {
pinMode(LEDrojo, OUTPUT);
pinMode(LEDazul, OUTPUT);
pinMode(LEDverde, OUTPUT);

pinMode(boton, INPUT);
pinMode(potPin, INPUT);
pinMode(pinServo, OUTPUT);

initRGB();
initServo();
initDisplay();

attachInterrupt(digitalPinToInterrupt(boton), lectura, RISING);
pinMode(display1, OUTPUT);
pinMode(display2, OUTPUT);
pinMode(display3, OUTPUT);
digitalWrite(display1, HIGH);
digitalWrite(display2, HIGH);
digitalWrite(display3, HIGH);

}

//******************************************/
// Loop Principal
//******************************************/
void loop() {
potValue = analogRead(potPin);
temperatura = map(potValue, 0, 4095, 21, 29);

if(flag){
  flag = false;
  leerEstado = 1;
}

//RGB para temperatura
if((!flag)&&(leerEstado==1)){
  leerEstado = 0;

if ((temperatura < 23)){
  color(0,0,4095);}
else if ((temperatura>=23)&&(temperatura<25)){
  color(0,4095,0);}
else if ((temperatura>=25)&&(temperatura<27)){
  color(4095,4095,0);}
else if ((temperatura>27)){
  color(4095,0,0);}

if ((temperatura < 23)){
  ledcWrite(canalServo, 102);} //0°
else if ((temperatura>=23)&&(temperatura<25)){
  ledcWrite(canalServo, 205);} //45°
else if ((temperatura>=25)&&(temperatura<27)){
  ledcWrite(canalServo, 205);} //45°
else if ((temperatura>27)){
  ledcWrite(canalServo, 307);} //90°


}
//Display 1
  digitalWrite(display1, HIGH);
  digitalWrite(display2, LOW);   
  digitalWrite(display3, LOW);
  displayNumero(1);              
  displayPunto(0);
  delay(5);

//Display 2
  digitalWrite(display1, LOW);
  digitalWrite(display2, HIGH);   
  digitalWrite(display3, LOW);
  displayNumero(8);              
  displayPunto(1);
  delay(5);

//Display 3
  digitalWrite(display1, LOW);
  digitalWrite(display2, LOW);   
  digitalWrite(display3, HIGH);
  displayNumero(4);              
  displayPunto(0);
  delay(5);


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
}

//******************************************/
// Funciones para Displays
//******************************************/
void initDisplay(void){
  for(size_t i=0; i<8; i++){
    pinMode(pinesDisplay[i], OUTPUT);
    digitalWrite(pinesDisplay[i], HIGH);
  }
}

void displayPunto(uint8_t punto){
  if(punto == 1){
    digitalWrite(pinDP, LOW); //encendido 
  }else{
    digitalWrite(pinDP, HIGH); //apagado
  }
}

void displayNumero(uint8_t numero){
  for(int i=0; i<7; i++){
    digitalWrite(pinesDisplay[i], HIGH);
  }

  switch(numero){
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

