//Wifi設定　　http://TorchFlasher.local/
char DeviceName[50] = "TorchFlasher";
char ssid[50] = "TorchFlasher";
char password[50] = "0123";
char ssidlocal[50] = "TorchFlasher";
char passwordlocal[50] = "0123";
#define LOCAL_PORT  52684  // 自分のポート番号
#define REMOTE_PORT 52684  // 相手のポート番号

// IPアドレス
IPAddress localIP;  // 自分のIPアドレス
IPAddress remoteIP; // 相手のIPアドレス


//NeoPixel設定
#define PIXEL_PIN   19    // Digital IO pin connected to the NeoPixels.
#define NUM_LEDS   40    // Digital IO pin connected to the button.
CRGBArray<NUM_LEDS> leds;

const int SleepSec = 1800;
const int VOLTMAX = 4200;
const int VOLTMIN = 3250;
const uint8_t BRIGHTNESSMIN = 30;

//Wifi定義
IPAddress apIP(192, 168, 1, 100);
WebServer Server(80);
// UDPオブジェクト
static WiFiUDP udp;
IPAddress castIP(192, 168, 1, 255);

// scan SSID
#define SSIDLIMIT 30
String ssid_rssi_str[SSIDLIMIT];
String ssid_str[SSIDLIMIT];

//変数各種
uint8_t ModeNo = 1;
#define JyroMode  0
#define NorMode   1
#define BunAMode  2
#define BunBMode  3
#define MoziAMode 4
#define MoziBMode 5
#define JpegMode  6
#define CDownMode 7
#define TimeMode  8
int ColorNo = 0;
#define NoWhite   272
#define NoColor   289
int LismWidthMill = 500; //リズム幅
uint8_t Brightness = 255;
unsigned long BriMill = 0;
unsigned long EEPMill = 1;
int AutoColorNo = 0;          //オートカラー位置
bool AutoColorChgFlg = false; //オートカラー変更フラグ
unsigned long LowPowerMill = 0;   //省エネモード設定
float GyroSUM = 0;
float GyroZ = 0;
bool Gyroflg = false;
unsigned long FlashStartMill = 0;
unsigned long FlashTotalMill = 2000;
bool PowerSaveFlg = false;
unsigned long Sleepmill = 0;
unsigned long BatteryMill = 0;
uint8_t SoundGain = 3;
bool Gainflg = false;
bool NonStopSound = false;
char *PlayFileName[2] = {"", ""}; //0 = 通常再生　1 = 次に再生　2 = 送信バッファ
bool bleflg = false;
uint8_t strnowcnt = 0;
word strbuf[100][16];
int stringcnt = 255;
int strlencnt;
int lastcol = 16;
unsigned long strDispMill = 0;
unsigned long strStartWaitMill = 0;
unsigned long PictDispMill = 0;
bool Mode0upflg = false;
int Mode0upSpeed;
unsigned long LastChangeMill = 0;
unsigned long CountDowmMill = 0;
uint8_t CountDownMax = 10;
int CountDownNowOld = 0;
bool MPUflg = true;
bool MemoryMode = false;
uint8_t MemoryNo = 1;
bool MemoryEdit = false;
bool KakusiModeflg = false;
uint8_t PictureNo = 0;
uint8_t LEDinsertMap[4][40];  //状態表示用バッファ
uint8_t ModeNo_old;
uint8_t MemoryNo_old;
int ColorNo_old;
uint8_t strnowcnt_old;
bool MemoryMode_old;
bool MemoryEdit_old;
uint8_t PictureNo_old;
bool MoziHalfMode = false;
int btn2pushcnt = 0;
unsigned long btn2lastmill = 0;
uint8_t WifilocalMode = 0;
bool WifiClientSetup = false;
unsigned long LastSendUdpMill = 0;
uint8_t UdpReturnCheck = 0;
bool ModeChangeFlg = false;
bool MemoryChangeFlg = false;

//メモリーモード記録ストラクチャー
struct MemoryS {
  uint8_t ModeNo;
  int ColorNo;
  uint8_t strnowcnt;
  bool MoziHalfMode;
  uint8_t CountDownMax;
  uint8_t PictureNo;
};

struct MemoryS MemoryData[11];

