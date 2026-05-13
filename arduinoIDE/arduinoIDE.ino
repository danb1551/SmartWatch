#include <M5StickCPlus2.h>
#include <WiFi.h>
#include "pictures.h"
#include "config.h"
#include "cube.h"

bool uspavat = true;
bool showHidden = false;
bool clearShowHidden = true;
bool debug = true;
bool clear = true;
bool showCopyright = true;
bool sleeping = false;
// 1 = hodiny; 2 = wifi; 3 = bluetooth; 4 = IR
int renderIndex = 1;
int renderedIndex;
int choosen = 0;
int prevMinute = 99;
String dnyTydnu[] = {"NE", "PO", "UT", "ST", "CT", "PA", "SO"};
int brightness = 1;

int lastActionTime = 0;
// buttons; 0 = nothing (default); 1 = was pressed in last actionTime; 2 = is currently pressed
// main front button
int lastBtnAState = 0;
// side navigation button
int lastBtnBState = 0;
// power button
int lastBtnPwrState = 0;

void pass() {}

void setup() {
    auto cfg = M5.config();
    // abych nekomunikoval s PC
    cfg.serial_baudrate = 0;
    cfg.output_power = false;
    cfg.clear_display = true;
    cfg.disable_rtc_irq = true;
    cfg.led_brightness = 0;
    M5.begin(cfg);
    M5.Power.begin();

    M5.Display.setRotation(1);
    M5.Display.setBrightness(1);
    //attachInterrupt(digitalPinToInterrupt(M5.BtnA), handlePress);
    // vypnout wifi a bluetooth
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    btStop();
    setCpuFrequencyMhz(debug ? 20 : 240);
    M5.Display.powerSave(true);
    M5.Imu.begin();
}

void loop() {
    M5.update();

    /* ještě vymyslet, jestli funkci nebo porovnání
    bude si to muset pamatovat poslední stav a podle něj porovnat
    if (M5.BtnA.getState() == M5.BtnA.state_clicked) {M5.BtnA.isPressed();}
    enum button_state_t : std::uint8_t
    { state_nochange
    , state_clicked
    , state_hold
    , state_decide_click_count
    }; */

    if (M5.BtnA.isPressed() && millis() - lastActionTime > 150) {
        select();
        lastActionTime = millis();
    } else if (M5.BtnB.isPressed() && millis() - lastActionTime > 150) {
        prev();
        lastActionTime = millis();
    } else if (M5.BtnPWR.isPressed() && millis() - lastActionTime > 150) {
        sleeping ? wakeup() : next();
        lastActionTime = millis();
    }
    if ((millis() - lastActionTime > 6000) && uspavat) {
        sleep();
    }

    render();
    delay(20);
}

void sleep() {
    sleeping = true;
    M5.Imu.sleep();
    M5.Display.setBrightness(0);
    M5.Display.sleep();
    M5.Power.lightSleep(pow(10, 14));
}

void wakeup() {
    sleeping = false;
    M5.Display.wakeup();
    render();
    M5.Display.setBrightness(brightness);
    delay(500);
    //... better change it in loop so that it will not call next() function
    //sleeping = false;
}

void select() {
    if (showHidden) {
        if (M5.Display.getBrightness() == brightness) {
            M5.Display.setBrightness(255);
        } else {
            M5.Display.setBrightness(brightness);
        }
    }
    if (renderedIndex == 1 || renderedIndex == 0) {
        if (showHidden == true) {
            renderHidden();
            return;
        }
        sleep();
    } else if (renderedIndex == 2) {
    } else if (renderedIndex == 3) {
    } else if (renderedIndex == 4) {
    } else if (renderedIndex == 5) {
        sleeping = true;
        M5.Imu.sleep();
        M5.Display.setBrightness(0);
        M5.Display.sleep();
        M5.Power.deepSleep(pow(10, 14));
    }
}

