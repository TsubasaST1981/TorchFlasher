#include <FastLED.h>
#include <EEPROM.h>
#include <NimBLEDevice.h>
//#include <BLEDevice.h>
//#include <BLEServer.h>
//#include <BLEUtils.h>
//#include <BLE2902.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Arduino.h>
//#include "efontEnableJaMini.h"
#include "efontEnableJa.h"
#include "efont.h"
#include <FS.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
//#include <WiFiUdp.h>
#include <WiFiUDP.h>
#include <Update.h>
#include <ArduinoOTA.h>
#include <WiFiAP.h>
#include <JPEGDecoder.h>
#include <time.h>

//Wifi設定　　http://TorchFlasher-xxxx.local/
//基本設定
const uint8_t Ver = 60;
uint8_t ModeNoMAX = 8;
bool TestMode = false; //起動時間測定

//文字表示設定
char stringdata[10][20] = {"文字１", "文字２", "文字３", "文字４", "文字５", "文字６", "文字７", "文字８", "文字９", "文字１０"};

//各種ピンアサイン設定
//#define SDAPIN      GPIO_NUM_22
//#define SCLPIN      GPIO_NUM_23
#define SDAPIN      GPIO_NUM_4
#define SCLPIN      GPIO_NUM_5
#define BTN_PIN0    GPIO_NUM_16
#define BTN_PIN1    GPIO_NUM_17
#define BTN_PIN2    GPIO_NUM_12
#define BTN_PIN2old GPIO_NUM_18
#define BATTERY_PIN GPIO_NUM_32
#define DMOVEPIN    GPIO_NUM_14

//スローメモリ 起動時間計測 テスト用
RTC_DATA_ATTR  long bootingMin = 0;
long bootingSecbuff = 0;
unsigned long bootingStartMill = 0;

//音ファイルリスト
char mp3File[71][30] = {
  "/0_kidou.mp3",
  "/1_02_bri-max.mp3",
  "/2_02_bri-min.mp3",
  "/3_VolumeTyousei.mp3",
  "/4_04_batteryEnd.mp3",
  "/5_04_powersave.mp3",
  "/6_BeamShort.mp3",
  "/7_BeamMid.mp3",
  "/8_BeamLong.mp3",
  "/9_AkarusaTyousei.mp3",
  "/10_Picture1.mp3",
  "/11_Picture2.mp3",
  "/12_Picture3.mp3",
  "/13_Picture4.mp3",
  "/14_Picture5.mp3",
  "/15_error.mp3",
  "/16_TanSyokuMode.mp3",
  "/17_MoziHalf_false.mp3",
  "/18_MoziHalf_true.mp3",
  "/19_Dammy.mp3",
  "/20_WifiAPMode.mp3",
  "/21_WifiConnect.mp3",
  "/22_AllReset.mp3",
  "/23_Sec1.mp3",
  "/24_Sec2.mp3",
  "/25_Sec3.mp3",
  "/26_Sec4.mp3",
  "/27_Sec5.mp3",
  "/28_Sec10.mp3",
  "/29_Sec15.mp3",
  "/30_Sec20.mp3",
  "/31_UpPi.mp3",
  "/32_DownPi.mp3",
  "/33_Pi.mp3",
  "/34_Pu.mp3",
  "/35_WifiUpdate.mp3",
  "/36_SleepStart.mp3",
  "/37_MemoryMode.mp3",
  "/38_Memory1.mp3",
  "/39_Memory2.mp3",
  "/40_Memory3.mp3",
  "/41_Memory4.mp3",
  "/42_Memory5.mp3",
  "/43_Memory6.mp3",
  "/44_Memory7.mp3",
  "/45_Memory8.mp3",
  "/46_Memory9.mp3",
  "/47_Memory10.mp3",
  "/48_KakusiMode.mp3",
  "/49_VolumeMax.mp3",
  "/50_MemoryEdit.mp3",
  "/51_MemorySave.mp3",
  "/52_ZyairoMode.mp3",
  "/53_IppanMode.mp3",
  "/54_BunsinModeA.mp3",
  "/55_BunsinModeB.mp3",
  "/56_MoziModeA.mp3",
  "/57_MoziModeB.mp3",
  "/58_ImageMode.mp3",
  "/59_CountDownMode.mp3",
  "/60_DateTimeMode.mp3",
  "/61_Mozi1.mp3",
  "/62_Mozi2.mp3",
  "/63_Mozi3.mp3",
  "/64_Mozi4.mp3",
  "/65_Mozi5.mp3",
  "/66_Mozi6.mp3",
  "/67_Mozi7.mp3",
  "/68_Mozi8.mp3",
  "/69_Mozi9.mp3",
  "/70_Mozi10.mp3"
};

