#include "SdFat.h"  // подключаем библотеку работы с SD картой.




#define WIFI_SSID "Mi"
#define WIFI_PASS "9177742926"
#define BOT_TOKEN "5736288863:AAHDkJKYvW-wlKYajFdyY32ckYJTytUSmd8"
#include <FastBot.h>
#include "EEPROM.h" // подключаем энергонезависимую память.

FastBot bot(BOT_TOKEN);


#define MY_PERIOD 3000
#define MY_PERIOD_1 10000
#define MY_PERIOD_2 5000

//int test = 0;
uint32_t tmr_1,tmr_2,tmr_3,tmr_4;

byte count_1 = 0;
byte count_2 = 0;
byte count_3 = 0;
bool flag = true;
bool flag_1 = false;
bool flag_2 = false;
float num_1 = 0;
float num_2 = 0;
uint16_t number;
uint16_t lastNum;
uint8_t waterDet = D2;

uint8_t waterSens = D1;                                         // пин подключенный к датчику расхода воды (water flow sensor).
uint32_t varTime;                                               // Объявляем переменную для хранения времени последнего расчёта.
float varQ;                                                     // Объявляем переменную для хранения рассчитанной скорости потока воды (л/с).
float varV;                                                     // Объявляем переменную для хранения рассчитанного объема воды (л).
volatile uint16_t varF;                                         // Объявляем переменную для хранения частоты импульсов (Гц).


void ICACHE_RAM_ATTR myIsr (){varF++;}                          // Определяем функцию, которая будет приращать частоту импульсов.

String lit;

SdFat32 sd;
File32 file;

// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else  // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN

// Try max SPI clock for an SD. Reduce SPI_CLOCK if errors occur.
#define SPI_CLOCK SD_SCK_MHZ(50)

// Try to select the best SD card configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif  ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
#endif  // HAS_SDIO_CLASS

void setup(){ 
    
  connectWiFi();
  bot.attach(newMsg);
  EEPROM.begin(100);

  if (!sd.begin(SD_CONFIG)) {
    bot.sendMessage("SD карта не обнаружена!", "609747937"); 
  }

  if (!sd.exists("SWFLog.csv")) {
    bot.sendMessage("файл записи не обнаружен!", "609747937"); 
  }

  if (!file.open("SWFLog.csv", FILE_WRITE)) {
    bot.sendMessage("открыть файл на SD карте не удалось!", "609747937");
  }

  file.close();


  pinMode(waterSens, INPUT);                                    // Конфигурируем вывод к которому подключён датчик, как вход.
  uint8_t intSensor = digitalPinToInterrupt(waterSens);         // Определяем номер прерывания который использует вывод waterSens.
  attachInterrupt(intSensor, myIsr, RISING);                    // Назначаем функцию myIsr как обработчик прерываний intSensor при каждом выполнении условия RISING - переход от 0 к 1.
  if(intSensor<0){Serial.print("Указан вывод без EXT INT");}    // Выводим сообщение о том, что датчик подключён к выводу не поддерживающему внешнее прерывание.
  varTime=0; varQ=0; varV=EEPROM.get(0, varV); varF=0;         // Обнуляем все переменные.


}

  void newMsg(FB_msg& msg) {
    //Serial.println(msg.replyText);
    Serial.println(msg.toString());
  //FB_Time t(msg.unix, 5);
  //bot.sendMessage(msg.text, msg.chatID);
  //bot.replyMessage("Hello!", 189, "609747937");
  //bot.sendMessage(t.timeString());
  //Serial.print(t.timeString());
  //Serial.print(' ');
  //Serial.println(t.dateString());
  //Serial.println(msg.text);
  if (msg.text == "/zero"){
    EEPROM.put(0, 0);
    EEPROM.commit();
    varV = 0;
    bot.editMessage(222, lit,"609747937");
    bot.sendMessage("Cчетчик воды и EEPROM обнулены!", "609747937");
  }

  if (msg.text == "/insert"){
    number = msg.messageID;
    bot.sendMessage("Введите значение в формате: 'хххх.хх'","609747937");
    Serial.println(number);
    lastNum = number+2;
    Serial.println(lastNum);
    
    //bot.replyMessage("что то", 497, "609747937"); 
  }

  if (msg.messageID == lastNum){
    //Serial.println(msg.text);
    String str = msg.text;
    //Serial.println(str);
    varV = str.toFloat();
    bot.editMessage(222, lit,"609747937");
    Serial.println(lit);
    bot.sendMessage("Введённые значения зафиксированы!","609747937");
    //Serial.println(varV);
  }
}





