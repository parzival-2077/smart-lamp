/*
Release: 22.04.2025
-------------------
[+] Преписана логика обработки плавной смены яркости относительно освещения
[+] Добавлено включение и выключение по удержанию 

Release: 27.04.2025
-------------------
[+] Полный рефакторинг кода
[+] Добавлено определение наличие человека через HC-SR04
[=] Обновленны коментариии, почистил код
[-] logicButton() закоменнитрован из-за вызова артефактов при удержании
[!] Вылезла фича/баг при удержании для выхода из выключенного состояния перескакивает на режим
[?] Добавить изменение яркости RGB режима
[?] Добавить по двойному нажатию на энкодер автоматическое проигрования цветового колеса
[?] Подготовить код в интеграции в веб

*/

#define PIN_BTN 5     //пин кнопки
#define PIN_WHITE 13  //пин для белой ленты
#define RPIN 2        //пины для RGB
#define GPIN 4
#define BPIN 15
int leds[] = { 12, 14, 27, 26 };  // пины светодиодов
int state = 0;                    //режим

//для энкодера
#define CLK 23
#define DT 22
#define SW 21

#include "GyverEncoder.h"  //библиотека энкодера
Encoder enc1(CLK, DT, SW);
int value = 0;  // положение энкодера
int hue = 0;    //положение по палитре hue

bool flag = false;
uint32_t btnTimer = 0;

uint32_t checkTimer = 0;
int lastBrightness = 10;
const int threshold = 15;       // Порог чувствительности
const int smoothDelay = 50;     // Задержка для плавности (мс)
const int minBrightness = 10;   // Минимальная яркость
const int maxBrightness = 255;  // Максимальная яркость

bool lightEnabled = true;  //флаг света для удержания
int savedState = 0;        //последний сохраненный режим
bool distanceControlEnabled = true;

#include <NewPing.h>
#define PIN_TRIG 19
#define PIN_ECHO 18
#define MAX_DISTANCE 200  // максимальное расстояние
#define SMALL_DISTANCE 30
#define DEBOUNCE_TIME 2000   // Время стабильности для переключения
#define SHUTOFF_DELAY 30000  // время для отключения света
NewPing sonar(PIN_TRIG, PIN_ECHO, MAX_DISTANCE);
bool ledState = false;
unsigned long lastSmallDistanceTime = 0;
unsigned long lastLargeDistanceTime = 0;
bool wasRecentlySmall = false;

void setup() {
  Serial.begin(9600);

  enc1.setType(TYPE2);  //тип энкодера
  enc1.setFastTimeout(40);

  pinMode(PIN_BTN, INPUT_PULLUP);  //кнопка

  for (int i = 0; i < 4; i++) pinMode(leds[i], OUTPUT);

  pinMode(RPIN, OUTPUT);
  pinMode(GPIN, OUTPUT);
  pinMode(BPIN, OUTPUT);
  pinMode(PIN_WHITE, OUTPUT);
}


void loop() {
  checkDistance();  //проверка наличия человека
  //logicButton();  //обработка кнопки
  bool btnState = digitalRead(PIN_BTN);
  if (btnState && !flag && millis() - btnTimer > 100) {  //нажатие
    flag = true;
    btnTimer = millis();

    if (lightEnabled) {  // только если свет включен
      digitalWrite(leds[state], LOW);
      state++;
      if (state >= 4) state = 0;
      savedState = state;  // сохраняем текущий режим
    }
  }

  // Обработка удержания кнопки (включение/выключение света)
  if (btnState && flag && millis() - btnTimer > 1000) {  // удержание 1 секунда
    lightEnabled = !lightEnabled;                        // переключаем состояние
    flag = false;
    btnTimer = millis();

    if (lightEnabled) {
      state = savedState;  // восстанавливаем сохраненный режим
    }
    delay(100);
  }

  // Обработка отпускания кнопки
  if (!btnState && flag) {
    flag = false;
    btnTimer = millis();
  }

  if (lightEnabled && distanceControlEnabled) {

    digitalWrite(leds[state], HIGH);  //отображение режима

    switch (state) {
      case 0: autoMode(); break;    //авторежим
      case 1: manualMode(); break;  //ручной режим
      case 2: maxMode(); break;     //максимальная яркость
      case 3: rgbMode(); break;     //RGB режим
    }
  } else ledOff();
}


/*void logicButton() {
  bool btnState = digitalRead(PIN_BTN);
  if (btnState && !flag && millis() - btnTimer > 100) {  //нажатие
    flag = true;
    btnTimer = millis();

    if (lightEnabled) {  // только если свет включен
      digitalWrite(leds[state], LOW);
      state++;
      if (state >= 4) state = 0;
      savedState = state;  // сохраняем текущий режим
    }
  }

  // Обработка удержания кнопки (включение/выключение света)
  if (btnState && flag && millis() - btnTimer > 1000) {  // удержание 1 секунда
    lightEnabled = !lightEnabled;                        // переключаем состояние
    flag = false;
    btnTimer = millis();

    if (lightEnabled) {
      state = savedState;  // восстанавливаем сохраненный режим
    }
    delay(100);
  }

  // Обработка отпускания кнопки
  if (!btnState && flag) {
    flag = false;
    btnTimer = millis();
  }
}*/


