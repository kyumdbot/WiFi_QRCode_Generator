# 自製 WiFi QR Code 產生器裝置

## § 材料準備

![材料](imgs/IMG3.jpg)

1. [LOLIN D32 Pro](https://wiki.wemos.cc/products:d32:d32_pro)
2. [LOLIN TFT 2.4 Touch Shield](https://wiki.wemos.cc/products:d1_mini_shields:tft_2.4_shield)
3. TFT Cable
4. 按鈕

1、2、3 項可以在 [WEMOS.CC](https://www.wemos.cc/) 找到購買網址，第 4 項按鈕在一般的電子材料行即可買到。


## § 組裝

先將按鈕的其中一腳接到 LOLIN D32 Pro 的 13 號腳位，按鈕對角線一腳接到 GND，如下圖所示：

![接腳圖1](imgs/IMG2.jpg)

或是如下圖這樣也可以：

![接腳圖2](imgs/img4.jpg)
![接腳圖3](imgs/img5.jpg)

然後將 TFT 螢幕接上：

![接上螢幕](imgs/IMG.jpg)


## § 設定 Arduino IDE 環境

1. 請先閱讀 [arduino-esp32](https://github.com/espressif/arduino-esp32) 的 README.md，照裡頭的步驟安裝好 ESP32 Board 套件。
2. 然後到 [WEMOS wiki](https://wiki.wemos.cc/downloads) 裡下載 Serial port Driver，並安裝之。
3. 把 D32 Pro 連接到電腦上，然後下載本 Repo 的 ESP32_QRCodeGenerator.ino，並用 Arduino IDE 開啟這個 ino 檔案。
4. 點選 Arduino IDE 選單 -> Sketch -> Include Library -> Manage Libraries...，然後安裝以下程式庫：

    - Adafruit GFX Library (by Adafruit)
    - Adafruit ILI9341 (by Adafruit)
    - XPT2046_Touchscreen (by Paul Stoffregen)
    - QRCode (by Richard Moore)

5. 如果之前有安裝過 BLEPeripheral (by Sandeep Mistry) 這個程式庫，請先刪除這個程式庫，因為裡頭一些 class name 跟 ESP32 BLE 程式庫裡的 class name 有衝突。
6. 點選 Arduino IDE 選單 -> Tools -> Board: -> 選擇 `LOLIN D32 PRO`； Upload Speed: -> 選擇 `115200`； Flash Frequency: -> 選擇 `40MHz`。
7. 點選 Arduino IDE 選單 -> Tools -> Port，選擇連接 D32 Pro 的 Serial port，Windows 上的名稱通常都是 COM`X`，Mac 上的名稱通常都是 /dev/cu.wchusbserial`XXXX` (X 為數字)。如下圖所示：

**Windows：**
![port-win](imgs/port_win.jpg)

**macOS：**
![port-mac](imgs/port_mac.png)


## § 修改 BLE DEVICE NAME

找到 ESP32_QRCodeGenerator.ino 程式碼中的這一行：

```cpp
#define MY_BLE_DEVICE_NAME  "RL_WIFIQR_Generator-1788"
```

把這個裝置名稱修改成自己想要的名字。我的習慣通常是用 **裝置前綴名** + **一組數字** 來組成名稱，以這裡為例，`RL_WIFIQR_Generator` 就是裝置前綴名，後面接一組數字 `-1788`，然後如果需要多台裝置，就改變後面那組數字即可。


## § 編譯並上傳程式

按下 Arduino IDE 的 Upload 按鈕，編譯程式並上傳程式到 D32 Pro：

![upload-win](imgs/upload_win.png)



## § 開啟裝置的 BLE 名稱廣播

按下裝置上的按鈕，大約長按 3 ~ 4 秒，TFT 螢幕下方會出現一行小字的 `Start BLE Advertising...`，表示此裝置已經開始廣播了，這時如果使用一些藍牙裝置掃描軟體，就可以發現會出現上面設定的那個裝置名稱 (例如 `RL_WIFIQR_Generator-1788`)。

**長按按鈕 3 ~ 4 秒，直到出現 Start BLE Advertising...：**
![press-btn](imgs/press_btn.jpg)



## § 設定 WiFi QR Code

用 PC/Mac 或是 Android 上的 Chrome 網頁瀏覽器開啟此網址： [https://kyumdbot.github.io/app/bleWiFiQR/](https://kyumdbot.github.io/app/bleWiFiQR/)

按下網頁上的 `Scan Bluetooth Devices` 按紐，開始掃描 BLE 裝置：
![scan-ble](imgs/scan_ble.jpg)

找到裝置名稱，點選裝置，並按下配對。
接著選擇 Authentication Type，填入 SSID 跟 Password 後，按下 `SETUP` 按鈕，等出現 Setup done! 即完成設定。

![setup-qr](imgs/setup_qr.jpg)



## § 停止裝置的 BLE 廣播

QR Code 設定完成後，請務必要停止裝置的 BLE 廣播，這樣才比較不會讓別人偷偷連連到你的裝置。

要停止裝置的 BLE 廣播，首先按下 Setup 網頁上的 `Disconnect` 按鈕，中斷 Setup 網頁跟裝置的 BLE 連線，接下來長按按鈕 3 ~ 4 秒，裝置會 Restart (或直接按下 D32 Pro 上的 reset 按鈕，或直接把電源拔掉重插，都有一樣的效果)。

(備註：本來在開啟廣播後，再長按按鈕會切換成 Advertising Stop 狀態，但因為日前發現 BLEAdvertising->stop() 不起作用，所以我把程式的 pAdvertising->stop() 改成 ESP.restart()。)



## §


## §