void next() {
    if (showHidden) {
        renderIndex = 0;
        showHidden = false;
        clearShowHidden = true;
    }
    clear = true;
    renderIndex++;
    if (renderIndex == 6) {
        renderIndex = 5;
        clear = false;
    }
    /*if (choosen > 1) {
        if (choosen == 2) {
            renderIndex = 20;
        } else if (choosen == 3) {
            renderIndex = 30;
        }
    }*/
}

void prev() {
    clear = true;
    renderIndex--;
    if (renderIndex == 0) {
        showHidden = true;
        renderIndex = 1;
        clear = false;
    }
}

void renderHidden() {
    if (clearShowHidden) {
        clearShowHidden = false;
        M5.Display.fillScreen(0x0);
        M5.Display.pushImage(0, 0, 240, 135, image_fotka_hidden);
    }
}

void renderMain() {
    if (showHidden) {
        return;
    }
    auto time = M5.Rtc.getTime();
    auto date = M5.Rtc.getDate();
    M5.Display.startWrite();
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.setFont(&fonts::FreeSans12pt7b);
    if (clear) {
        M5.Display.fillScreen(TFT_BLACK);
        // line
        M5.Display.drawLine(64, 80, 174, 80, TFT_GREEN);
        // rect
        M5.Display.drawRect(63, 79, 113, 3, 0x0400);
        // datum
        M5.Display.setTextSize(1);
        M5.Display.setCursor(45, 85);
        M5.Display.print(dnyTydnu[date.weekDay] + " " + (String) date.date +
                         "." + (String) date.month + "." + (String) date.year);
        drawArrow(2);
    }
    if (prevMinute != time.minutes || clear) {
        M5.Display.fillRect(33, 21, 174, 55, TFT_BLACK);
        M5.Display.setTextSize(3);
        M5.Display.setCursor(33, 21);
        String hours;
        String minutes;
        if ((time.hours + 2) % 24 < 10) {
            hours = "0" + (String) ((time.hours + 2) % 24);
        } else hours = (String) ((time.hours + 2) % 24);
        if (time.minutes < 10) {
            minutes = "0" + (String) time.minutes;
        } else minutes = (String) time.minutes;
        M5.Display.print(hours + ":" + minutes);
        prevMinute = time.minutes;
        drawBattery();
    }
    if (clear && showCopyright) {

        // podpis
        M5.Display.setTextSize(0.5);
        // při velikosti 0.5 je pozice [100;122]
        // při velikosti 1 je pozice [53;110]
        M5.Display.setCursor(95, 122);
        M5.Display.print("by danb1551");
    }
    M5.Display.endWrite();
    if (clear) {
        delay(400);
    }
}

void renderWifi() {
    if (clear) {
        M5.Display.fillScreen(TFT_BLACK);
        drawArrow(1);
        drawArrow(2);
        M5.Display.drawBitmap(51, 6, image_wifi_logo, 139, 99, TFT_GREEN);
        M5.Display.setTextSize(1);
        M5.Display.setTextColor(TFT_GREEN);
        M5.Display.setFont(&fonts::FreeSans12pt7b);
        M5.Display.setCursor(90, 112);
        M5.Display.print("Wi-Fi");
        delay(400);
    }
}

void renderWifiMenu(int index) {
    if (index == 1) {
        return;
    }
}

void renderBluetooth() {
    if (clear) {
        M5.Display.fillScreen(TFT_BLACK);
        drawArrow(1);
        drawArrow(2);
        M5.Display.fillRect(102, 30, 28, 52, TFT_WHITE);
        M5.Display.drawBitmap(92, 24, image_bluetooth_logo, 48, 64, TFT_BLUE);
        M5.Display.setTextSize(1);
        M5.Display.setTextColor(TFT_GREEN);
        M5.Display.setFont(&fonts::FreeSans12pt7b);
        M5.Display.setCursor(70, 112);
        M5.Display.print("Bluetooth");
        delay(400);
    }
}

void renderBluetoothMenu(int index) {
    if (index == 1) {
        return;
    }
}

