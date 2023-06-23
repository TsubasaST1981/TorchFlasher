#include "AudioFileSourceLittleFS.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

//mp3再生設定
AudioGeneratorMP3 *mp3;
AudioFileSourceLittleFS *file;
AudioOutputI2S *out;

//MP3ファイル名を再生待ちにセット
void playMP3(char *filename, bool NonStopflg = false, bool Nextflg = false) {
  if (NonStopSound == false) {
    NonStopSound = NonStopflg;
    if (Nextflg == true) PlayFileName[1] = filename;
    else PlayFileName[0] = filename;
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