//時間オブジェクト
bool WatchMode = true;
bool WatchGyroflg = true;
uint8_t WatchCnt = 0;
struct tm timeInfo;

#include "DefineMemorys.h"
#include "MP3Play.h"
#include "JpegDec.h"
#include "LEDDisplay.h"

//マルチタスク用
TaskHandle_t thp[1];

//加速度計
Adafruit_MPU6050 mpu;

//電池残量取得
int battvoltget() {
  return analogReadMilliVolts(BATTERY_PIN) * 2;
}

//スリープへ移行
void DeepSleepMode() {
  playMP3(mp3File[36]);
  BatteryDisplay(battvoltget());
  FlashStartMill = millis();
  FlashTotalMill = 500;
  while ((millis() - FlashStartMill) < FlashTotalMill) {
    updownColor(CRGB(255, 0, 0), FlashTotalMill, false);
    vTaskDelay(1);
  }
  mpu.enableSleep(true);
  vTaskDelay(10);
  while (mp3->isRunning()) vTaskDelay(10);
  setColor(CRGB(0, 0, 0));
  digitalWrite(DMOVEPIN, LOW);

  //スローメモリ 起動時間計測
  bootingSecbuff = (millis() - bootingStartMill) / 1000;
  if (bootingSecbuff > 300) bootingStartMill = millis();
  else if (bootingSecbuff > 60) {
    bootingMin += bootingSecbuff / 60;
    bootingStartMill = millis() - ((bootingSecbuff % 60) * 1000);
  }

  //ボタンの押しっぱなしによる再起動回避
  while (digitalRead(BTN_PIN2) == LOW);
  vTaskDelay(50);
  EEPMill = 1;
  EEPWrite();
  esp_sleep_enable_ext0_wakeup(BTN_PIN2, LOW);
  esp_deep_sleep_start();
}

//スリープして即復帰
void RestartESP32() {
  digitalWrite(DMOVEPIN, LOW);
  // タイマー設定
  esp_sleep_enable_timer_wakeup(10);
  // ディープスリープ
  esp_deep_sleep_start();

}
#include "ButtonPush.h"
#include "WifiWeb.h"
#include "BLESetting.h"

