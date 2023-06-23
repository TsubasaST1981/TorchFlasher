String Headder_str() {
  String html = "";
  html += "<!DOCTYPE html><html lang=”ja”><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.3'>";
  html += "<meta http-equiv='Pragma' content='no-cache'>";
  html += "<meta http-equiv='Cache-Control' content='no-cache'></head>";
  html += "<meta http-equiv='Expires' content='0'>";
  html += "<meta charset='utf-8'>";
  html += "<style>";
  html += "a:link, a:visited { background-color: #009900; color: white; padding: 5px 15px;";
  html += "text-align: center; text-decoration: none;  display: inline-block;}";
  html += "a:hover, a:active { background-color: green;}";
  html += "bo32{ background-color: #EEEEEE;}";
  html += "input[type=button], input[type=submit], input[type=reset] {";
  html += "background-color: #000099;  border: none;  color: white;  padding: 5px 20px;";
  html += "text-decoration: none;  margin: 4px 2px;";
  html += "</style>";
  html += "<body>";
  return html;
}
String WIFI_Form_str() {
  Serial.println("wifi scan start");

  // WiFi.scanNetworks will return the number of networks found
  uint8_t ssid_num = WiFi.scanNetworks();
  Serial.println("scan done\r\n");

  if (ssid_num == 0) {
    Serial.println("no networks found");
  } else {
    Serial.printf("%d networks found\r\n\r\n", ssid_num);
    if (ssid_num > SSIDLIMIT) ssid_num = SSIDLIMIT;
    for (int i = 0; i < ssid_num; ++i) {
      ssid_str[i] = WiFi.SSID(i);
      String wifi_auth_open = ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      ssid_rssi_str[i] = ssid_str[i] + " (" + WiFi.RSSI(i) + "dBm)" + wifi_auth_open;
      ssid_rssi_str[i] = ssid_str[i] + wifi_auth_open;
      Serial.printf("%d: %s\r\n", i, ssid_rssi_str[i].c_str());
      delay(10);
    }
  }

  String str = "";
  str += "<form action='/wifiset' method='get'>";
  str += "<select name='ssid' id ='ssid'>";
  for (int i = 0; i < ssid_num; i++) {
    str += "<option value=" + ssid_str[i] + ">" + ssid_rssi_str[i] + "</option>";
  }
  str += "<option value=" + String(ssid) + ">" + String(ssid) + "(current)</option>";
  if (ssid != DeviceName) {
    str += "<option value=" + (String)DeviceName + ">" + (String)DeviceName + "(default)</option>";
  }
  str += "</select><br>\r\n";
  str += "Password<br><input type='password' name='password' value='" + String(password) + "'>";
  str += "<br><input type='submit' value='set'>";
  str += "</form><br>";
  str += "<script>document.getElementById('ssid').value = '" + String(ssid) + "';</script>";
  return str;
}
//各種Wifi設定
String maskpasswd(String passwd) {
  String maskpasswd = "";

  for (int i = 0; i < passwd.length(); i++) maskpasswd = maskpasswd + "*";
  if (passwd.length() == 0) maskpasswd = "(null)";

  return maskpasswd;
}
void wifimgr_top() {
  String html = Headder_str();
  html += "<a href='/wifiinput'>WIFI setup</a>";
  html += "<hr><h3>Current Settings</h3>";
  html += "SSID: " + String(ssid) + "<br>";
  html += "password: " + maskpasswd(String(password)) + "<br>";
  html += "<hr><p><center><a href='/reboot'>Reboot</a></center>";
  html += "<br><a href='/textInput'>TextSetting</a>";
  html += "<br><a href='/upOSfile'>OSUpdate</a>";
  html += "<br><br><a href='/allReset'>ALLdataReset</a>";
  html += "</body></html>";
  Server.send(200, "text/html", html);
}
void wifiinput() {
  String html = Headder_str();
  html += "<a href='/'>TOP</a> ";
  html += "<hr><p>";
  html += "<h3>WiFi Selector</h3>";
  html += WIFI_Form_str();
  html += "<br><hr><p><center><a href='/'>Cancel</a></center>";
  html += "</body></html>";
  Server.send(200, "text/html", html);
}

