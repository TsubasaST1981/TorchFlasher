//ボタン変数
bool btnoldstate[3] = {HIGH, HIGH, HIGH};
unsigned long btnpushmill[3] = {0, 0, 0};
unsigned long btnlongpushmill[3] = {0, 0, 0};
uint8_t btnpushflg[3] = {0, 0, 0};

//ボタン状態取得
void Button_PushGet() {
  bool btnnewstate[3] = {
    digitalRead(BTN_PIN0),
    digitalRead(BTN_PIN1),
    digitalRead(BTN_PIN2)
  };

  unsigned long nowmill = millis();
  for (int i = 0; i < 3; i++) {
    if ((btnnewstate[i] == LOW) && (btnoldstate[i] == HIGH)) btnpushmill[i] = millis();
    else if ((btnnewstate[i] == HIGH) && (btnpushmill[i] > 0)) {
      if ((btnpushmill[i] + 1000) < nowmill) ;
      else if ((btnpushmill[i] + 350) < nowmill) btnpushflg[i] = 2;
      else if ((btnpushmill[i] + 20) < nowmill) btnpushflg[i] = 1;
      btnpushmill[i] = 0;
      btnlongpushmill[i] = 0;
    } else if ((btnnewstate[i] == LOW) && (btnpushmill[i] + 1000) < nowmill) {
      if (btnlongpushmill[i] == 0) {
        btnpushflg[i] = 3;
        btnlongpushmill[i] = nowmill;
      } else if (nowmill > (btnlongpushmill[i] + 2000)) {
        btnpushflg[i] = 4;
        btnlongpushmill[i] = nowmill;
        if ((btnnewstate[0] == LOW) && (btnnewstate[1] == LOW) && (btnnewstate[2] == LOW)) {
          EEPROM.put(EEP_ADRS0, 0);
          DeepSleepMode();
        }
      }
    }
    btnoldstate[i] = btnnewstate[i];
  }

  if ((btnpushflg[0] == 0) && (btnpushflg[1] == 0) && (btnpushflg[2] == 0)) EEPWrite();
  else {
    LastChangeMill = millis();
    EEPMill = millis();
    ModeChangeFlg = true;
  }
}

//バッテリー表示モード
//輝度調整＆音量調整モードのボタン処理
void BatteryDisplayMode_Button() {
  //ボタン状態取得
  Button_PushGet();

  //ボタンが押されていたら処理
  int BtnNo = 0;
  if ((digitalRead(BTN_PIN1) == LOW) && ((btnpushmill[BtnNo] + 20) < millis())) BriMill = millis();
  if (btnpushflg[BtnNo] == 4) bootingMin = 0;
  if (btnpushflg[BtnNo] > 0) {
    if (Gainflg == false) {
      if (Brightness <= 230) Brightness += 25;
      if ((Brightness >= 255) || (btnpushflg[BtnNo] == 3)) {
        Brightness = 255;
        playMP3(mp3File[1]);
      } else playMP3(mp3File[31]);
    } else {
      SoundGain += 1;
      if ((SoundGain >= 5) || (btnpushflg[BtnNo] == 3)) {
        SoundGain = 5;
        out->SetGain((float)SoundGain * 0.5);
        playMP3(mp3File[49]);
      } else {
        out->SetGain((float)SoundGain * 0.5);
        playMP3(mp3File[31]);
      }
    }
  }
  btnpushflg[BtnNo] = 0;

  BtnNo = 1;
  if ((digitalRead(BTN_PIN0) == LOW) && ((btnpushmill[BtnNo] + 20) < millis())) BriMill = millis();
  if (btnpushflg[BtnNo] > 0) {
    if (Gainflg == false) {
      Brightness -= 25;
      if ((Brightness <= BRIGHTNESSMIN) || (btnpushflg[BtnNo] == 3)) {
        Brightness = BRIGHTNESSMIN;
        playMP3(mp3File[2]);
      } else playMP3(mp3File[32]);
    } else {
      if (SoundGain > 0) {
        if (btnpushflg[BtnNo] == 3) SoundGain = 0;
        else SoundGain -= 1;
        out->SetGain((float)SoundGain * 0.5);
        playMP3(mp3File[32]);
      }
    }
  }
  btnpushflg[BtnNo] = 0;

  BtnNo = 2;
  if ((digitalRead(BTN_PIN2) == LOW) && ((btnpushmill[BtnNo] + 20) < millis())) BriMill = millis();
  if (btnpushflg[BtnNo] > 0) {
    if (btnpushflg[BtnNo] == 1) {
      if ((btn2lastmill + 500) > millis()) {
        btn2pushcnt++;
        if (btn2pushcnt >= 4) {
          if (Gainflg == true) Wifimode();
          else {
            WifilocalMode += 1;
            if (WifilocalMode > 2) WifilocalMode = 0;
            EEPMill = 1;
            Wifilocalinit();
          }
          btn2pushcnt = 0;
        }
      }
      else btn2pushcnt = 0;
      btn2lastmill = millis();
    }

    if (btnpushflg[BtnNo] == 3) {
      Gainflg = !Gainflg;
      if (Gainflg == true) playMP3(mp3File[3]);
      else playMP3(mp3File[9]);
    }
    if (btnpushflg[BtnNo] == 4) DeepSleepMode();
  }

  btnpushflg[BtnNo] = 0;

  if (ModeChangeFlg == true) udpModeCast();
  ModeChangeFlg = false;
}