//起動セットアップ
void setup() {
  pinMode(BTN_PIN0, INPUT_PULLUP);
  pinMode(BTN_PIN1, INPUT_PULLUP);
  pinMode(BATTERY_PIN, ANALOG);
  pinMode(DMOVEPIN, OUTPUT);
  pinMode(BTN_PIN2, INPUT_PULLUP);
  pinMode(BTN_PIN2old, INPUT);
  //周辺機器電源OFF
  digitalWrite(DMOVEPIN, LOW);

  int BatVolt = battvoltget();
  if (BatVolt < VOLTMIN) {
    //バッテリーが無ければ再度スリープ
    esp_sleep_enable_ext0_wakeup(BTN_PIN2, LOW);
    esp_deep_sleep_start();
  }

  //省電力おまじない
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  //起動判定
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  if (wakeup_reason != ESP_SLEEP_WAKEUP_TIMER) {
    unsigned long KidouMill = millis();
    //ボタンの押しっぱなしによる起動確認
    while ((digitalRead(BTN_PIN2) == LOW) && ((KidouMill + 3000) > millis())) ;
    if ((KidouMill + 3000) > millis()) {
      //おしっぱ起動でなければ再度スリープ
      esp_sleep_enable_ext0_wakeup(BTN_PIN2, LOW);
      esp_deep_sleep_start();
    }
  }

  //周辺機器電源ON
  digitalWrite(DMOVEPIN, HIGH);

  EEPROM.begin(1000);
  uint8_t vercheck;
  EEPROM.get(EEP_ADRS0, vercheck);

  if (vercheck == Ver) {
    EEPRead();
    if (MemoryMode == true) {
      MemoryDisplay();
    }
  }
  else {
    //初回起動初期設定セット
    EEPROM.put(EEP_ADRS0, Ver);
    uint64_t chipid;
    chipid = ESP.getEfuseMac();
    sprintf(DeviceName, "TorchFlasher-%04X", (uint16_t)(chipid >> 32));
    sprintf(ssid, "TorchFlasher-%04X", (uint16_t)(chipid >> 32));

    for (uint8_t i = 1; i <= 10; i++) {
      MemoryData[i].ModeNo = ModeNo;
      MemoryData[i].ColorNo = ColorNo;
      MemoryData[i].strnowcnt = strnowcnt;
      MemoryData[i].CountDownMax = CountDownMax;
      MemoryData[i].MoziHalfMode = MoziHalfMode;
      MemoryData[i].PictureNo = PictureNo;
      if (ModeNo >= MoziBMode) {
        ModeNo = NorMode;
        strnowcnt++;
        MoziHalfMode = !MoziHalfMode;
      } else ModeNo++;
      if (ColorNo >= NoColor) ColorNo = 0;
      else ColorNo += 68;
    }
    ModeNo = 1;
    ColorNo = NoWhite;
    strnowcnt = 0;
    MoziHalfMode = false;
    EEPWrite();
  }

  //文字列を読み込み
  strtobuff(stringdata[strnowcnt]);

  //サウンド設定
  xTaskCreatePinnedToCore(Core0task, "Core0task", 8192, NULL, 7, &thp[0], 0);
  //起動音
  if (wakeup_reason != ESP_SLEEP_WAKEUP_TIMER) playMP3(mp3File[0]);

  //LED初期化
  FastLED.addLeds<NEOPIXEL, PIXEL_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(Brightness);

  BatVolt = battvoltget();
  if (BatVolt < VOLTMIN) DeepSleepMode();
  BatteryDisplay(BatVolt);


  if (wakeup_reason != ESP_SLEEP_WAKEUP_TIMER) {
    //Wifiアップデートモード
    unsigned long UpdateModeMill = millis();
    while ((digitalRead(BTN_PIN0) == LOW) && (digitalRead(BTN_PIN1) == LOW) && (digitalRead(BTN_PIN2) == LOW) && ((millis() - UpdateModeMill) < 3000)) vTaskDelay(1);
    if ((digitalRead(BTN_PIN0) == LOW) && (digitalRead(BTN_PIN1) == LOW) && (digitalRead(BTN_PIN2) == LOW) && ((millis() - UpdateModeMill) >= 3000)) {
      Wifimode();
    }

    //長押し起動で隠しモード起動
    unsigned long KakusiModeMill = millis();
    while ((digitalRead(BTN_PIN2) == LOW) && ((millis() - KakusiModeMill) < 10000)) ;
    if ((digitalRead(BTN_PIN2) == LOW) && ((millis() - KakusiModeMill) >= 10000)) {
      setColor(CRGB(50, 50, 50));
      //隠しモード起動音
      playMP3(mp3File[48]);
      KakusiModeflg = true;
    }
  }

  //Wire起動
  Wire.begin(SDAPIN, SCLPIN);

  //モーションセンサー起動
  unsigned long MPUCheckmill = millis();
  while ((MPUCheckmill + 3000) > millis()) {
    vTaskDelay(100);
    if (mpu.begin()) {
      MPUflg = true;
      mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
      mpu.setGyroRange(MPU6050_RANGE_250_DEG);
      mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
      break;
    } else MPUflg = false;
  }

  //Bluetooth初期化
  BLEinit();

  //画像読み込み
  SPIFFS.begin();
  setColor(CRGB(255, 255, 255));
  Jpegtobuff(jpegFiles[PictureNo]);
  setColor(CRGB(0, 0, 0));

  //モード音声
  while (mp3->isRunning()) vTaskDelay(10);
  ModeSound();

  LastChangeMill = millis();
}

