#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(9, 10);//подключаем нрф CE к 9, CS к 10

byte address[][1] = {"1Node"}; //возможные номера труб
int recieved_data[3];//массив получаемых данных, первый по оси Х (лево/право), второй по У (вперед назад), третий - нажатие кнопки
int forward = 495;//в расслабленном состоянии джостик выдает первое значение
int sideway = 506;//второе значение
//у джостика диапазон от 0 до 1023

void setup() {
  Serial.begin(115200); //открываем порт для связи с ПК
  radio.begin(); //активировать радио модуль
  radio.setAutoAck(1); //режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15); //(время между попыткой достучаться, число попыток)
  radio.enableAckPayload(); //разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32); //размер пакета, в байтах
  radio.openReadingPipe(1, address[0]);     //хотим слушать трубу 0
  radio.setChannel(0x60);  //выбираем канал (в котором нет шумов!)
  radio.setPALevel (RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate (RF24_250KBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!
  radio.powerUp(); //начать работу
  radio.startListening();  //начинаем слушать эфир, мы приёмный модуль
  pinMode(2, OUTPUT); //in1 
  pinMode(3, OUTPUT); //in2
  pinMode(6, OUTPUT); //en1
  pinMode(4, OUTPUT); //in3
  pinMode(7, OUTPUT); //in4
  pinMode(5, OUTPUT); //en2

  //биты in1 и in2 отвечают за логику направления движения двигателя, 10-вперед, 01-назад
}

void loop() {
  byte pipeNo;
  while ( radio.available(&pipeNo)) {  // слушаем эфир со всех труб
    radio.read( &recieved_data, sizeof(recieved_data) ); // чиатем входящий сигнал

    Serial.print("Recieved: ");
    Serial.print(recieved_data[0]);
    Serial.print(" ");
    Serial.print(recieved_data[1]);
    Serial.print(" ");
    Serial.println(recieved_data[2]);
    
    double outputL = 0.0;
    double outputR = 0.0;

    int input1 = recieved_data[0];
    int input2 = recieved_data[1];

    if (input1 > sideway) {
      if (input2 > forward) {
        outputL = CalcVector(input1 - sideway, input2 - forward);
      } else {
        outputL = -1.0 * CalcVector(input1 - sideway, input2 - forward);
      }
      outputR = abs(outputL);
      if (input2 > forward) {
        outputR = CalcSide(input1 - sideway, outputR * 2.0);
      } else {
        outputR = -1.0 * CalcSide(input1 - sideway, outputR * 2.0);
      }
    } else {
      if (input2 > forward) {
        outputR = CalcVector(sideway - input1, input2 - forward);
      } else {
        outputR = -1.0 * CalcVector(sideway - input1, input2 - forward);
      }
      outputL = abs(outputR);
      if (input2 > forward) {
        outputL = CalcSide(sideway - input1, outputL * 2.0);
      } else {
        outputL = -1.0 * CalcSide(sideway - input1, outputL * 2.0);
      }
    }
    
    if (outputL < -20) {
      digitalWrite(2, LOW);
      digitalWrite(3, HIGH);
    }
    if (outputL > 20) {
      digitalWrite(2, HIGH);
      digitalWrite(3, LOW);
    }
    if (outputR < -20) {
      digitalWrite(4, LOW);
      digitalWrite(7, HIGH);
    }
    if (outputR > 20) {
      digitalWrite(4, HIGH);
      digitalWrite(7, LOW);
    }
    if (abs(outputL) < 20) {
      digitalWrite(2, LOW);
      digitalWrite(3, LOW);
      outputL = 0;
    }
    if (abs(outputR) < 20) {
      digitalWrite(4, LOW);
      digitalWrite(7, LOW);
      outputR = 0;
    }
    if (abs(outputL) > 255) {
      outputL = 255;
    }
    if (abs(outputR) > 255) {
      outputR = 255;
    }
    outputL = abs(outputL);
    outputR = abs(outputR);

    analogWrite(6, outputL);
    analogWrite(5, outputR);
    Serial.print(outputL);
    Serial.print(" ");
    Serial.println(outputR);
  }
}

double CalcVector(double x, double y) {
  double output = sqrt(x * x + y * y) / 2.0;
  if (output > 255.0)
    output = 255.0;
  return output;
}

double CalcSide(double x, double vector) {
  vector /= 2;
  x /= 2;
  return vector - x * vector / 255;

}