void autoMode() {
  analogWrite(13, lastBrightness);  // Всегда используем lastBrightness для плавности
  analogWrite(2, 0);
  analogWrite(4, 0);
  analogWrite(15, 0);

  if (millis() - checkTimer >= smoothDelay) {  // Проверка с задержкой для плавности
    checkTimer = millis();

    // Чтение и нормализация значения
    int rawValue = analogRead(25);
    int sensorValue = map(max(rawValue, 100), 100, 4095, minBrightness, maxBrightness);
    sensorValue = constrain(sensorValue, minBrightness, maxBrightness);

    // Плавное изменение
    if (sensorValue != lastBrightness) {
      int step = (sensorValue > lastBrightness) ? 1 : -1;
      lastBrightness += step;
      lastBrightness = constrain(lastBrightness, minBrightness, maxBrightness);
    }
    //для отладки
    // Serial.print("Датчик: ");
    // Serial.print(rawValue);
    // Serial.print(" -> Яркость: ");
    // Serial.print(sensorValue);
    // Serial.print(" | Текущая: ");
    // Serial.println(lastBrightness);
  }
}

void manualMode() {
  enc1.tick();

  if (enc1.isRight()) value++;  // если был поворот направо, увеличиваем на 1
  if (enc1.isLeft()) value--;   // если был поворот налево, уменьшаем на 1

  if (enc1.isFastR()) value += 10;  // если был быстрый поворот направо, увеличиваем на 10
  if (enc1.isFastL()) value -= 10;  // если был быстрый поворот налево, уменьшаем на 10
  // if (enc1.isTurn()) {              // если был совершён поворот (индикатор поворота в любую сторону)
  //   Serial.println(value);          // выводим значение при повороте
  // }
  if (value >= 255) value = 254;
  if (value <= 0) value = 1;
  lastBrightness = value;
  analogWrite(13, lastBrightness);
}

void maxMode() {
  lastBrightness = 255;
  analogWrite(13, lastBrightness);
}

void rgbMode() {
  analogWrite(13, 0);
  enc1.tick();

  if (enc1.isRight()) hue += 10;  // если был поворот направо, увеличиваем на 1
  if (enc1.isLeft()) hue -= 10;   // если был поворот налево, уменьшаем на 1

  if (enc1.isFastR()) hue += 30;  // если был быстрый поворот направо, увеличиваем на 10
  if (enc1.isFastL()) hue -= 30;  // если был быстрый поворот налево, уменьшаем на 10
  // if (enc1.isTurn()) {              // если был совершён поворот (индикатор поворота в любую сторону)
  //   Serial.println(hue);          // выводим значение при повороте
  // }
  if (hue >= 1530) hue = 1530;
  if (hue <= 0) hue = 0;
  colorWheel(hue);
}

void ledOff() {
  digitalWrite(leds[state], LOW);
  analogWrite(13, 0);
  analogWrite(2, 0);
  analogWrite(4, 0);
  analogWrite(15, 0);
}
void colorWheel(int color) {  //цветовое колесо по hue
  byte r, g, b;
  if (color <= 255) {  // красный макс, зелёный растёт
    r = 255;
    g = color;
    b = 0;
  } else if (color > 255 && color <= 510) {  // зелёный макс, падает красный
    r = 510 - color;
    g = 255;
    b = 0;
  } else if (color > 510 && color <= 765) {  // зелёный макс, растёт синий
    r = 0;
    g = 255;
    b = color - 510;
  } else if (color > 765 && color <= 1020) {  // синий макс, падает зелёный
    r = 0;
    g = 1020 - color;
    b = 255;
  } else if (color > 1020 && color <= 1275) {  // синий макс, растёт красный
    r = color - 1020;
    g = 0;
    b = 255;
  } else if (color > 1275 && color <= 1530) {  // красный макс, падает синий
    r = 255;
    g = 0;
    b = 1530 - color;
  }
  analogWrite(RPIN, 255 - r);
  analogWrite(GPIN, 255 - g);
  analogWrite(BPIN, 255 - b);
}

void checkDistance() {
  static uint32_t distanceCheckTimer = 0;
  if (millis() - distanceCheckTimer >= 50) { // Проверяем расстояние каждые 50 мс
    distanceCheckTimer = millis();
    
    unsigned int distance = sonar.ping_cm();
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    bool isSmallNow = (distance > 0 && distance < SMALL_DISTANCE);

    if (isSmallNow) {
      lastSmallDistanceTime = millis();
      wasRecentlySmall = true;
      if (!lightEnabled && distanceControlEnabled) {
        lightEnabled = true;
        state = savedState;
        Serial.println("Light ON (обнаружено близкое расстояние)");
      }
    } 
    else {
      // Если расстояние большое, запоминаем время последнего большого расстояния
      if (wasRecentlySmall) {
        lastLargeDistanceTime = millis();
      }
    }

    // Если было близкое расстояние, но прошло больше DEBOUNCE_TIME с последнего обнаружения
    if (wasRecentlySmall && (millis() - lastSmallDistanceTime > DEBOUNCE_TIME)) {
      wasRecentlySmall = false;
      Serial.println("Расстояние стабильно большое (дебаунс пройден)");
    }

    // Если свет был включен, но прошло SHUTOFF_DELAY с момента последнего большого расстояния
    if (lightEnabled && distanceControlEnabled && !wasRecentlySmall && (millis() - lastLargeDistanceTime > SHUTOFF_DELAY)) {
      lightEnabled = false;
      Serial.println("Light OFF (прошло время без близких объектов)");
    }
  }
}