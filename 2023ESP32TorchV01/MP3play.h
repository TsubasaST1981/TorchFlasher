#include "AudioFileSourceSPIFFS.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

//mp3再生設定
AudioGeneratorMP3 *mp3;
AudioFileSourceSPIFFS *file;
AudioOutputI2S *out;

//MP3ファイル名を再生待ちにセット
void playMP3(char *filename, bool NonStopflg = false, bool Nextflg = false) {
  if (NonStopSound == false) {
    NonStopSound = NonStopflg;
    if (Nextflg == true) PlayFileName[1] = filename;
    else PlayFileName[0] = filename;
  }
}

//サブループMP3
void Core0task(void *args) {
  //サウンド設定
  SPIFFS.begin();
  out = new AudioOutputI2S(0, 0);
  out->SetPinout(26, 25, 22);
  out->SetOutputModeMono(true);
  out->SetGain((float)SoundGain * 0.5);
  mp3 = new AudioGeneratorMP3();

  while (1) {
    if (PlayFileName[0] != "") {
      if (mp3->isRunning()) {
        mp3->stop();
        delete mp3;
        mp3 = new AudioGeneratorMP3();
      }
      file = new AudioFileSourceSPIFFS(PlayFileName[0]);
      if (!file) ;
      else mp3->begin(file, out);
      PlayFileName[0] = "";
    } else if (mp3->isRunning()) {
      if (!mp3->loop()) {
        mp3->stop();
        delete mp3;
        mp3 = new AudioGeneratorMP3();
      }
    } else if ((PlayFileName[0] == "") && (PlayFileName[1] != "")) {
      PlayFileName[0] = PlayFileName[1];
      PlayFileName[1] = "";
    } else {
      NonStopSound = false;
    }

    vTaskDelay(1);

    //スローメモリ 起動時間計測
    bootingSecbuff = (millis() - bootingStartMill) / 1000;
    if (bootingSecbuff > 300) bootingStartMill = millis();
    else if (bootingSecbuff > 60) {
      bootingMin += bootingSecbuff / 60;
      bootingStartMill = millis() - ((bootingSecbuff % 60) * 1000);
      if (TestMode == true) {
        if ((bootingMin % 30) == 0) {
          BriMill = millis();
          Gainflg = false;
          playMP3(mp3File[38 + (bootingMin / 30)]);
        }
      }
    }
  }
}


//チェンジサウンド各種
void StrNowSound() {
  playMP3(mp3File[61 + strnowcnt]);
}
void ImageNowSound() {
  playMP3(mp3File[10 + PictureNo]);
}
void ModeSound() {
  if ((MemoryMode == true) && (MemoryEdit == false)) {
    playMP3(mp3File[37 + MemoryNo]);
    playMP3(mp3File[52 + ModeNo], false, true);
  } else {
    playMP3(mp3File[52 + ModeNo]);
  }
}
void MoziHalfSound() {
  if (MoziHalfMode == true) playMP3(mp3File[18]);
  else playMP3(mp3File[17]);
}
