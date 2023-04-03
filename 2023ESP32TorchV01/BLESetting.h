//BLE設定
NimBLEServer * pServer = NULL;
NimBLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(NimBLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        String str = "";
        for (int i = 0; i < rxValue.length(); i++) {
          str += rxValue[i];
        }

        str.toCharArray(stringdata[strnowcnt], 20);
        bleflg = true;

      }
      Sleepmill = millis();
    }
};

//ブルートゥース初期設定
void BLEinit() {
  // Create the BLE Device
  NimBLEDevice::init(DeviceName);

  // Create the BLE Server
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  NimBLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  //  pTxCharacteristic = pService->createCharacteristic(
  //                        CHARACTERISTIC_UUID_TX,
  //                        BLECharacteristic::PROPERTY_NOTIFY
  //                      );
  //
  //  pTxCharacteristic->addDescriptor(new BLE2902());

  NimBLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_RX,
      NIMBLE_PROPERTY::WRITE
                                          );

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
}
