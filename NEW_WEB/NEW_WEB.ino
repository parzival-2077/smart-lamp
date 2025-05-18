#define AP_SSID "DELL155_plus_1"
#define AP_PASS "AUwCX2uV"

#include <Arduino.h>
#include <GyverHub.h>
GyverHub hub;
bool flag = false;
void btn_cb(){
  flag = !flag;
}
void build(gh::Builder& b) {
  //gh::Row r(b);
  if (b.beginRow()) {
    b.Button().attach(btn_cb);
    b.endRow();
  }

}
void setup() {
  Serial.begin(115200);

#ifdef GH_ESP_BUILD
  // подключение к роутеру
  WiFi.mode(WIFI_STA);
  WiFi.begin(AP_SSID, AP_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println(WiFi.localIP());

  // если нужен MQTT - подключаемся
  hub.mqtt.config("test.mosquitto.org", 1883);
  // hub.mqtt.config("test.mosquitto.org", 1883, "login", "pass");

  // ИЛИ

  // режим точки доступа
  // WiFi.mode(WIFI_AP);
  // WiFi.softAP("My Hub");
  // Serial.println(WiFi.softAPIP());    // по умолч. 192.168.4.1
#endif

  // указать префикс сети, имя устройства и иконку
  hub.config(F("MyDevices"), F("ESP"), F(""));

  // подключить билдер
  hub.onBuild(build);

  // запуск!
  hub.begin();

  pinMode(5, INPUT_PULLUP);
  pinMode(12, OUTPUT);
  attachInterrupt(5, btnInp, FALLING); 
}

void loop() {
  hub.tick();

  // =========== ОБНОВЛЕНИЯ ПО ТАЙМЕРУ ===========
  // в библиотеке предусмотрен удобный класс асинхронного таймера
  static gh::Timer tmr(1000);  // период 1 секунда

  // каждую секунду будем обновлять заголовок
  if (tmr) {
    hub.update(F("title")).value(millis());
  }
  digitalWrite(12, flag);
}

void btnInp() {  // Обработка кнопки
  flag = !flag;
}

