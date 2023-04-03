//オートカラーモードで色の切り替え
void AutoColorChange() {
  if (AutoColorChgFlg == true) {
    AutoColorNo += 68;
    if (AutoColorNo > 255) AutoColorNo -= 255;
    AutoColorChgFlg = false;
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
CRGB colorChenge(int WheelPos) {
  if (WheelPos >= NoColor) WheelPos = AutoColorNo;
  else if (WheelPos >= NoWhite) return CRGB(85, 85, 85);

  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return CRGB(255 - (WheelPos * 3), 0, WheelPos * 3);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return CRGB(0, WheelPos * 3, 255 - (WheelPos * 3));
  } else {
    WheelPos -= 170;
    return CRGB(WheelPos * 3, 255 - (WheelPos * 3), 0);
  }
}

//状態LED表示
void LEDinsertMapping(uint8_t low, int startcnt, int endcnt) {
  for (int col = startcnt; col < endcnt; col++) {
    LEDinsertMap[low][col] = 170;
    LEDinsertMap[low][39 - col] = 170;
  }
  if ((startcnt - 1) >= 0) {
    LEDinsertMap[low][startcnt - 1] = 10;
    LEDinsertMap[low][39 - (startcnt - 1)] = 10;
  }
  if ((endcnt) < 20) {
    LEDinsertMap[low][endcnt] = 10;
    LEDinsertMap[low][39 - (endcnt)] = 10;
  }
}
void LEDShow() {
  if ((LastChangeMill + 500) > millis()) {
    //状態変更表示違いがあればマッピング開始
    if ((MemoryNo != MemoryNo_old) || (ModeNo != ModeNo_old) || (ColorNo != ColorNo_old) ||
        (strnowcnt != strnowcnt_old) || (MemoryMode != MemoryMode_old) || (MemoryEdit != MemoryEdit_old) || (PictureNo != PictureNo_old)) {
      //マップの初期化
      for (int l = 0; l < 4; l++) {
        for (int i = 0; i < 20; i++) LEDinsertMap[l][i] = 0;
      }
      //マッピング
      if (MemoryNo != MemoryNo_old) LEDinsertMapping(1, (MemoryNo - 1) * 2, ((MemoryNo - 1) * 2) + 2);
      else if (ModeNo != ModeNo_old) LEDinsertMapping(0, (ModeNo * 2) + 2, (ModeNo * 2) + 4);
      else if (ColorNo != ColorNo_old) LEDinsertMapping(2, (ColorNo / 17) + 1, (ColorNo / 17) + 2);
      else if (strnowcnt != strnowcnt_old) LEDinsertMapping(3, (strnowcnt * 2), (strnowcnt * 2) + 2);
      else if (PictureNo != PictureNo_old) LEDinsertMapping(3, (PictureNo * 4), (PictureNo * 4) + 4);
      ModeNo_old = ModeNo;
      MemoryNo_old = MemoryNo;
      ColorNo_old = ColorNo;
      strnowcnt_old = strnowcnt;
      MemoryMode_old = MemoryMode;
      MemoryEdit_old = MemoryEdit;
      PictureNo_old = PictureNo;
    }
    //マップからの表示
    for (int i = 0; i < 20; i++) {
      if ((LEDinsertMap[0][i] != 0) || (LEDinsertMap[1][i] != 0) || (LEDinsertMap[2][i] != 0) || (LEDinsertMap[3][i] != 0)) {
        CRGB c = CRGB(LEDinsertMap[2][i] + (LEDinsertMap[3][i] / 2), LEDinsertMap[0][i] + (LEDinsertMap[3][i] / 2), LEDinsertMap[1][i] + (LEDinsertMap[3][i] / 2));
        leds[i] = c;
        leds[39 - i] = c;
      }
    }
  } else if ((LastChangeMill + 505) > millis()) {
    //マップの初期化
    for (int l = 0; l < 4; l++) {
      for (int i = 0; i < 20; i++) LEDinsertMap[l][i] = 0;
    }
  }
  FastLED.show();
}

//単色塗りつぶし
void setColor(CRGB c, int cnt = NUM_LEDS / 2) {
  for (int i = 0; i < NUM_LEDS / 2; i++) {
    if (i < cnt) {
      leds[i] = c;
      leds[39 - i] = c;
    } else {
      leds[i] = CRGB(0, 0, 0);
      leds[39 - i] = CRGB(0, 0, 0);
    }
  }
  LEDShow();
}

//フルカラーうねうね表示
void ColorFull() {
  unsigned int lismBuf = LismWidthMill;
  unsigned int stklism = millis() % lismBuf;
  byte justcolor = stklism / (lismBuf / 256);

  for (int i = 0; i < 20; i++) {
    byte ColorChgJust = (i * 6) + justcolor;
    CRGB color = colorChenge(ColorChgJust);
    leds[19 - i] = color;
    leds[20 + i] = color;
  }
  LEDShow();
}

//点滅分身表示
void bunsin(bool Colorflg) {
  unsigned int lismBuf = LismWidthMill;
  unsigned int stklism = millis() % lismBuf;
  unsigned int lismtime = lismBuf / 64;
  stklism = stklism % (lismtime * 32);
  byte lismiti = stklism / lismtime;

  if ((lismiti % 4) < 1) {
    if (Colorflg == true) setColor(colorChenge(ColorNo));
    else setColor(colorChenge(AutoColorNo));
    AutoColorChgFlg = true;
  } else {
    setColor(CRGB(0, 0, 0));
    AutoColorChange();
  }
}

//バッテリー残量表示
void BatteryDisplay(int BatVolt) {
  CRGB Bat1 = CRGB(127, 127, 0);
  CRGB Bat2 = CRGB(0, 0, 255);
  if (Gainflg == true) {
    Bat1 = CRGB(0, 0, 255);
    Bat2 = CRGB(127, 127, 0);
  }
  for (int i = 0; i < 20; i++) {
    if ((BatVolt - VOLTMIN) > (((VOLTMAX - VOLTMIN) / 20) * i)) {
      leds[i] = Bat1;
      leds[39 - i] = Bat1;
    } else {
      leds[i] = Bat2;
      leds[39 - i] = Bat2;
    }
  }
  FastLED.show();
}

//文字表示
void stringDisplay(bool Colorflg, bool Hantenflg = false) {
  CRGB truebit;
  CRGB falsebit = CRGB(0, 0, 0);
  if (Colorflg == true) truebit = colorChenge(ColorNo);
  else truebit = colorChenge(AutoColorNo);
  if (Hantenflg == true) {
    falsebit = truebit;
    truebit = CRGB(0, 0, 0);
  }

  if ((strStartWaitMill + 5) > millis()) {
    if (lastcol != -1) {
      setColor(falsebit);
      lastcol = -1;
      stringcnt = -1;
    }
    strDispMill = 0;
  } else {
    int col = (millis() - strDispMill) / (7.5 - (((float)GyroSUM) / 2.5));
    if (col != lastcol) {
      if (col > 15) {
        col = 0;
        strDispMill = millis();
        AutoColorChgFlg = true;
        AutoColorChange();
        falsebit = CRGB(0, 0, 0);
        if (Colorflg == true) truebit = colorChenge(ColorNo);
        else truebit = colorChenge(AutoColorNo);
        if (Hantenflg == true) {
          falsebit = truebit;
          truebit = CRGB(0, 0, 0);
        }
        stringcnt++;
        if (stringcnt >= strlencnt) stringcnt = -1;
      }
      lastcol = col;

      if (stringcnt < 0) setColor(falsebit);
      else {
        leds[0] = falsebit;
        leds[1] = falsebit;
        leds[18] = falsebit;
        leds[19] = falsebit;
        leds[20] = falsebit;
        leds[21] = falsebit;
        leds[38] = falsebit;
        leds[39] = falsebit;

        for (int i = 0; i < 16; i++) {
          if (strbuf[stringcnt][i] & (0x8000 >> col)) {
            if ((MoziHalfMode == false) || (GyroZ > 0)) leds[17 - i] = truebit;
            else leds[17 - i] = falsebit;
            if ((MoziHalfMode == false) || (GyroZ < 0)) leds[22 + i] = truebit;
            else leds[22 + i] = falsebit;
          } else {
            leds[17 - i] = falsebit;
            leds[22 + i] = falsebit;
          }
        }
        LEDShow();
      }
    }
  }
}

//文字列をメモリバッファに展開
void strtobuff(char * pUTF8) {
  uint16_t pUTF16;
  byte buff[32];
  strlencnt = 0;
  while (*pUTF8 != 0x00) {
    pUTF8 = efontUFT8toUTF16(&pUTF16, pUTF8);  // UTF8からUTF16に変換する
    getefontData(buff, pUTF16);  // フォントデータの取得
    for (uint8_t row = 0; row < 16; row++) {
      strbuf[strlencnt][row] = buff[row * 2] * 256 + buff[row * 2 + 1];
    }
    strlencnt++;
  }
}
//インジケーター表示
void updownColor(CRGB c, int TotalMill, bool upflg) {
  int LedCount = 0;
  long nowmill = (millis() - FlashStartMill);
  if (nowmill > TotalMill) LedCount = 20;
  else LedCount = nowmill / (TotalMill / 20);

  if (upflg == false) LedCount = 20 - LedCount;

  for (int i = 0; i < NUM_LEDS / 2; i++) {
    if (i < LedCount) {
      leds[i] = c;
      leds[39 - i] = c;
    } else {
      leds[i] = CRGB(0, 0, 0);
      leds[39 - i] = CRGB(0, 0, 0);
    }
  }
  LEDShow();
}
//ジャイロインジケーター表示
void updownColorGyro(int TotalMill, bool upflg) {
  int LedCount = 0;
  long nowmill = (millis() - FlashStartMill);
  if (nowmill > TotalMill) LedCount = 20;
  else LedCount = nowmill / (TotalMill / 20);

  if (upflg == false) {
    LedCount = 20 - LedCount;
    AutoColorChgFlg = true;
  } else {
    AutoColorChange();
  }

  for (int i = 0; i < NUM_LEDS / 2; i++) {
    if (i < LedCount) {
      leds[i] = colorChenge(AutoColorNo);
      leds[39 - i] = colorChenge(AutoColorNo);
    } else {
      leds[i] = CRGB(0, 0, 0);
      leds[39 - i] = CRGB(0, 0, 0);
    }
  }
  LEDShow();
}

//カウントダウン設定
void CountDownDisplay() {
  CRGB c = colorChenge(ColorNo);

  unsigned int lismBuf = LismWidthMill;
  unsigned int stklism = millis() % lismBuf;
  byte justcolor = stklism / (lismBuf / 256);

  int CountDownTime = CountDownMax - ((millis() - CountDowmMill) / 1000);
  if (CountDownTime < 0) CountDownTime = 0;

  if (CountDownNowOld != CountDownTime) {
    CountDownNowOld = CountDownTime;

    if ((CountDownTime > 0) && (CountDownTime <= 5)) playMP3(mp3File[22 + CountDownTime]);
    else if (CountDownTime == 10) playMP3(mp3File[28]);
    else if (CountDownTime == 15) playMP3(mp3File[29]);
    else if (CountDownTime == 20) playMP3(mp3File[30]);
  }
  for (int i = 0; i < 20; i++) {
    if (((20 / CountDownMax) * CountDownTime) > i) {
      if (ColorNo == NoColor) {
        byte ColorChgJust = (i * 6) + justcolor;
        c = colorChenge(ColorChgJust);
      }

      leds[i] = c;
      leds[39 - i] = c;
    } else {
      leds[i] = CRGB(85, 85, 85);
      leds[39 - i] = CRGB(85, 85, 85);
    }
  }
  LEDShow();
}

//画像表示
void pictureDisplay() {
  uint16_t col = (millis() - PictDispMill) / (7.5 - (((float)GyroSUM) / 2.5));
  if (col != lastcol) {
    if (col > devid.wfbuf) {
      col = 0;
      PictDispMill = millis();
    }
    lastcol = col;
    for (int i = 0; i < 20; i++) {
      leds[19 - i] = CRGB(devid.fbuf[(col * 3) + (devid.wfbuf * i * 3)] / 2, devid.fbuf[(col * 3) + (devid.wfbuf * i * 3) + 1] / 2, devid.fbuf[(col * 3) + (devid.wfbuf * i * 3) + 2] / 2);
      leds[20 + i] = CRGB(devid.fbuf[(col * 3) + (devid.wfbuf * i * 3)] / 2, devid.fbuf[(col * 3) + (devid.wfbuf * i * 3) + 1] / 2, devid.fbuf[(col * 3) + (devid.wfbuf * i * 3) + 2] / 2);
    }
    LEDShow();
  }
}

void TimeCheckSet() {
  if (WatchGyroflg == true) {
    WatchGyroflg = false;
    time_t t;
    struct tm *tm;
    char cbuff[15];

    t = time(NULL);
    tm = localtime(&t);

    if ((millis() % 5000) < 2500) sprintf(cbuff, "%d月%d日", tm->tm_mon + 1, tm->tm_mday);
    else sprintf(cbuff, "%d時%d分%d秒", tm->tm_hour, tm->tm_min, tm->tm_sec);
    strtobuff(cbuff);

    WatchCnt++;
    if (WatchCnt >= 4) WatchCnt = 0;
  }
}

void ModeAblack() {
  //黒表示
  for (int i = 0; i < (NUM_LEDS - 1); i++) {
    leds[i] = CRGB(0, 0, 0);
  }
  int LEDcolor = abs((int)((millis() % 2000) / 200) - 5) - 2;
  if (LEDcolor < 0) LEDcolor = 0;
  leds[39] = CRGB(LEDcolor, LEDcolor, LEDcolor);
  LEDShow();
}

//LEDをモード別に表示
void LEDmodeDisplay() {
  //LED表示
  if (ModeNo == JyroMode) {
    //ジャイロモード
    if (Gyroflg == true) {
      if (GyroSUM > 10) playMP3(mp3File[6], true);
      else playMP3(mp3File[7], true);
      if (ColorNo != NoColor) updownColor(colorChenge(ColorNo), Mode0upSpeed, true);
      else updownColorGyro(Mode0upSpeed, true);
    }
    else {
      if (ColorNo != NoColor) updownColor(colorChenge(ColorNo), 500, false);
      else updownColorGyro(500, false);
    }
  }
  else if (ModeNo == NorMode) {
    //一般モード
    if (ColorNo != NoColor) setColor(colorChenge(ColorNo)); //単
    else ColorFull();
  }
  else if (ModeNo == BunAMode) {
    //分身モードA(普段消灯)
    if ((LastChangeMill + 500) > millis()) {
      if (ColorNo != NoColor) setColor(colorChenge(ColorNo)); //単
      else ColorFull();
    }
    else if (ColorNo != NoColor) {
      if (Gyroflg == true) bunsin(true);   //単色点滅
      else ModeAblack();
    } else {
      if (Gyroflg == true) bunsin(false);  //フルカラー点滅
      else ModeAblack();
    }
  }
  else if (ModeNo == BunBMode) {
    //分身モードB(普段点灯)
    if (ColorNo != NoColor) {
      if (Gyroflg == true) bunsin(true);   //単色点滅
      else setColor(colorChenge(ColorNo)); //単
    } else {
      if (Gyroflg == true) bunsin(false);  //フルカラー点滅
      else ColorFull();
    }
  }
  else if (ModeNo == MoziAMode) {
    //文字表示モードA(普段消灯)
    if ((LastChangeMill + 500) > millis()) {
      if (ColorNo != NoColor) setColor(colorChenge(ColorNo)); //単
      else ColorFull();
    }
    else if (Gyroflg == true) stringDisplay(ColorNo != NoColor);   //単色の文字表示
    else ModeAblack();
  }
  else if (ModeNo == MoziBMode) {
    //文字表示モードB(普段点灯)
    if (Gyroflg == true) stringDisplay(ColorNo != NoColor);   //単色の文字表示
    else if (ColorNo != NoColor) setColor(colorChenge(ColorNo)); //単
    else ColorFull();
  }
  else if (ModeNo == JpegMode) {
    //画像表示モード
    if (Gyroflg == true) pictureDisplay();        //画像表示
    else if (ColorNo != NoColor) setColor(colorChenge(ColorNo)); //単
    else ColorFull();
  }
  else if (ModeNo == CDownMode) {
    //カウントダウンモード
    if (Gyroflg == true) CountDowmMill = millis();
    CountDownDisplay();
  }
  else if (ModeNo == TimeMode) {
    //時間表示モード
    if (ColorNo != NoColor) {
      if (Gyroflg == true) {
        stringDisplay(true);   //単色の文字表示
        WatchGyroflg = true;
      }
      else {
        setColor(colorChenge(ColorNo)); //単
        TimeCheckSet();
      }
    } else {
      if (Gyroflg == true) {
        stringDisplay(false);  //フルカラーの文字表示
        WatchGyroflg = true;
      }
      else {
        ColorFull();
        TimeCheckSet();
      }
    }
  }
}
