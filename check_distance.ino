#include <NewPing.h>

#define PIN_TRIG 19
#define PIN_ECHO 18
#define LED_PIN 13          // Пин светодиода
#define MAX_DISTANCE 200    // Максимальное корректное расстояние (см)
#define SMALL_DISTANCE 30   // Порог "маленького" расстояния (см)
#define DEBOUNCE_TIME 2000  // Время стабильности для переключения (мс)
#define SHUTOFF_DELAY 30000 // 5 минут (300 000 мс) до выключения света

NewPing sonar(PIN_TRIG, PIN_ECHO, MAX_DISTANCE);

bool ledState = false;
unsigned long lastSmallDistanceTime = 0;
unsigned long lastLargeDistanceTime = 0;
bool wasRecentlySmall = false;

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  delay(50);
  unsigned int distance = sonar.ping_cm();

  Serial.print(distance);
  Serial.println(" см");

  bool isSmallNow = (distance > 0 && distance < SMALL_DISTANCE);

  if (isSmallNow) {
    lastSmallDistanceTime = millis();
    wasRecentlySmall = true;
    if (!ledState) {
      digitalWrite(LED_PIN, HIGH);
      ledState = true;
      Serial.println("LED ON (обнаружено близкое расстояние)");
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

  // Если свет был включен, но прошло 5 минут с момента последнего большого расстояния
  if (ledState && !wasRecentlySmall && (millis() - lastLargeDistanceTime > SHUTOFF_DELAY)) {
    digitalWrite(LED_PIN, LOW);
    ledState = false;
    Serial.println("LED OFF (прошло 5 минут без близких объектов)");
  }
}