void Wifimode() {
  playMP3(mp3File[35]);
  setColor(CRGB(50, 50, 50));

  //Wifi起動
  uint8_t retry = 0;
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(200); // 200ms
    retry++;
    if (retry > 50) { // 200ms x 50 = 10 sec
      //アクセスポイントモード
      setColor(CRGB(0, 0, 150));

      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      vTaskDelay(100);
      WiFi.mode(WIFI_AP);
      WiFi.softAP(DeviceName);
      vTaskDelay(200);
      WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

      EspOTAsetting();

      //WifiAPモードサーバーセッティング

      Server.on("/", wifimgr_top);
      Server.on("/wifiinput", HTTP_GET, wifiinput);
      Server.on("/wifiset", HTTP_GET, wifiset);
      Server.on("/reboot", reboot);
      Server.on("/doreboot", doreboot);
      Server.on("/textInput", APtextinput);
      Server.on("/StringSet", StringSet);
      Server.on("/allReset", allDataResetcheck);
      Server.on("/allDataReset", allDataReset);

      Server.on("/upOSfile", HTTP_GET, OSfileUp);
      /*handling uploading firmware file */
      Server.on("/update", HTTP_POST, []() {
        Server.sendHeader("Connection", "close");
        Server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
      }, []() {
        HTTPUpload& upload = Server.upload();
        if (upload.status == UPLOAD_FILE_START) {
          Serial.printf("Update: %s\n", upload.filename.c_str());
          if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
            Update.printError(Serial);
          }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
          /* flashing firmware to ESP*/
          if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
          }
        } else if (upload.status == UPLOAD_FILE_END) {
          if (Update.end(true)) { //true to set the size to the current progress
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
          } else {
            Update.printError(Serial);
          }
        }
      });


      Server.onNotFound(wifimgr_top);
      Server.begin();

      playMP3(mp3File[20]);
      ServerLoop();
    }
  }

  //Wifiで時間合わせ
  configTime(9 * 3600L, 0, "ntp.nict.jp", "time.google.com", "ntp.jst.mfeed.ad.jp");//NTPの設定
  uint8_t delaycnt = 0;
  while ((!getLocalTime(&timeInfo)) && (delaycnt < 20)) {
    vTaskDelay(100);
    delaycnt++;
  }

  EspOTAsetting();

  //Wifiセッティング
  Server.on("/", HTTP_GET, RTtextinput);
  Server.on("/textInput", HTTP_GET, RTtextinput);
  Server.on("/allReset", allDataResetcheck);
  Server.on("/allDataReset", allDataReset);
  Server.on("/upOSfile", HTTP_GET, OSfileUp);
  /*handling uploading firmware file */
  Server.on("/update", HTTP_POST, []() {
    Server.sendHeader("Connection", "close");
    Server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = Server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  Server.on("/StringSet", StringSet);
  Server.begin();

  playMP3(mp3File[21]);

  ServerLoop();
}

//Wifi同期操作
int Wifilocalinit() {
  playMP3(mp3File[34]);  //音追加
  setColor(CRGB(50, 50, 50));

  //Wifi起動
  uint8_t retry = 0;
  WiFi.begin(ssidlocal, passwordlocal);
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(100); // 200ms
    retry++;
    if (retry > 50) { // 200ms x 50 = 10 sec
      //Wifi切断
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      vTaskDelay(100);

      //アクセスポイントモード
      WiFi.mode(WIFI_AP);
      WiFi.softAP(ssidlocal);
      vTaskDelay(200);
      WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
      //udp.begin(LOCAL_PORT);
      playMP3(mp3File[20]);
      return 1;
    }
  }
  //udp.begin(LOCAL_PORT);
  playMP3(mp3File[21]);
  return 2;
}

