/****************************************************
 * 
 * LOLIN D32 Pro (based ESP32) WiFi QRCode Generator
 * 
 ****************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include "Adafruit_ILI9341.h" // Hardware-specific library
#include <SPI.h>
#include <Preferences.h>
#include <qrcode.h>
// BLE libs
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>



Preferences preferences;


// TFT Pin
#define TFT_CS  14    // for D32 Pro
#define TFT_DC  27    // for D32 Pro
#define TFT_RST 33    // for D32 Pro
#define TS_CS   12    // for D32 Pro

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// Color
#define MY_BLACK_COLOR 0x0000    // Black
#define MY_WHITE_COLOR 0xFFFF    // White


// Values
std::string wifi_type;
std::string wifi_ssid;
std::string wifi_pass;
std::string my_action;


// BLE Device Name
#define MY_BLE_DEVICE_NAME  "RL_WIFIQR_Generator-1788"


// BLE UUID
#define  SERVICE_UUID                   "0000dd00-0000-1000-8000-00805f9b34fb"
#define  WIFITYPE_CHARACTERISTIC_UUID   "0000dd01-0000-1000-8000-00805f9b34fb"
#define  WIFISSID_CHARACTERISTIC_UUID   "0000dd02-0000-1000-8000-00805f9b34fb"
#define  WIFIPASS_CHARACTERISTIC_UUID   "0000dd03-0000-1000-8000-00805f9b34fb"
#define  ACTION_CHARACTERISTIC_UUID     "0000dd05-0000-1000-8000-00805f9b34fb"


// BLE Characteristic
BLECharacteristic *typeCharacteristic;
BLECharacteristic *ssidCharacteristic;
BLECharacteristic *passCharacteristic;
BLECharacteristic *actionCharacteristic;


// BLE Service / Advertising
BLEService *pService;
BLEAdvertising *pAdvertising;
bool isAdvertising = false;
bool isCheckAdvertisingState = false;

// Button Pin
int buttonPin = 13;
bool isPressButton = false;


// Timing
long previousMillis = 0;
long interval = 3000;


//
// Draw WiFi QR Code
//
void draw_WiFi_QRCode() {
    // QR code content text
    std::string qrStr = "WIFI:T:" + wifi_type + ";S:" + wifi_ssid + ";P:" + wifi_pass + ";;";

    // Create the QR code (Version 4, ECC_MEDIUM)
    QRCode qrcode;
    uint8_t qrcodeData[qrcode_getBufferSize(4)];
    qrcode_initText(&qrcode, qrcodeData, 4, ECC_MEDIUM, qrStr.c_str());


    // Draw QR code
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextSize(3);
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(23, 24);
    tft.println("WiFi QRCode");
    
    tft.fillRect(30, 96, 180, 180, MY_WHITE_COLOR);

    int offset_x = 54;
    int offset_y = 120;
    
    for (int y = 0; y < qrcode.size; y++) {
        for (int x = 0; x < qrcode.size; x++) {
            int newX = offset_x + (x * 4);
            int newY = offset_y + (y * 4);
            
            if (qrcode_getModule(&qrcode, x, y)) {
                tft.fillRect( newX, newY, 4, 4, MY_BLACK_COLOR);
            }
            else {
                tft.fillRect( newX, newY, 4, 4, MY_WHITE_COLOR);
            }
        }
    }

    if (isAdvertising) {
        tft.setTextSize(1);
        tft.setTextColor(ILI9341_YELLOW);
        tft.setCursor(50, 305);
        tft.println("Start BLE Advertising...");
    }
}

//
// BLE Server Callback class
//
class MyServerCallbacks: public BLEServerCallbacks
{
    void onConnect(BLEServer* pServer) {
        Serial.println("Connected.");
    };

    void onDisconnect(BLEServer* pServer) {
        Serial.println("Disconnected.");
        isCheckAdvertisingState = true;
    }
};

//
// BLE Characteristic Read/Write Callback class
//
class MyCallbacks: public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic) {
        Serial.println("onWrite");
        std::string value = pCharacteristic->getValue();

        BLEUUID myuuid = pCharacteristic->getUUID();
        std::string myuuid_str = myuuid.toString();
        //Serial.println(myuuid_str.c_str());
        
        
        if ( myuuid_str.compare(WIFITYPE_CHARACTERISTIC_UUID) == 0 ) {       //WIFI_TYPE
            wifi_type = value;
            Serial.println("WIFI TYPE:");
        }
        else if ( myuuid_str.compare(WIFISSID_CHARACTERISTIC_UUID) == 0 ) {  //WIFI_SSID
            wifi_ssid = value;
            Serial.println("WIFI SSID:");
        }
        else if ( myuuid_str.compare(WIFIPASS_CHARACTERISTIC_UUID) == 0 ) {  //WIFI_PASS
            wifi_pass = value;
            Serial.println("WIFI PASSWORD:");
        }
        else if ( myuuid_str.compare(ACTION_CHARACTERISTIC_UUID) == 0 ) {    //ACTION
            my_action = value;
            Serial.println("ACTION:");
        }

        if (value.length() > 0) {
            Serial.println("*********");
            Serial.print("New value: ");
            for (int i = 0; i < value.length(); i++) {
                Serial.print(value[i]);
            }
            Serial.println();
            Serial.println("*********");
        }
    }

    void onRead(BLECharacteristic* pCharacteristic) {
        Serial.println("onRead");
    }
};


//
//-------------------- main --------------------
//
void setup(void) {
    Serial.begin(115200);
    
    pinMode(buttonPin, INPUT_PULLUP);
    init_preferences();
    my_action = "0";


    tft.begin();
    yield();

    tft.setRotation(0);
    draw_WiFi_QRCode();

    // 
    // Setup BLE Server
    //
    Serial.println("Starting BLE Server...");
    
    BLEDevice::init(MY_BLE_DEVICE_NAME);
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    pService = pServer->createService(SERVICE_UUID);


    // WIFI_TYPE 
    typeCharacteristic = pService->createCharacteristic(
                            WIFITYPE_CHARACTERISTIC_UUID,
                            BLECharacteristic::PROPERTY_READ   |
                            BLECharacteristic::PROPERTY_WRITE  |
                            BLECharacteristic::PROPERTY_NOTIFY
                         );
    typeCharacteristic->setValue(wifi_type.c_str());
    typeCharacteristic->addDescriptor(new BLE2902());
    typeCharacteristic->setCallbacks(new MyCallbacks());


    // WIFI_SSID
    ssidCharacteristic = pService->createCharacteristic(
                            WIFISSID_CHARACTERISTIC_UUID,
                            BLECharacteristic::PROPERTY_READ   |
                            BLECharacteristic::PROPERTY_WRITE  |
                            BLECharacteristic::PROPERTY_NOTIFY
                         );
    ssidCharacteristic->setValue(wifi_ssid.c_str());
    ssidCharacteristic->addDescriptor(new BLE2902());
    ssidCharacteristic->setCallbacks(new MyCallbacks());


    // WIFI_PASS
    passCharacteristic = pService->createCharacteristic(
                            WIFIPASS_CHARACTERISTIC_UUID,
                            BLECharacteristic::PROPERTY_READ   |
                            BLECharacteristic::PROPERTY_WRITE  |
                            BLECharacteristic::PROPERTY_NOTIFY
                         );
    passCharacteristic->setValue(wifi_pass.c_str());
    passCharacteristic->addDescriptor(new BLE2902());
    passCharacteristic->setCallbacks(new MyCallbacks());


    // ACTION
    actionCharacteristic = pService->createCharacteristic(
                            ACTION_CHARACTERISTIC_UUID,
                            BLECharacteristic::PROPERTY_READ   |
                            BLECharacteristic::PROPERTY_WRITE  |
                            BLECharacteristic::PROPERTY_NOTIFY
                         );
    actionCharacteristic->setValue(my_action.c_str());
    actionCharacteristic->addDescriptor(new BLE2902());
    actionCharacteristic->setCallbacks(new MyCallbacks());


    pService->start();
    pAdvertising = pServer->getAdvertising();
    
    
    Serial.println("BLE Server is ready.");
    delay(1000);
}


void loop() {
    // Check my_action value
    if ( my_action.compare("0") != 0 ) {
        tft.fillScreen(ILI9341_WHITE);
        
        draw_WiFi_QRCode();
        save_wifi_info();
        
        my_action = "0";
        actionCharacteristic->setValue(my_action.c_str());
        actionCharacteristic->notify();
    }

    if ( digitalRead(buttonPin) == LOW ) {
        isPressButton = true;
    }

    if ( millis() - previousMillis > interval ) {
        if ( isPressButton && (digitalRead(buttonPin) == LOW) ) {
            isAdvertising = !isAdvertising;
            
            if ( isAdvertising ) {
                pAdvertising->start();
            } else {
                //Serial.println("Stop BLE Advertising.");
                pAdvertising->stop();
            }

            draw_WiFi_QRCode();
        }

        isPressButton = false;
        previousMillis = millis();
    }

    if ( isCheckAdvertisingState ) {
        isCheckAdvertisingState = false;
        
        if ( !isAdvertising ) {
            delay(100);
            pAdvertising->stop();
            Serial.println("Check and Stop advertising.");
        }
    }
}


void init_preferences() {
    preferences.begin("wifiqr", false);
    String read_type = preferences.getString("type");
    String read_ssid = preferences.getString("ssid");
    String read_pass = preferences.getString("pass");
  
    if ( read_type.equals("") || read_ssid.equals("") || read_pass.equals("") ) {
        Serial.println("Not WiFi info");
        wifi_type = "WPA";
        wifi_ssid = "demo";
        wifi_pass = "demopass";
    } else {
        wifi_type = std::string(read_type.c_str());
        wifi_ssid = std::string(read_ssid.c_str());
        wifi_pass = std::string(read_pass.c_str());
        Serial.println("----- WiFi info -----");
        Serial.println(wifi_type.c_str());
        Serial.println(wifi_ssid.c_str());
        Serial.println(wifi_pass.c_str());
        Serial.println("---------------------");
    }
    preferences.end();
}

void save_wifi_info() {
    preferences.begin("wifiqr",false);
    preferences.putString("type", wifi_type.c_str());
    preferences.putString("ssid", wifi_ssid.c_str());
    preferences.putString("pass", wifi_pass.c_str());
    preferences.end();
}