void renderIR() {
    if (clear) {
        M5.Display.fillScreen(TFT_BLACK);
        drawArrow(1);
        drawArrow(2);
        M5.Display.drawBitmap(94, 25, image_ir_signal, 49, 23, TFT_RED);
        M5.Display.fillRoundRect(102, 51, 36, 45, 12, 0xBDF7);
        M5.Display.fillEllipse(113, 68, 5, 5, 0x0);
        M5.Display.fillEllipse(126, 68, 5, 5, 0x0);
        M5.Display.fillEllipse(113, 80, 5, 5, 0x0);
        M5.Display.fillEllipse(126, 80, 5, 5, 0x0);
        M5.Display.setTextSize(1);
        M5.Display.setTextColor(TFT_GREEN);
        M5.Display.setFont(&fonts::FreeSans12pt7b);
        M5.Display.setCursor(80, 112);
        M5.Display.print("InfraRed");
        delay(400);
    }
}

void renderIRMenu() {
}

void renderImu() {
    drawCubeIMU();
    delay(100);
}

    /// bitmaps (1 = left; 2 = right)
void drawArrow(int combination) {
    M5.Display.startWrite();
    if (combination == 1) {
        M5.Display.drawBitmap(4, 116, image_arrow_left, 24, 15, TFT_GREEN);
    } else if (combination == 2) {
        M5.Display.drawBitmap(212, 116, image_arrow_right, 24, 15, TFT_GREEN);
    }
    M5.Display.endWrite();
}

void clearBatteryPlace() {
    M5.Display.fillRect(224, 10, 10, 14, TFT_BLACK);
}

void drawBattery() {
    int batteryLevel = M5.Power.getBatteryLevel();
    M5.Display.startWrite();
    // somehow this doesn't work and it stop my M5Stick from responding
    // later figured that it does not work on M5stickC plus2
    // but if you have another product feel free to uncomment
    /*if (M5.Power.isCharging()) {
        clearBatteryPlace();
        M5.Display.drawBitmap(224, 10, image_battery_charging, 10, 14, TFT_GREEN);
    } else*/
    if (batteryLevel > 75) {
        clearBatteryPlace();
        M5.Display.drawBitmap(224, 8, image_baterry_case, 10, 14, TFT_GREEN);
        M5.Display.fillRect(226, 12, 6, 2, TFT_GREEN);
        M5.Display.fillRect(226, 15, 6, 2, TFT_GREEN);
        M5.Display.fillRect(226, 18, 6, 2, TFT_GREEN);
    } else if (batteryLevel > 50) {
        clearBatteryPlace();
        M5.Display.drawBitmap(224, 10, image_baterry_case, 10, 14, 0x67EC);
        M5.Display.fillRect(226, 15, 6, 2, 0x67EC);
        M5.Display.fillRect(226, 18, 6, 2, 0x67EC);
    } else if (batteryLevel > 25) {
        clearBatteryPlace();
        M5.Display.drawBitmap(224, 10, image_baterry_case, 10, 14, TFT_YELLOW);
        M5.Display.fillRect(226, 18, 6, 2, TFT_YELLOW);
    } else {
        clearBatteryPlace();
        M5.Display.drawBitmap(224, 10, image_baterry_case, 10, 14, TFT_RED);
    }
    M5.Display.endWrite();
}

void render() {
    /*if (choosen > 1) {
        if (choosen == 2) {
            renderWifiMenu(1);
        } else if (choosen == 3) {
            renderBluetoothMenu(1);
        }
    }*/
    if (showHidden) {
        M5.Display.powerSave(false);
        renderHidden();
        uspavat = false;
    } else {
        M5.Display.powerSave(true);
        uspavat = true;
    }

    if (renderIndex == 0 || renderIndex == 1) {
        renderMain();
        renderedIndex = 1;
    } else if (renderIndex == 2) {
        renderWifi();
        renderedIndex = 2;
    } else if (renderIndex == 3) {
        renderBluetooth();
        renderedIndex = 3;
    } else if (renderIndex == 4) {
        renderIR();
        renderedIndex = 4;
    } else if (renderIndex == 5) {
        renderImu();
        uspavat = false;
        renderedIndex = 5;
    }
    clear = false;
}