//サーバーモードループ
void ServerLoop() {
  unsigned long Wifimill = millis();
  while ((btnpushmill[0] > 0) || (btnpushmill[1] > 0) || (btnpushmill[2] > 0)) Button_PushGet();
  while (1) {
    Button_PushGet();
    if (btnpushflg[0] == 4) {
      EEPMill = 1;
      EEPROM.put(EEP_ADRS0, 1);
      EEPWrite();
      //オールリセット音
      playMP3(mp3File[22]);
      vTaskDelay(10);
      while (mp3->isRunning()) vTaskDelay(10);
      ESP.restart(); // restart ESP32
    }
    if (btnpushflg[2] == 4) RestartESP32();
    btnpushflg[0] = 0;
    btnpushflg[1] = 0;
    btnpushflg[2] = 0;

    ColorFull();

    ArduinoOTA.handle();
    Server.handleClient();

    vTaskDelay(1);

    if ((millis() - Wifimill) > (30 * 60 * 1000)) DeepSleepMode();
  }
}

//メインループ
void loop() {
  //バッテリー表示＆明るさ・音量設定
  while (millis() < (3000 + BriMill)) BatteryDisplayBrightnessSet();

  //ボタン動作
  Button_Check_Mainloop();

  //バッテリー残量取得
  if (millis() > (BatteryMill + 1000)) {
    BatteryMill = millis();
    int BatVolt = battvoltget();
    if (BatVolt < VOLTMIN) {
      playMP3(mp3File[4]);
      vTaskDelay(10);
      while (mp3->isRunning()) vTaskDelay(10);
      vTaskDelay(200);
      DeepSleepMode();
    }
    //バッテリー残量３分の１以下で省エネモード判定
    if ((PowerSaveFlg == false) && (BatVolt > (VOLTMIN + ((VOLTMAX - VOLTMIN) / 3)))) LowPowerMill = millis();
  }

  //輝度設定
  uint8_t BriNow = Brightness;
  if (BriNow < BRIGHTNESSMIN) BriNow = BRIGHTNESSMIN;
  if ((millis() - LowPowerMill) > (SleepSec * 1000)) {
    BriNow = BriNow / 8;
    if (PowerSaveFlg == false) {
      PowerSaveFlg = true;
      playMP3(mp3File[5], true);
    }
  } else PowerSaveFlg = false;
  FastLED.setBrightness(BriNow);

  //加速度計チェック
  if (MPUflg == true) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    GyroSUM = abs(g.gyro.x);
    GyroSUM += abs(g.gyro.y);
    GyroSUM += abs(g.gyro.z);
    GyroZ = g.gyro.z;
  } else GyroSUM = 8;

  //省エネ動作
  if (GyroSUM > 5) LowPowerMill = millis();

  //ジャイロモード判定
  if ((Gyroflg == false) && (GyroSUM > 6.3)) {
    Gyroflg = true;
    FlashStartMill = millis();
    strStartWaitMill = millis();
    PictDispMill = millis();
    Mode0upSpeed = 1000 - ((GyroSUM - 8) * 25);
  }
  else if ((Gyroflg == true) && (GyroSUM < 5)) {
    Gyroflg = false;
  }

  //LEDをモード設定で切り替え表示
  LEDmodeDisplay();

  //Bluetooth文字登録処理
  if (bleflg) {
    if ((ModeNo < MoziAMode) || (ModeNo > MoziBMode)) ModeNo = 3;
    strtobuff(stringdata[strnowcnt]);
    EEPMill = millis();
    bleflg = false;
  }

  vTaskDelay(1);
}

//バッテリー表示＆明るさ設定モードルーチン
void BatteryDisplayBrightnessSet() {
  //ボタン処理
  BatteryDisplayMode_Button();

  //範囲外処理
  FastLED.setBrightness(Brightness);
  if (TestMode == false) BatteryDisplay(battvoltget());
  else {
    if (Gainflg == false) BatteryDisplay(battvoltget());
    else setColor(CRGB(85, 85, 85), bootingMin / 30);
  }

  vTaskDelay(1);
}