void wifiset() {

  Server.arg("ssid").toCharArray(ssid, 30);
  Server.arg("password").toCharArray(password, 30);
  //ssid.trim();
  //password.trim();

  EEPMill = 1;
  EEPWrite();

  // 「/」に転送
  Server.sendHeader("Location", String("/"), true);
  Server.send(302, "text/plain", "");
}

void reboot() {
  String html = Headder_str();
  html += "<hr><p>";
  html += "<h3>reboot confirmation</h3><p>";
  html += "Are you sure to reboot?<p>";
  html += "<center><a href='/doreboot'>YES</a> <a href='/'>no</a></center>";
  html += "<p><hr>";
  html += "</body></html>";
  Server.send(200, "text/html", html);
}

void doreboot() {
  String html = Headder_str();
  html += "<hr><p>";
  html += "<h3>rebooting</h3><p>";
  html += "The setting WiFi connection will be disconnected...<p>";
  html += "<hr>";
  html += "</body></html>";
  Server.send(200, "text/html", html);

  // reboot esp32
  Serial.println("reboot esp32 now.");

  delay(2000); // hold 2 sec
  ESP.restart(); // restart ESP32
}

//OTAセッティング
void EspOTAsetting() {
  //Wifiプログラム更新設定
  ArduinoOTA.setHostname(DeviceName);
  ArduinoOTA
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  })
  .onEnd([]() {
    Serial.println("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();
}



void allDataResetcheck() {
  String Html = Headder_str();
  Html += "<h1>ALLdataReset</h1>";
  Html += "<form action='/allDataReset' method='POST'>";
  Html += "<input type='submit' value='ResetOK'>";
  Html += "</form><br><br>";
  Html += "<a href='/'>Back to Root</a>";
  Server.send(200, "text/html", Html);
}

void allDataReset() {
  EEPMill = 1;
  EEPROM.put(EEP_ADRS0, 1);
  EEPWrite();
  ESP.restart(); // restart ESP32
}

void StringSet() {
  Server.arg("textdata").toCharArray(stringdata[Server.arg("TextNo").toInt()], 20);
  EEPMill = 1;
  EEPWrite();

  // テキスト設定に戻る
  Server.sendHeader("Location", String("/textInput"), true);
  Server.send(302, "text/plain", "");
}

String StrDataForm() {
  String Html = Headder_str();
  Html += "<h1>TextSet</h1>";
  for (int i = 0; i < 10; i++) {
    Html += "<form action='/StringSet' method='POST'>";
    Html += "Text" + String(i + 1) + "<br><input type='textdata' name='textdata' maxlength = '8' value='" + String(stringdata[i]) + "'>";
    Html += "<input type='submit' value='set'>";
    Html += "<input type='hidden' id='TextNo' name='TextNo' value='" + String(i) + "'>";
    Html += "</form><br>";
  }
  return Html;
}

void RTtextinput() {
  String Html = Headder_str();
  Html += StrDataForm();
  Html += "<a href='/upOSfile'>OSupload</a><br><br>";
  Html += "<br><br><a href='/allReset'>ALLdataReset</a>";

  Server.send(200, "text/html", Html);
}

void APtextinput() {
  String Html = Headder_str();
  Html += StrDataForm();
  Html += "<a href='/'>Back to Root</a>";

  Server.send(200, "text/html", Html);
}

void OSfileUp() {
  String Html = Headder_str();
  Html += "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>";
  Html += "<h1>OSUpdate</h1>";
  Html += "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>";
  Html += "<input type='file' name='update'>";
  Html += "<input type='submit' value='Update'>";
  Html += "</form>";
  Html += "<div id='prg'>progress: 0%</div>";
  Html += "<br><a href='/'>Go to Root</a>";
  Html += "<script>";
  Html += "$('form').submit(function(e){";
  Html += "e.preventDefault();";
  Html += "var form = $('#upload_form')[0];";
  Html += "var data = new FormData(form);";
  Html += " $.ajax({";
  Html += "url: '/update',";
  Html += "type: 'POST',";
  Html += "data: data,";
  Html += "contentType: false,";
  Html += "processData:false,";
  Html += "xhr: function() {";
  Html += "var xhr = new window.XMLHttpRequest();";
  Html += "xhr.upload.addEventListener('progress', function(evt) {";
  Html += "if (evt.lengthComputable) {";
  Html += "var per = evt.loaded / evt.total;";
  Html += "$('#prg').html('progress: ' + Math.round(per*100) + '%');";
  Html += "}";
  Html += "}, false);";
  Html += "return xhr;";
  Html += "},";
  Html += "success:function(d, s) {";
  Html += "console.log('success!')";
  Html += "},";
  Html += "error: function (a, b, c) {";
  Html += "}";
  Html += "});";
  Html += "});";
  Html += "</script>";

  Server.sendHeader("Connection", "close");
  Server.send(200, "text/html", Html);
}

//UDPブロードキャスト
void udpModeCast() {
  if (WifilocalMode == 0) return;

  uint8_t sendPacket[9];
  sendPacket[0] = ModeNo;
  sendPacket[1] = (uint8_t)(ColorNo / 256);
  sendPacket[2] = (uint8_t)(ColorNo % 256);
  sendPacket[3] = strnowcnt;
  sendPacket[4] = (uint8_t)MoziHalfMode;
  sendPacket[5] = CountDownMax;
  sendPacket[6] = PictureNo;
  sendPacket[7] = Brightness;
  sendPacket[8] = SoundGain;

  // パケット送信
  udp.beginPacket(castIP, REMOTE_PORT);
  udp.write(sendPacket[0]);
  udp.write(sendPacket[1]);
  udp.write(sendPacket[2]);
  udp.write(sendPacket[3]);
  udp.write(sendPacket[4]);
  udp.write(sendPacket[5]);
  udp.write(sendPacket[6]);
  udp.write(sendPacket[7]);
  udp.write(sendPacket[8]);
  udp.endPacket();
  LastSendUdpMill = millis();
}

//UDPメモリーブロードキャスト
void udpMemoryCast() {
  if (WifilocalMode == 0) return;

  uint8_t sendPacket[1];
  sendPacket[0] = MemoryNo;

  // パケット送信
  udp.beginPacket(castIP, REMOTE_PORT);
  udp.write(sendPacket[0]);
  udp.endPacket();
  LastSendUdpMill = millis();
}

//UDP受信返答
void udpGettoPut() {
  if (WifilocalMode == 0) return;

  uint8_t packetBuffer[9];

  int packetSize = udp.parsePacket();
  if (packetSize > 0) {
    int len = udp.read(packetBuffer, packetSize);

    if (packetSize == 9) {
      ModeNo = packetBuffer[0];
      ColorNo = (packetBuffer[1] * 256) + packetBuffer[2];
      strnowcnt = packetBuffer[3];
      MoziHalfMode = (bool)packetBuffer[4];
      CountDownMax = packetBuffer[5];
      PictureNo = packetBuffer[6];
      Brightness = packetBuffer[7];
      SoundGain = packetBuffer[8];
    } else if (packetSize == 1) {
      MemoryNo = packetBuffer[0];
      MemoryDisplay();
    }
  }
}

//Wifi同期初期化
void Wifilocalinit(bool Setupflg = false) {
  //切断処理
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  vTaskDelay(100);


  if (WifilocalMode == 0) {
    if (Setupflg == false) playMP3(mp3File[72]);
  } else {
    playMP3(mp3File[71]);
    if (WifilocalMode == 1) {
      //アクセスポイントモード　※ホスト
      WiFi.mode(WIFI_AP);
      WiFi.softAP("TorchFlasher", "TorchFlasher");
      vTaskDelay(200);
      WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
      vTaskDelay(500);
      WiFi.setTxPower(WIFI_POWER_15dBm);
      udp.begin(LOCAL_PORT);
      while (mp3->isRunning()) vTaskDelay(10);
      playMP3(mp3File[73]);
    } else {
      //ステーションモード　※クライアント
      WiFi.mode(WIFI_STA);
      WiFi.begin("TorchFlasher", "TorchFlasher");
      vTaskDelay(500);
      WiFi.setTxPower(WIFI_POWER_15dBm);
      while (mp3->isRunning()) vTaskDelay(10);
      WifiClientSetup = false;
    }
  }
}