//EEPROM設定
#define EEP_ADRS0 0
#define EEP_ADRS1 EEP_ADRS0 + sizeof(Ver)
#define EEP_ADRS2 EEP_ADRS1 + sizeof(ModeNo)
#define EEP_ADRS3 EEP_ADRS2 + sizeof(ColorNo)
#define EEP_ADRS4 EEP_ADRS3 + sizeof(Brightness)
#define EEP_ADRS5 EEP_ADRS4 + sizeof(LismWidthMill)
#define EEP_ADRS6 EEP_ADRS5 + sizeof(SoundGain)
#define EEP_ADRS7 EEP_ADRS6 + sizeof(strnowcnt)
#define EEP_ADRS8 EEP_ADRS7 + sizeof(stringdata)
#define EEP_ADRS9 EEP_ADRS8 + sizeof(DeviceName)
#define EEP_ADRS10 EEP_ADRS9 + sizeof(MoziHalfMode)
#define EEP_ADRS11 EEP_ADRS10 + sizeof(CountDownMax)
#define EEP_ADRS12 EEP_ADRS11 + sizeof(MemoryMode)
#define EEP_ADRS13 EEP_ADRS12 + sizeof(MemoryNo)
#define EEP_ADRS14 EEP_ADRS13 + sizeof(MemoryData)
#define EEP_ADRS15 EEP_ADRS14 + sizeof(PictureNo)
#define EEP_ADRS16 EEP_ADRS15 + sizeof(ssid) + 10
#define EEP_ADRS17 EEP_ADRS16 + sizeof(password)
#define EEP_ADRS18 EEP_ADRS17 + sizeof(WifilocalMode)


void EEPWrite() {
  if (((EEPMill > 0) && (millis() > (EEPMill + 2000))) || (EEPMill == 1)) {
    EEPROM.put(EEP_ADRS1, ModeNo);
    EEPROM.put(EEP_ADRS2, ColorNo);
    EEPROM.put(EEP_ADRS3, Brightness);
    EEPROM.put(EEP_ADRS4, LismWidthMill);
    EEPROM.put(EEP_ADRS5, SoundGain);
    EEPROM.put(EEP_ADRS6, strnowcnt);
    EEPROM.put(EEP_ADRS7, stringdata);
    EEPROM.put(EEP_ADRS8, DeviceName);
    EEPROM.put(EEP_ADRS9, MoziHalfMode);
    EEPROM.put(EEP_ADRS10, CountDownMax);
    EEPROM.put(EEP_ADRS11, MemoryMode);
    EEPROM.put(EEP_ADRS12, MemoryNo);
    EEPROM.put(EEP_ADRS13, MemoryData);
    EEPROM.put(EEP_ADRS14, PictureNo);
    EEPROM.put(EEP_ADRS15, ssid);
    EEPROM.put(EEP_ADRS16, password);
    EEPROM.put(EEP_ADRS17, WifilocalMode);

    EEPROM.commit();
    EEPMill = 0;
  }
}

void EEPRead() {
  EEPROM.get(EEP_ADRS1, ModeNo);
  EEPROM.get(EEP_ADRS2, ColorNo);
  EEPROM.get(EEP_ADRS3, Brightness);
  EEPROM.get(EEP_ADRS4, LismWidthMill);
  EEPROM.get(EEP_ADRS5, SoundGain);
  EEPROM.get(EEP_ADRS6, strnowcnt);
  EEPROM.get(EEP_ADRS7, stringdata);
  EEPROM.get(EEP_ADRS8, DeviceName);
  EEPROM.get(EEP_ADRS9, MoziHalfMode);
  EEPROM.get(EEP_ADRS10, CountDownMax);
  EEPROM.get(EEP_ADRS11, MemoryMode);
  EEPROM.get(EEP_ADRS12, MemoryNo);
  EEPROM.get(EEP_ADRS13, MemoryData);
  EEPROM.get(EEP_ADRS14, PictureNo);
  EEPROM.get(EEP_ADRS15, ssid);
  EEPROM.get(EEP_ADRS16, password);
  EEPROM.get(EEP_ADRS17, WifilocalMode);
}

//メモリーからの設定読み込み
void MemoryDisplay() {
  ModeNo = MemoryData[MemoryNo].ModeNo;
  ColorNo = MemoryData[MemoryNo].ColorNo;
  strnowcnt = MemoryData[MemoryNo].strnowcnt;
  MoziHalfMode = MemoryData[MemoryNo].MoziHalfMode;
  CountDownMax = MemoryData[MemoryNo].CountDownMax;
  PictureNo = MemoryData[MemoryNo].PictureNo;
}