void loop() {
  bot.tick();


  if((varTime+1000) < millis() || varTime > millis()){       // Если c момента последнего расчёта прошла 1 секунда, или произошло переполнение millis то ...
   
     varQ    = varF / ((float)varF*5.9f+4570.0f);           // Определяем скорость потока воды л/с.
     varF    = 0;                                           // Сбрасываем частоту импульсов датчика, значение этой переменной приращается по прерываниям.
     varTime = millis();                                    // Сохраняем  время последних вычислений.
     varV   += varQ;                                        // Определяем объем воды л.
     lit =  ((String) "Объем: "+varV+"л");
     Serial.println((String) "Объем "+varV+"л, скорость "+(varQ*60.0f)+"л/м.");
     }


  
  if (millis() - tmr_2 >= 10000){
    if (varQ != 0){
      count_1 ++;
      flag = false;
      }else{ count_1 = 0;}
    tmr_2 = millis();
    Serial.print("Счётчик: ");
    Serial.println(count_1);
  }

  if (count_1 == 6){
    count_1 = 0;
    bot.sendMessage("Клапан открыт больше минуты", "609747937"); 
  }
    

  if (varQ == 0 && !flag){
    if (millis() - tmr_1 >= 5000 ) {
    bot.editMessage(222, lit,"609747937");
    Logg(); 
    Serial.println("Данные отправлены в телеграм");
    flag = true;
    count_3 ++;
    tmr_1 = millis();
    }
  }

  if (count_3 == 10){
    EEPROM.put(0, varV);
    EEPROM.commit();
    Serial.println("Данные в EEPROM записаны");
    Serial.println(EEPROM.get(0, varV));
    count_3 = 0;
  }


  if (millis() - tmr_3 >= 5000 ){
    Serial.print("Датчик сухой: ");
    Serial.println(digitalRead(waterDet));
    tmr_3 = millis();

    if (digitalRead(waterDet)!= 1){
      count_2 ++;
    }else{count_2 = 0;}

    if (count_2 > 5){
      bot.sendMessage("Обнаружена протечка!", "609747937");
      count_2 = 0;
    }
  }

  if (varV > 7000.00){                                                          //  при достижении обьема воды более 7000 литров.
    if (millis() - tmr_4 >= 86400000 ){                                         //  отправка сообшения раз в сутки.
      bot.sendMessage("Необходимо заменить картриджы!", "609747937");
      tmr_4 = millis();
    }
  }
  

}



void Logg(){
   if (file.open("SWFLog.csv", FILE_WRITE)){
     FB_Time t = bot.getTime(5);
     file.print(t.dateString());
     file.print(", ");
     file.print(t.timeString());
     file.print(", ");
     file.print(varV);
     file.print(".\r\n");
     Serial.println("Записано на SD");
     }else{
     bot.sendMessage("Ошибка открытия SWFLog.csv", "609747937"); 
     }
   file.flush();
   file.close();
}


void connectWiFi() {
  delay(2000);
  Serial.begin(115200);
  Serial.println();
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() > 15000) ESP.restart();
  }
  Serial.println("Connected");
  bot.sendMessage("Я в сети!", "609747937");
}