//メインループのボタンチェック
void Button_Check_Mainloop() {
  //ボタン状態取得
  Button_PushGet();

  //左上ボタンが押されていたら処理
  int BtnNo = 0;
  if (btnpushflg[BtnNo] == 1) {
    if ((MemoryMode == false) || (MemoryEdit == true)) {
      if (ColorNo >= NoColor) ColorNo = 0;
      else ColorNo += 17;
      playMP3(mp3File[33]);
    } else {
      if (MemoryNo >= 10) MemoryNo = 1;
      else MemoryNo++;
      MemoryDisplay();
      ModeSound();
      MemoryChangeFlg = true;
    }
  }
  if (btnpushflg[BtnNo] == 2) {
    if ((MemoryMode == false) || (MemoryEdit == true)) {
      if (ColorNo <= 0) ColorNo = NoColor;
      else ColorNo -= 17;
      playMP3(mp3File[34]);
    } else {
      if (MemoryNo >= 10) MemoryNo = 1;
      else MemoryNo++;
      MemoryDisplay();
      ModeSound();
      MemoryChangeFlg = true;
    }
  }
  if (btnpushflg[BtnNo] == 3) {
    //メモリーモード処理
    if (MemoryMode == false) {
      MemoryMode = true;
      MemoryEdit = false;
      MemoryData[0].ModeNo = ModeNo;
      MemoryData[0].ColorNo = ColorNo;
      MemoryData[0].strnowcnt = strnowcnt;
      MemoryData[0].MoziHalfMode = MoziHalfMode;
      MemoryData[0].CountDownMax = CountDownMax;
      MemoryData[0].PictureNo = PictureNo;

      MemoryDisplay();
    } else {
      if (MemoryEdit == false) {
        //通常モードに戻る
        MemoryMode = false;
        ModeNo = MemoryData[0].ModeNo;
        ColorNo = MemoryData[0].ColorNo;
        strnowcnt = MemoryData[0].strnowcnt;
        MoziHalfMode = MemoryData[0].MoziHalfMode;
        CountDownMax = MemoryData[0].CountDownMax;
        PictureNo = MemoryData[0].PictureNo;
      } else {
        //メモリーに保存
        MemoryEdit = false;
        MemoryData[MemoryNo].ModeNo = ModeNo;
        MemoryData[MemoryNo].ColorNo = ColorNo;
        MemoryData[MemoryNo].strnowcnt = strnowcnt;
        MemoryData[MemoryNo].MoziHalfMode = MoziHalfMode;
        MemoryData[MemoryNo].CountDownMax = CountDownMax;
        MemoryData[MemoryNo].PictureNo = PictureNo;
        playMP3(mp3File[51]);
        vTaskDelay(10);
        while (mp3->isRunning()) vTaskDelay(10);
      }
    }
    ModeSound();
  }
  if (btnpushflg[BtnNo] == 4) {
    MemoryMode = true;
    MemoryEdit = true;
    MemoryDisplay();
    playMP3(mp3File[50]);
    vTaskDelay(10);
    while (mp3->isRunning()) vTaskDelay(10);
    ModeSound();
  }

  btnpushflg[BtnNo] = 0;

  //右上ボタン処理
  BtnNo = 1;
  if (btnpushflg[BtnNo] == 1) {
    if ((MemoryMode == false) || (MemoryEdit == true)) {
      if (ModeNo == MoziAMode || ModeNo == MoziBMode) {
        if (strnowcnt >= 9) strnowcnt = 0;
        else strnowcnt++;
        strtobuff(stringdata[strnowcnt]);
        StrNowSound();
      } else if (ModeNo == CDownMode) {
        if (CountDownMax >= 20) CountDownMax = 5;
        else CountDownMax += 5;
        CountDowmMill = millis();
      } else if (ModeNo == JpegMode) {
        if (PictureNo >= 4) PictureNo = 0;
        else PictureNo++;
        Jpegtobuff(jpegFiles[PictureNo]);
        ImageNowSound();
      } else {
        if (ColorNo <= 0) ColorNo = NoColor;
        else ColorNo -= 17;
        playMP3(mp3File[34]);
      }
    } else {
      if (MemoryNo <= 1) MemoryNo = 10;
      else MemoryNo--;
      MemoryDisplay();
      ModeSound();
      MemoryChangeFlg = true;
    }
  }
  if (btnpushflg[BtnNo] == 2) {
    if ((MemoryMode == false) || (MemoryEdit == true)) {
      if (ModeNo == MoziAMode || ModeNo == MoziBMode) {
        if (strnowcnt == 0) strnowcnt = 9;
        else strnowcnt--;
        strtobuff(stringdata[strnowcnt]);
        StrNowSound();
      } else if (ModeNo == CDownMode) {
        if (CountDownMax <= 5) CountDownMax = 20;
        else CountDownMax -= 5;
        CountDowmMill = millis();
      } else if (ModeNo == JpegMode) {
        if (PictureNo == 0) PictureNo = 4;
        else PictureNo--;
        Jpegtobuff(jpegFiles[PictureNo]);
        ImageNowSound();
      } else {
        if (ColorNo >= NoColor) ColorNo = 0;
        else ColorNo += 17;
        playMP3(mp3File[33]);
      }
    } else {
      if (MemoryNo <= 1) MemoryNo = 10;
      else MemoryNo--;
      MemoryDisplay();
      ModeSound();
      MemoryChangeFlg = true;
    }
  }
  if (btnpushflg[BtnNo] == 3) {
    if ((MemoryMode == false) || (MemoryEdit == true)) {
      MoziHalfMode = !MoziHalfMode;
      MoziHalfSound();
    }
  }
  btnpushflg[BtnNo] = 0;

  //下ボタン処理
  BtnNo = 2;
  if (btnpushflg[BtnNo] == 1) {
    if ((MemoryMode == false) || (MemoryEdit == true)) {
      if (ModeNo >= ModeNoMAX) ModeNo = 0;
      else ModeNo++;
      if ((KakusiModeflg == false) && ((ModeNo == 0) || (ModeNo > MoziBMode))) ModeNo = NorMode;
      if ((ModeNo == MoziAMode) || (ModeNo == MoziBMode)) strtobuff(stringdata[strnowcnt]);
      ModeSound();
    } else playMP3(mp3File[6]);
  }
  if (btnpushflg[BtnNo] == 2) {
    if ((MemoryMode == false) || (MemoryEdit == true)) {
      if (ModeNo <= 0) ModeNo = ModeNoMAX;
      else ModeNo--;
      if ((KakusiModeflg == false) && ((ModeNo == 0) || (ModeNo > MoziBMode))) ModeNo = MoziBMode;
      if ((ModeNo == MoziAMode) || (ModeNo == MoziBMode)) strtobuff(stringdata[strnowcnt]);
      ModeSound();
    } else playMP3(mp3File[7]);
  }
  if (btnpushflg[BtnNo] == 3) {
    BriMill = millis();
    playMP3(mp3File[9]);
    Gainflg = false;
  }
  if (btnpushflg[BtnNo] == 4) DeepSleepMode();
  btnpushflg[BtnNo] = 0;

  if (MemoryChangeFlg == true) udpMemoryCast();
  else if (ModeChangeFlg == true) udpModeCast();
  MemoryChangeFlg = false;
  ModeChangeFlg = false;
}
