#define PIN_BUTON 5
int leds[] = {12, 14, 27, 26};
int state = 0;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_BUTON, INPUT_PULLUP);
  for (int i = 0; i < 4; i++)
    pinMode(leds[i], OUTPUT);
}

bool flag = false; // Флаг для отслеживания нажатия кнопки
bool flagHold = false; // Флаг для режима удержания кнопки
bool blinkingMode = false; // Флаг для режима моргания
uint32_t btnTimer = 0; // Таймер для отслеживания времени нажатия кнопки

int ledState = LOW; // Текущее состояние светодиодов (вкл/выкл)
long interval = 500; // Интервал моргания светодиодов (500 мс)
long previousMillis = 0; // Время последнего изменения состояния светодиодов

void loop() {
  // Читаем инвертированное значение для удобства
  bool btnState = digitalRead(PIN_BUTON); // Инвертируем, так как INPUT_PULLUP

  // Обработка нажатия кнопки
  if (btnState && !flag && millis() - btnTimer > 10) {
    flag = true;
    btnTimer = millis();
    Serial.println("press");
  }

  // Обработка удержания кнопки
  if (btnState && flag && millis() - btnTimer > 500) {
    btnTimer = millis();
    Serial.println("press hold");

    if (!flagHold) {
      flagHold = true; // Включаем режим удержания
      Serial.println("Hold mode ON");
    }
  }

  // Обработка отпускания кнопки
  if (!btnState && flag) {
    flag = false;
    btnTimer = millis();
    Serial.println("release");

    if (flagHold) {
      // Если кнопка удерживалась, включаем режим моргания
      blinkingMode = true;
      flagHold = false;
      Serial.println("Blinking mode ON");
    } else {
      // Если кнопка была нажата кратко, переключаем светодиоды
      blinkingMode = false;
      btnInp(); // Переключение светодиодов
      Serial.println("Single press: Running light");
    }
  }

  // Режим моргания светодиодов
  if (blinkingMode) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis > interval) {
      previousMillis = currentMillis;
      ledState = !ledState; // Инвертируем состояние светодиодов
      for (int i = 0; i < 4; i++)
        digitalWrite(leds[i], ledState); // Управляем всеми светодиодами
    }
  } else {
    // Режим бегущего огонька: включаем только текущий светодиод
    for (int i = 0; i < 4; i++)
      digitalWrite(leds[i], (i == state) ? HIGH : LOW);
  }
}

// Обработчик короткого нажатия кнопки
void btnInp() {
  state++;
  if (state >= 4) {
    state = 0; // Переход к первому светодиоду после четвертого
  }
}
// #define PIN_BUTON 5
// int leds[] = { 12, 14, 27, 26 };
// int state = 0;
// void setup() {
//   Serial.begin(9600);
//   pinMode(PIN_BUTON, INPUT_PULLUP);
//   for (int i = 0; i < 4; i++)
//     pinMode(leds[i], OUTPUT);
// }

// bool flag = false;
// bool flagHold = false;
// uint32_t btnTimer = 0;

// int ledState = LOW;
// long interval = 1000;  
// long previousMillis = 0;    
// void loop() {
//   // читаем инвертированное значение для удобства
//   bool btnState = digitalRead(PIN_BUTON);
//   if (btnState && !flag && millis() - btnTimer > 10) {
//     flag = true;
//     btnTimer = millis();
//     btnInp();
//     digitalWrite(leds[state], HIGH);
//     Serial.println("press");
//   }
//   if (btnState && flag && millis() - btnTimer > 500) {
//     btnTimer = millis();
    
//     Serial.println("press hold");
//     if (!flagHold) {
//       unsigned long currentMillis = millis();
//       if (currentMillis - previousMillis > interval) {
//         previousMillis = currentMillis;
//         if (ledState == LOW)
//           ledState = HIGH;
//         else
//           ledState = LOW;
//         for(int i = 0; i < 4; i++)
//           digitalWrite(leds[i], ledState);
//       }
      
//     }

//   }
//   if (!btnState && flag && millis() - btnTimer > 500) {
//     flag = false;
//     btnTimer = millis();
//     Serial.println("release");
//   }
// }
// void allLed() {
// }
// void btnInp() {
//   for (int i = 0; i < 4; i++)
//     digitalWrite(leds[i], LOW);
//   state++;
//   if (state >= 4) {
//     state = 0;
//   }
// }

// #define PIN_BUTON 5
// int leds[] = {12, 14, 27, 26};
// int state = 0;
// void setup() {
//   for(int i = 0; i < 4; i++)
//     pinMode(leds[i], OUTPUT);

//   for(int i = 0; i < 4; i++){
//     digitalWrite(leds[i], HIGH);
//     delay(100);
//     digitalWrite(leds[i], LOW) ;
//   }
//   attachInterrupt(PIN_BUTON, btnInp, FALLING);
//   Serial.begin(9600);
// }

// void loop() {
//   Serial.println(state);
//   digitalWrite(leds[state], HIGH);
// }

// void btnInp(){
//   for(int i = 0; i < 4; i++)
//     digitalWrite(leds[i], LOW);
//   state++;
//   if(state >= 4){
//     state = 0;
//   }
// }
