#include <SPI.h>          // библиотека для работы с шиной SPI
#include "nRF24L01.h"     // библиотека радиомодуля
#include "RF24.h"         // ещё библиотека радиомодуля
#include "string.h" 

/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
RF24 radio(3, 10); // "создать" модуль на пинах 9 и 10 Для Уно

byte address[][1] = {"1Node"}; //возможные номера труб
int transmit_data[3];  

const int switchPin = 2;  // кнопка джойстика
const int pinX = A1; // Ось X джойстика
const int pinY = A0; // Ось Y джойстика

void setup() {
  pinMode(pinX, INPUT);
  pinMode(pinY, INPUT);
  pinMode(switchPin, INPUT);
  digitalWrite(switchPin, HIGH);
  Serial.begin(115200);
  
  radio.begin(); //активировать модуль
  radio.setAutoAck(1);         //режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);    //(время между попыткой достучаться, число попыток)
  radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);     //размер пакета, в байтах

  radio.openWritingPipe(address[0]);   //мы - труба 0, открываем канал для передачи данных
  radio.setChannel(0x60);  //выбираем канал (в котором нет шумов!)

  radio.setPALevel (RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate (RF24_250KBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!

  radio.powerUp(); //начать работу
  radio.stopListening();  //не слушаем радиоэфир, мы передатчик
  
}

void loop() {
  transmit_data[0] = analogRead(pinX); // считываем аналоговое значение оси Х
  transmit_data[1] = analogRead(pinY);
  transmit_data[2] = digitalRead(switchPin);
  
  
/****************** Ping Out Role ***************************/  
  radio.write(&transmit_data, sizeof(transmit_data));
  delay(10);
} // Loop

