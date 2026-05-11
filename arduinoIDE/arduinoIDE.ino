#include <M5StickCPlus2.h>
#include <WiFi.h>
#include "pictures.h"
#include "config.h"

bool debug = true;
bool clear = true;
bool showCopyright = true;
bool sleeping = false;
// 1 = hodiny; 2 = wifi; 3 = bluetooth; 4 = IR
int renderIndex = 0;
int renderedIndex;
int prevMinute = 99;
String dnyTydnu[] = {"NE", "PO", "ÚT", "ST", "ČT", "PÁ", "SO"};
int brightness = 1;

int lastActionTime = 0;
// buttons; 0 = nothing (default); 1 = was pressed in last actionTime; 2 = is currently pressed
// main front button
int lastBtnAState = 0;
// side navigation button
int lastBtnBState = 0;
// power button
int lastBtnPwrState = 0;


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
}

void loop() {
    M5.update();
    // ještě vymyslet, jestli funkci nebo porovnání
    //bude si to muset pamatovat poslední stav a podle něj porovnat
    //if (M5.BtnA.getState() == M5.BtnA.state_clicked) {M5.BtnA.isPressed();}
    //enum button_state_t : std::uint8_t
    //{ state_nochange
    //, state_clicked
    //, state_hold
    //, state_decide_click_count
    //};

    if (M5.BtnA.isPressed() && millis() - lastActionTime > 150) {
        wakeup();
        select();
        lastActionTime = millis();
    } else if (M5.BtnB.isPressed() && millis() - lastActionTime > 150) {
        wakeup();
        prev();
        lastActionTime = millis();
    } else if (M5.BtnPWR.isPressed() && millis() - lastActionTime > 150) {
        wakeup();
        next();
        lastActionTime = millis();
    }
    if (millis() - lastActionTime > 6000) {
        sleep();
        sleep();
    }

    M5.Display.powerSave(true);

    render();
    delay(10);
}

void sleep() {
    sleeping = true;
    M5.Display.setBrightness(0);
    M5.Display.sleep();
    M5.Power.lightSleep(pow(10, 6000));
}

void wakeup() {
    M5.Display.wakeup();
    render();
    M5.Display.setBrightness(brightness);
    sleeping = false;
}

void select() {
    if (renderedIndex == 1 || renderedIndex == 0) {
        sleep();
    }
    //M5.Display.fillRect(0, 0, 100, 100, 0x6767);
    return;
    if (renderedIndex == 2) {
        return;
    }
}

void next() {
    renderIndex++;
    if (renderIndex == 4) {}
}

void prev() {
    drawBattery();
}

void renderMain() {
    auto time = M5.Rtc.getTime();
    auto date = M5.Rtc.getDate();
    M5.Display.startWrite();
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.setFont(&fonts::FreeSans12pt7b);
    if (clear) {
        M5.Display.fillScreen(TFT_BLACK);
        // line
        M5.Display.drawLine(64, 67, 174, 67, TFT_GREEN);
        // rect
        M5.Display.drawRect(63, 66, 113, 3, 0x0400);
        // datum
        M5.Display.setTextSize(1);
        M5.Display.setCursor(45, 72);
        M5.Display.print(dnyTydnu[date.weekDay] + " " + (String) date.date +
                         "." + (String) date.month + "." + (String) date.year);
        drawArrow(2);
    }
    if (prevMinute != time.minutes || clear) {
        M5.Display.fillRect(33, 8, 174, 55, TFT_BLACK);
        M5.Display.setTextSize(3);
        M5.Display.setCursor(33, 8);
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
        M5.Display.setCursor(100, 122);
        M5.Display.print("by danb1551");
    }
    M5.Display.endWrite();
}


void renderTools() {}

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
        M5.Display.drawBitmap(224, 10, image_baterry_full, 10, 14, TFT_GREEN);
    } else if (batteryLevel > 50) {
        clearBatteryPlace();
        M5.Display.drawBitmap(224, 10, image_baterry_middle, 10, 14, 0x67EC);
    } else if (batteryLevel > 25) {
        clearBatteryPlace();
        M5.Display.drawBitmap(224, 10, image_baterry_low, 10, 14, TFT_YELLOW);
    } else {
        clearBatteryPlace();
        M5.Display.drawBitmap(224, 10, image_baterry_empty, 10, 14, TFT_RED);
    }
    M5.Display.endWrite();
}

void render() {
    if (renderIndex == 0 || renderIndex == 1) {
        renderMain();
        renderIndex = 1;
        renderedIndex = 1;
    } else if (renderIndex == 2) {
        renderTools();
    } else if (renderIndex == 1) {
        renderMain();
    }
    clear = false;
}












