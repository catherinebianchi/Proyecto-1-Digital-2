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

//Condiciones 
#define freqPWM 50
#define resolution 8
#define debounce 200

#define potPin 34

//******************************************/
// Prototipos de funciones
//******************************************/
void initRGB(void);
void color(int rojo, int verde, int azul);

void initServo(void);

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

initRGB();
initServo();

attachInterrupt(digitalPinToInterrupt(boton), lectura, RISING);

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
  color(0,0,255);}
else if ((temperatura>23)&&(temperatura<25)){
  color(0,255,0);}
else if ((temperatura>25)&&(temperatura<27)){
  color(255,255,0);}
else if ((temperatura>27)){
  color(255,0,0);}


if ((temperatura < 23)){
  ledcWrite(canalServo, 0);}
else if ((temperatura>23)&&(temperatura<25)){
  //SERVO 
  }
else if ((temperatura>25)&&(temperatura<27)){
  //SERVO 
  }
else if ((temperatura>27)){
  //SERVO 
  }
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
}