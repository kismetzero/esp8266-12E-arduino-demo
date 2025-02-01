#include <Arduino.h>

#include <RTClib.h>
// #define RTC_SCL_PIN     5   // D1
// #define RTC_SDA_PIN     4   // D2
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
// #define DISP_CLK_PIN 14  // D5
// #define DISP_DATA_PIN 13 // D7
// #define DISP_CS_PIN 12   // D6
// U8G2_MAX7219_32X8_F_4W_SW_SPI um8g2 = U8G2_MAX7219_32X8_F_4W_SW_SPI(U8G2_R0, DISP_CLK_PIN, DISP_DATA_PIN, DISP_CS_PIN, U8X8_PIN_NONE, U8X8_PIN_NONE);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C um8g2 = U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE);
void disp_init();
void disp_loop();

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
DHT_Unified dht(D4, DHT11);
void dht_init();
int t3s = 0;

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
char timeStr[18];
char dateStr[18];
char dhtStr[24];

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
    // 初始化 dht
    dht_init();
    Serial.println("dht_init : OK!");
    delay(1);
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
    disp_loop();
}

bool wifi_init(const char *apName, const char *apPassword)
{
    bool res = wm.autoConnect(apName, apPassword);
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
    um8g2.enableUTF8Print();
    um8g2.setFont(u8g2_font_siji_t_6x10);
}

void disp_loop()
{
    if (f_tckr1s)
    {
        f_tckr1s = false;
        DateTime now = rtc.now();
        sprintf(timeStr, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
        sprintf(dateStr, "%04d-%02d-%02d", now.year(), now.month(), now.day());
        uint16_t tmp_wifi_status;
        if (WiFi.status() == WL_CONNECTED)
            tmp_wifi_status = 0x0e21a;
        else
            tmp_wifi_status = 0x0e217;
        sensors_event_t event;
        float t = 99.99, h = 99.99;
        t3s++;
        if(t3s>3)
        {
        dht.temperature().getEvent(&event);
        if (isnan(event.temperature))
        {
            Serial.println(F("Error reading temperature!"));
            t = 99.99;
        }
        else
        {
            Serial.print(F("Temperature: "));
            t = event.temperature;
            Serial.print(event.temperature);
            Serial.println(F("°C"));
        }
        // 获取湿度事件并打印其值.
        dht.humidity().getEvent(&event);
        if (isnan(event.relative_humidity))
        {
            Serial.println(F("Error reading humidity!"));
            h = 99.99;
        }
        else
        {
            Serial.print(F("Humidity: "));
            h = event.relative_humidity;
            Serial.print(event.relative_humidity);
            Serial.println(F("%"));
        }
        }
        sprintf(dhtStr, "%02.1f-%02.1f", t, h);
        um8g2.clearBuffer();
        um8g2.drawFrame(0, 0, 128, 64); // 外框
        um8g2.drawGlyph(2, 12, tmp_wifi_status);
        um8g2.setCursor(16, 12);
        um8g2.print(WiFi.SSID());
        um8g2.setCursor(3, 23);
        um8g2.print(WiFi.localIP());
        um8g2.setCursor(3, 34);
        um8g2.print(timeStr);
        Serial.println(timeStr);
        um8g2.setCursor(62, 34);
        um8g2.print(dateStr);
        Serial.println(dateStr);
        um8g2.drawGlyph(3, 45, 0x0e0cf);
        um8g2.setCursor(16, 45);
        um8g2.print(dhtStr);
        Serial.println(dhtStr);
        um8g2.sendBuffer();
    }
}

void dht_init()
{
    dht.begin();
    Serial.println(F("DHTxx Unified Sensor Example"));
    // 打印温度传感器详细信息.
    sensor_t sensor;
    dht.temperature().getSensor(&sensor);
    Serial.println(F("------------------------------------"));
    Serial.println(F("Temperature Sensor"));
    Serial.print(F("Sensor Type: "));
    Serial.println(sensor.name);
    Serial.print(F("Driver Ver:  "));
    Serial.println(sensor.version);
    Serial.print(F("Unique ID:   "));
    Serial.println(sensor.sensor_id);
    Serial.print(F("Max Value:   "));
    Serial.print(sensor.max_value);
    Serial.println(F("°C"));
    Serial.print(F("Min Value:   "));
    Serial.print(sensor.min_value);
    Serial.println(F("°C"));
    Serial.print(F("Resolution:  "));
    Serial.print(sensor.resolution);
    Serial.println(F("°C"));
    Serial.println(F("------------------------------------"));
    Serial.print(F("Temperature min_delay:  "));
    Serial.print(sensor.min_delay);
    Serial.println(F("------------------------------------"));
    // 打印湿度传感器详细信息
    dht.humidity().getSensor(&sensor);
    Serial.println(F("Humidity Sensor"));
    Serial.print(F("Sensor Type: "));
    Serial.println(sensor.name);
    Serial.print(F("Driver Ver:  "));
    Serial.println(sensor.version);
    Serial.print(F("Unique ID:   "));
    Serial.println(sensor.sensor_id);
    Serial.print(F("Max Value:   "));
    Serial.print(sensor.max_value);
    Serial.println(F("%"));
    Serial.print(F("Min Value:   "));
    Serial.print(sensor.min_value);
    Serial.println(F("%"));
    Serial.print(F("Resolution:  "));
    Serial.print(sensor.resolution);
    Serial.println(F("%"));
    Serial.println(F("------------------------------------"));
    Serial.print(F("Humidity min_delay:  "));
    Serial.print(sensor.min_delay);
    Serial.println(F("------------------------------------"));
}