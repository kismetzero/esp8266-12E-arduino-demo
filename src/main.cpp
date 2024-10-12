#include <Arduino.h>

#define DISP_CLK_PIN 14  // D5
#define DISP_DATA_PIN 13 // D7
#define DISP_CS_PIN 12   // D6

// #define RTC_SCL_PIN     5   // D1
// #define RTC_SDA_PIN     4   // D2

#include <RTClib.h>
RTC_DS3231 rtc;

#include <WiFiManager.h>
WiFiManager wm;
bool wifi_init(const char *apName, const char *apPassword);

#include <NTPClient.h>
// change next line to use with another board/shield
#include <ESP8266WiFi.h>
// #include <WiFi.h> // for WiFi shield
// #include <WiFi101.h> // for WiFi 101 shield or MKR1000
#include <WiFiUdp.h>
WiFiUDP ntpUDP;               // UDP 客户端
NTPClient timeClient(ntpUDP); // NTP 客户端
void ntp_init(const char *poolServerName, long timeOffset, unsigned long updateInterval);

void update_time_ntp_rtc();

#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
U8G2_MAX7219_32X8_F_4W_SW_SPI um8g2 = U8G2_MAX7219_32X8_F_4W_SW_SPI(U8G2_R0, DISP_CLK_PIN, DISP_DATA_PIN, DISP_CS_PIN, U8X8_PIN_NONE, U8X8_PIN_NONE);
void disp_init();

#include <Ticker.h>
Ticker tckr;
bool f_tckr1s = false;
bool f_tckr50ms = false;
void timer50ms()
{
    static unsigned int cnt50ms = 0;
    f_tckr50ms = true;
    cnt50ms++;
    if (cnt50ms == 20)
    {
        f_tckr1s = true; // 1 sec
        cnt50ms = 0;
    }
}
char timeStr[10];

void setup()
{
    delay(1);
    // 初始化 串口
    Serial.begin(9600);
    Serial.println("Serial init : 9600");
    delay(1);
    tckr.attach(0.05, timer50ms); // every 50 msec
    // 初始化 disp
    disp_init();
    Serial.println("disp_init : OK!");
    delay(1);
    um8g2.clearBuffer();
    um8g2.drawStr(0, 7, "WIFI-CONNT!");
    um8g2.sendBuffer();
    delay(500);
    // 初始化 rtc
    rtc.begin();
    Serial.println("rtc_init : OK!");
    delay(1);
    // 初始化 wifi
    wifi_init("ESP", "147258369..");
    Serial.println("wifi_init : OK!");
    delay(1);
    // 初始化 ntp
    ntp_init("ntp3.aliyun.com", 60 * 60 * 8, 1000 * 60 * 60);
    Serial.println("ntp_init : OK!");
    delay(1);
    // 更新 rtc 时间
    update_time_ntp_rtc();
    Serial.println("update_rtc : OK!");
    delay(1);
}

void loop()
{
    timeClient.update();
    DateTime now = rtc.now();
    if (f_tckr1s)
    {
        f_tckr1s = false;
        sprintf(timeStr, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
        um8g2.clearBuffer();
        um8g2.drawStr(3, 7, timeStr);
        um8g2.sendBuffer();
    }
}

bool wifi_init(const char *apName, const char *apPassword)
{
    bool res;
    res = wm.autoConnect(apName, apPassword);
    if (!res)
    {
        Serial.println("Failed to connect");
        // ESP.restart();
        um8g2.clearBuffer();
        um8g2.drawStr(0, 7, "WIFI-NO!");
        um8g2.sendBuffer();
    }
    else
    {
        Serial.println("connected...yeey :)");
        um8g2.clearBuffer();
        um8g2.drawStr(0, 7, "WIFI-OK!");
        um8g2.sendBuffer();
    }
    return res;
}

void ntp_init(const char *poolServerName, long timeOffset, unsigned long updateInterval)
{
    timeClient.begin();
    timeClient.setPoolServerName(poolServerName);
    timeClient.setTimeOffset(timeOffset);
    timeClient.setUpdateInterval(updateInterval);
    timeClient.update();
}

void update_time_ntp_rtc()
{
    Serial.print("ntp_FormattedTime : ");
    Serial.println(timeClient.getFormattedTime());
    Serial.print("ntp_EpochTime : ");
    Serial.println(timeClient.getEpochTime());
    DateTime now = rtc.now();
    Serial.print("rtc_secondstime : ");
    Serial.println(now.secondstime());
    Serial.print("rtc_timestamp : ");
    Serial.println(now.timestamp());
    uint64_t t1 = timeClient.getEpochTime() - 946684800;
    uint64_t t2 = now.secondstime();
    if (t1 != t2)
    {
        timeClient.forceUpdate();
        uint64_t t = timeClient.getEpochTime();
        DateTime d = DateTime(t);
        rtc.adjust(d);
        Serial.println("update_time_ntp_rtc!!");
    }
}

void disp_init()
{
    um8g2.begin();
    um8g2.setFont(u8g2_font_blipfest_07_tr);
    um8g2.setContrast(2);
}
