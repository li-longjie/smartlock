#include <Adafruit_Fingerprint.h>
//#include <U8g2lib.h>
//#include <U8x8lib.h>
#include <TFT_eSPI.h>
#include <SPI.h>
//#include <Wire.h>

//#include <WiFi.h>    //wifi库
//#include <NTPClient.h>    //NTP库
#include <ArduinoJson.h>  //Json库
#include <HTTPClient.h>  //HTTP库
#ifndef MYFONT_H
#include "MyFont.h"  // 自制字体模板库
#endif
#include "Ticker.h"      

#include "./Pic/ConnectWifi/Connect.h"
#include "./Pic/Astronaut/As.h"
#include "./Pic/weather/Weather.h"
#include "./Pic/picture/Picture.h"
#include "./Pic/butterfly/butterfly.h"
#include "./Pic/door/door.h"
#include <Servo.h>
#include <DHT.h>
#include <NTPClient.h>

#define BLINKER_WIFI
#define BLINKER_MIOT_MULTI_OUTLET
#include <Blinker.h>


#include <Wire.h>
#include <WiFiUdp.h>
#include <Keypad.h>
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {13, 12, 14, 33};
byte colPins[COLS] = {3, 5, 4, 15};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
char password[] = "1234";
char admin_password[] = "1234";
char buffer[5] = "";

int bufferIndex = 0;
uint32_t read_time = 0;
#define RELAY_PIN 32
//#include "font.h"
#define BUZZER_PIN 2 // 蜂鸣器引脚
#define FREQ 2000 // 频率赫兹  、、频率赫兹（FREQ）是指蜂鸣器每秒振动的次数，决定了声音的音调。占空比百分比（DUTY）是指蜂鸣器在一个周期内处于高电平的时间占总时间的比例
#define DUTY 50 // 占空比百分比
//unsigned long currentSec;
#define openkey 21 
TFT_eSPI tft = TFT_eSPI();  //设定屏幕
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2 (U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);  //屏幕接sda、scl接口
/*#define UP 13         //实体按键接口
#define DOWN 12
#define LEFT 11
#define RIGHT 33*/
DHT dht(22, DHT11);//温度传感器接口
BlinkerNumber HUMI("humi");
BlinkerNumber TEMP("temp");
BlinkerNumber HEAT_INDEX("heat_index"); // 新增热指数的 BlinkerNumber 对象
float t,h;

float humi_read = 0, temp_read = 0,heat_index_read=0;
 void heartbeat()
{
    HUMI.print(humi_read);
    TEMP.print(temp_read);
    HEAT_INDEX.print(heat_index_read);
   
}

#define mySerial Serial2   //指纹接urt2，esp芯片接1上电乱码

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
Servo myServo;//舵机对象不用改
char auth[] = "23981871607b";   //点灯秘钥自己设置
char ssid[] = "Xiaomi";   //WiFi名
char pswd[] = "12345678";   //WiFi密码
const char* host = "api.seniverse.com";  //心知天气服务器地址
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp1.aliyun.com", 60 * 60 * 8, 30 * 60 * 1000);
Ticker t1; // 获取天气间隔时间
String months[12]={"January", "February", "March", "April","May", "June", "July", "August", "September", "October", "November", "December"};
String State[17]={"空","添加指纹","删除指纹","验证指纹","请按手指","指纹正常","再按一次","创建模板","模板创建成功",
"模板创建失败","录入指纹成功","指纹验证成功","未搜索到指纹","删除指纹库","删除指纹","成功","失败"};
String now_temperature="", now_date="",now_address="",now_time="",now_high_tem="",now_low_tem="",now_rainfall="",now_wind_direction="",now_wind_scale="",now_hum="",now_weather=""; //用来存储报文得到的字符串
String weekDays[7]={"周日", "周一", "周二","周三", "周四", "周五", "周六"};
String weekEnglish[7] = {"Sun","Mon","Tues","Wed","Thurs","Fri","Sat"};
char determineqing[]="晴";
char determineduoyun[]="多云";
char determineyin[]="阴";
char determineyu[]="雨";
char determinexue[]="雪";
char determinemai[]="霾";
unsigned long currentSec;
int lastMinu;
 int i = 0;
int k = 0;
int ph;
int flag = 1;
char* now_wea;
int tm_Hour,tm_Minute,monthDay,tm_Month;
String weekDay;
char* week;
int q =0;

void get_wifi()
{
    WiFi.begin(ssid,  pswd);              // 连接网络
    while (WiFi.status() != WL_CONNECTED)    //等待wifi连接
    {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");        //连接成功
    Serial.print("IP address: ");            //打印IP地址
    Serial.println(WiFi.localIP());
    tft.fillScreen(TFT_BLACK);
    tft.pushImage(35, 40, 48, 48, ConnectWifi[7]);//调用图片数据
    tft.setCursor(20, 30, 1);                //设置文字开始坐标(20,30)及1号字体
    tft.setTextSize(1);
    tft.println("WiFi Connected!");
    delay(200);
}
void get_weather()
{
  //创建TCP连接
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort))
    {
      Serial.println("Connection failed");  //网络请求无响应打印连接失败
      return;
    }
    //URL请求地址
    String url ="/v3/weather/daily.json?key=ShVlP0pCL7LQ1hest&location=siping&language=zh-Hans&unit=c&start=0&days=5";
    
    //发送网络请求
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
              "Host: " + host + "\r\n" +
              "Connection: close\r\n\r\n");
    delay(2000);
                           
    String answer;               //定义answer变量用来存放请求网络服务器后返回的数据
    while(client.available())
    {
      String line = client.readStringUntil('\r');
      answer += line;
    }
      //断开服务器连接
    client.stop();
    Serial.println();
    Serial.println("closing connection");
    //获得json格式的数据
    String jsonAnswer;
    int jsonIndex;
    //找到有用的返回数据位置i 返回头不要
    for (int i = 0; i < answer.length(); i++) {
      if (answer[i] == '{') {
        jsonIndex = i;
        break;
      }
    }
    jsonAnswer = answer.substring(jsonIndex);
    Serial.println();
    Serial.println("JSON answer: ");
    Serial.println(jsonAnswer);

 StaticJsonDocument<2048> doc;

deserializeJson(doc, jsonAnswer);

JsonObject results_0 = doc["results"][0];

JsonObject results_0_location = results_0["location"];
const char* results_0_location_name = results_0_location["name"]; // "四平"
now_address = results_0_location_name;

for (JsonObject results_0_daily_item : results_0["daily"].as<JsonArray>()) {

  const char* results_0_daily_item_date = results_0_daily_item["date"]; // "2023-03-30", "2023-03-31", ...
  now_date = results_0_daily_item_date;
  const char* results_0_daily_item_text_day = results_0_daily_item["text_day"]; // "雾", "晴", "多云"
 // now_weather = results_0_daily_item_text_day;
 
  const char* results_0_daily_item_high = results_0_daily_item["high"]; // "18", "22", "22"
   now_high_tem = results_0_daily_item_high;
  const char* results_0_daily_item_low = results_0_daily_item["low"]; // "4", "5", "8"
   now_low_tem = results_0_daily_item_low;
  const char* results_0_daily_item_rainfall = results_0_daily_item["rainfall"]; // "0.00", "0.00", "0.00"
   now_rainfall = results_0_daily_item_rainfall;
  const char* results_0_daily_item_precip = results_0_daily_item["precip"]; // "0.00", "0.00", "0.00"
  
  const char* results_0_daily_item_wind_direction = results_0_daily_item["wind_direction"]; // "西南", "西南", ...
  now_wind_direction = results_0_daily_item_wind_direction;
  const char* results_0_daily_item_wind_scale = results_0_daily_item["wind_scale"]; // "4", "2", "5"
  now_wind_scale = results_0_daily_item_wind_scale;
  const char* results_0_daily_item_humidity = results_0_daily_item["humidity"]; // "62", "51", "46"
    now_hum = results_0_daily_item_humidity;
     break;
}


}
void get_weather1()
{
  //创建TCP连接
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort))
    {
      Serial.println("Connection failed");  //网络请求无响应打印连接失败
      return;
    }
    //URL请求地址
    String url ="https://api.seniverse.com/v3/weather/now.json?key=ShVlP0pCL7LQ1hest&location=siping&language=zh-Hans&unit=c";
    
    //发送网络请求
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
              "Host: " + host + "\r\n" +
              "Connection: close\r\n\r\n");
    delay(2000);
                           
    String answer;               //定义answer变量用来存放请求网络服务器后返回的数据
    while(client.available())
    {
      String line = client.readStringUntil('\r');
      answer += line;
    }
      //断开服务器连接
    client.stop();
    Serial.println();
    Serial.println("closing connection");
    //获得json格式的数据
    String jsonAnswer;
    int jsonIndex;
    //找到有用的返回数据位置i 返回头不要
    for (int i = 0; i < answer.length(); i++) {
      if (answer[i] == '{') {
        jsonIndex = i;
        break;
      }
    }
    jsonAnswer = answer.substring(jsonIndex);
    Serial.println();
    Serial.println("JSON answer: ");
    Serial.println(jsonAnswer);

// Stream& input;

StaticJsonDocument<512> doc;
deserializeJson(doc, jsonAnswer);

JsonObject results_0 = doc["results"][0];


JsonObject results_0_now = results_0["now"];
const char* results_0_now_text = results_0_now["text"]; // "晴"
now_weather=results_0_now_text;
const char* results_0_now_code = results_0_now["code"]; // "1"
const char* results_0_now_temperature = results_0_now["temperature"]; // "10"
now_temperature=results_0_now_temperature;

    if(strstr(now_weather.c_str(),determineqing)!=0)
    {  now_wea = "晴";
       ph = 0;
    }
    if(strstr(now_weather.c_str(),determineduoyun)!=0)
    {  now_wea = "多云";
       ph = 1;
    }
    if(strstr(now_weather.c_str(),determineyin)!=0)
    {  now_wea = "阴";
       ph = 2;
    }
    
    if(strstr(now_weather.c_str(),determineyu)!=0)
    {  now_wea = "雨";
       ph = 4;
    }
    if(strstr(now_weather.c_str(),determinexue)!=0)
    {  now_wea = "雪";
       ph = 7;
    }
    if(strstr(now_weather.c_str(),determinemai)!=0)
    {  now_wea = "霾";
       ph = 9;
    }

    
}

void open_door(){
  
   digitalWrite(RELAY_PIN, HIGH);
  //延时5秒
  Serial.println(digitalRead(RELAY_PIN));
  delay(3000);
  //输出低电平，关闭继电器，断开给电磁锁的通路，锁打开
  digitalWrite(RELAY_PIN, LOW);
  }
/*void key_init()
{
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(LEFT, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);
}*/
void buzzer()
{ 
  pinMode(BUZZER_PIN,OUTPUT);

ledcSetup(0, FREQ, 8);
 ledcAttachPin(BUZZER_PIN, 0); // 将PWM通道0附加到BUZZER_PIN
  for (int i = 0; i < 2; i++) { // 循环两次
    ledcWrite(0, (DUTY * 255) / 100); // 写入PWM占空比值，范围为0-255 
    delay(200); // 延时200毫秒
    ledcWrite(0, 0); // 写入PWM占空比值为0，关闭蜂鸣器
    delay(200); // 延时200毫秒
  }
}

void Mg966r()
{
  myServo.attach(19);//舵机接口
  myServo.write(180);
  //myServo.write(0);
  Blinker.delay(3000);
  myServo.write(0);
  Blinker.delay(400);
  myServo.detach();
}
bool oState = false;

void Butterfly()
{
   tft.setSwapBytes(true); 
  tft.pushImage(0, 0,  127, 127,butterfly[k]);
  Serial.println(k);
    delay(300);
    k+=1;
    if(k>1){k=0;}
  
  }
void show_firstpage()
{
   tft.setSwapBytes(true);
    tft.pushImage(0, 0,  127, 127,butterfly[0]);
   tft.pushImage(0, 0, 129, 31, jlnu3);
   showdMyFonts(25,45,"好学近知",TFT_WHITE);
   showdMyFonts(35,70,"力行近仁",TFT_WHITE);
   
   tft.drawFastHLine(10, 104, 108, tft.alphaBlend(0, TFT_BLACK,  TFT_WHITE));
   showtext(35,110,2,1,TFT_WHITE,TFT_BLACK,"15");
   showsMyFont(55,112,"舍",TFT_WHITE);
   showtext(75,110,2,1,TFT_WHITE,TFT_BLACK,"625");
   //showtext(74,115,1,2,TFT_WHITE,TFT_BLACK,currentTime);
  }
void show_inform()
{
   tft.setSwapBytes(true);
   tft.pushImage(32, 15, 57, 50, jlnu4);
   showsMyFonts(0,3,"基于",TFT_WHITE);
   showtext(25,0,2,1,TFT_WHITE,TFT_BLACK,"arduino");
     showsMyFonts(55,3,"的智能门锁",TFT_WHITE);
   showsMyFonts(0,65,"姓名",TFT_WHITE);
   showtext(27,65,2,1,TFT_WHITE,TFT_BLACK,":");
   showsMyFonts(32,65,"李龙杰",TFT_WHITE);
   showsMyFonts(0,80,"学号",TFT_WHITE);
   showtext(27,80,2,1,TFT_WHITE,TFT_BLACK,":");
   showtext(32,80,2,1,TFT_WHITE,TFT_BLACK,"201941040109");
   showsMyFonts(0,95,"专业",TFT_WHITE);
    showtext(27,95,2,1,TFT_WHITE,TFT_BLACK,":");
    showsMyFonts(32,95,"软件工程",TFT_WHITE);
     showtext(82,95,2,1,TFT_WHITE,TFT_BLACK,"ISEC");
   showsMyFonts(0,110,"导师",TFT_WHITE);
   showtext(27,110,2,1,TFT_WHITE,TFT_BLACK,":");
    showsMyFonts(32,110,"邹晓辉",TFT_WHITE);
  } 
void show_indoor()
{
   tft.setSwapBytes(true);
    showMyFonts(20,10,"室内温湿度",TFT_WHITE);
    tft.pushImage(32, 35, 32, 32, tempt);
    tft.pushImage(32, 80, 32, 32, humi1);
     tft.setCursor(68,35, 2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.printf("%0.2f",t);
    tft.setCursor(70,80, 2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
    tft.printf("%0.2f",h);
   
  }
//APP
#define BUTTON_1 "ButtonKey"
BlinkerButton Button1(BUTTON_1);                  //点灯按键
void button1_callback(const String & state)
{
  BLINKER_LOG("get button state: ", state);

  if (state == BLINKER_CMD_BUTTON_TAP) {
    BLINKER_LOG("Button tap!");
   // Button1.color("#0000ff");
   // Button1.text("开门");
   // Button1.print();
    /*myServo.attach(18); //D4
    myServo.write(0);
    Blinker.delay(3000);
    myServo.write(180);
    Blinker.delay(400);
    myServo.detach();*/
   // Mg966r();
   open_door();
    tft.setSwapBytes(true);
    tft.fillScreen(TFT_BLACK);
    for(int l=0;l<2;l++){
    showMyFonts(40,80,"验证成功",TFT_WHITE);
     tft.pushImage(50, 32, 32, 32, Open_door [l]); 
    delay(300);
  }}
  else {
    // Mg966r();
    open_door();
   // BLINKER_LOG("Get user setting: ", state);

  //  Button1.icon("icon_10");
   // Button1.color("#0000ff");
    //Button1.text("Your button name or describe");
    // Button1.text("Your button name", "describe");
   // Button1.print();
  }
}



void miotPowerState(const String & state)              //点灯电源
{
  BLINKER_LOG("need set power state: ", state);

  if (state == BLINKER_CMD_ON) {
  //  myServo.attach(18); //D4
   // myServo.write(180);
    // Mg966r();
   // open_door();
    open_door();
    tft.setSwapBytes(true);
    tft.fillScreen(TFT_BLACK);
    for(int l=0;l<2;l++){
    showMyFonts(40,80,"验证成功",TFT_WHITE);
     tft.pushImage(50, 32, 32, 32, Open_door [l]); 
    delay(300);
    
    BlinkerMIOT.powerState("on");
    BlinkerMIOT.print();

    oState = true;
   // myServo.write(0);
   // Blinker.delay(5000);
  //  myServo.write(180);
  //  Blinker.delay(400);
   // myServo.detach();
    oState = false;
  }}
  else if (state == BLINKER_CMD_OFF) {
    BlinkerMIOT.powerState("off");
    BlinkerMIOT.print();

    oState = false;
  }
}

void miotQuery(int32_t queryCode)                      //状态回调
{
  BLINKER_LOG("MIOT Query codes: ", queryCode);

  switch (queryCode)
  {
    case BLINKER_CMD_QUERY_ALL_NUMBER :
      BLINKER_LOG("MIOT Query All");
      BlinkerMIOT.powerState(oState ? "on" : "off");
      BlinkerMIOT.print();
      break;
    case BLINKER_CMD_QUERY_POWERSTATE_NUMBER :
      BLINKER_LOG("MIOT Query Power State");
      BlinkerMIOT.powerState(oState ? "on" : "off");
      BlinkerMIOT.print();
      break;
    default :
      BlinkerMIOT.powerState(oState ? "on" : "off");
      BlinkerMIOT.print();
      break;
  }
}

void dataRead(const String & data)                //点灯数据保存
{
  BLINKER_LOG("Blinker readString: ", data);

  Blinker.vibrate();

  uint32_t BlinkerTime = millis();

  Blinker.print("millis", BlinkerTime);
}
void Check_password(int match){
  char buffer1[5] = "";
int bufferIndex = 0;
  // 在屏幕上显示输入框
 tft.fillScreen(TFT_BLACK);
 
  showMyFonts(20,20,"请输管理密码",TFT_WHITE);

 tft.pushImage(10, 50,  20, 20, admin);
  tft.drawRect(30, 50, 80, 20, TFT_WHITE);
  tft.setCursor(0, 110,2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, tft.color565(38, 115, 77));
  tft.println('*');
  showMyFonts(10,110,"清空",TFT_WHITE);
   tft.setCursor(0, 90,2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, tft.color565(220,20,60));
  tft.println('B');
  showMyFonts(10,90,"返回",TFT_WHITE);
   tft.setCursor(74, 90,2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, tft.color565(30,144,255));
  tft.println('#');
  showMyFonts(84,90,"确认",TFT_WHITE);
   tft.setCursor(74, 110,2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, tft.color565(134, 0, 179));
  tft.println('C');
  showMyFonts(84,110,"首页",TFT_WHITE);
   tft.setCursor(35, 55,2);
        tft.setTextSize(1);
        tft.setTextColor(TFT_WHITE);
  // 等待用户输入密码
  while (true) {
    // 读取键盘输入
    char key = keypad.getKey();
    if (key != NO_KEY) {
      // 处理键盘输入
      if (key == '#') {
        // 如果输入完成，判断密码是否正确
        if ((strcmp(buffer1, admin_password) == 0)&&(match==0)){
          delay(100);
          Del_FR();
          char buffer1[5] = "";
           return;}
         else if((strcmp(buffer1, admin_password) == 0)&&(match==1)){
             delay(100);
          Add_FR();
          char buffer1[5] = "";
           return;}
            
         else {
          // 如果密码错误，提示重新输入
          tft.fillScreen(TFT_BLACK);
          tft.pushImage(32, 32,  64, 64,  wrong);
          showMyFonts(20,100,"密码错误",tft.color565(255, 80, 80));
          delay(2000);
          Check_password(1);
        }
      } else if (key == '*') {
        // 如果用户取消输入，返回时间显示界面
         Check_password(1);
       //char buffer1[5] = "";
        return;
      }else if (key == 'B') {
        return;}
        else if (key == 'C') {
        return;}
      else {
        // 如果用户输入了字符，添加到缓冲区并在屏幕上显示
        if (bufferIndex < 4) {
  buffer1[bufferIndex++] = key;
   //tft.setCursor(0, 60,1);
   tft.setTextSize(1);
  tft.print(key);
  delay(200);
   tft.fillRect(tft.getCursorX() - 9,  tft.getCursorY()+3, 10, 10, TFT_BLACK); // 清除时间显示区域
  tft.setCursor(tft.getCursorX() - 5, tft.getCursorY());
  tft.print("*");
  
}
  }
  }
  }
  }
void Add_FR()      //添加指纹
{
   
  int i, ensure, processnum = 0;
  int ID_NUM = 0;
  int IDstate =1;
  char str2[10];
  while (1)
  {
    switch (processnum)
    {
      case 0:
        i++;
         tft.fillScreen(TFT_BLACK);
       
        showMyFonts(32, 54,"请按手指",TFT_WHITE);
        ensure = finger.getImage();
        if (ensure == FINGERPRINT_OK)
        {
          ensure = finger.image2Tz(1); //生成特征
          if (ensure == FINGERPRINT_OK)
          { const char* state5=State[5].c_str();
           tft.fillScreen(TFT_BLACK);
             showMyFonts(32, 54,"指纹正常",TFT_WHITE);
             Serial.println(" 000 is true");
            i = 0;
            processnum = 1; //跳到第二步
          }
          else {};
        }
        else {};
        break;

      case 1:
      {
        i++;
         tft.fillScreen(TFT_BLACK);
        showMyFonts(32, 54, "再按一次",TFT_WHITE);
        ensure = finger.getImage();
        if (ensure == FINGERPRINT_OK)
        {
          ensure = finger.image2Tz(2); //生成特征
          if (ensure == FINGERPRINT_OK)
          {
              tft.fillScreen(TFT_BLACK);
            showMyFonts(32, 54, "指纹正常",TFT_WHITE);
            i = 0;
            processnum = 2; //跳到第三步
          }
          else {};
        }
        else {};
        break;
      }
      case 2:
       
         tft.fillScreen(TFT_BLACK);
                showMyFonts(32, 54, "创建模板",TFT_WHITE);
        ensure = finger.createModel();
        if (ensure == FINGERPRINT_OK)
        {
        
                  tft.fillScreen(TFT_BLACK);
                  showMyFonts(12, 54, "模板创建成功",TFT_WHITE);
          processnum = 3; //跳到第四步
        }
        else
        {
          tft.fillScreen(TFT_BLACK);
          showMyFonts(32, 54, "模板创建失败",TFT_WHITE);
          i = 0;
          processnum = 0; //跳回第一步
        }
        delay(500);
        break;
      case 3:
         tft.fillScreen(TFT_BLACK);
 
       showMyFonts(28,20,"请输入",TFT_WHITE);
        showtext(75, 20,2,1,TFT_WHITE,TFT_BLACK,"ID");
       showtext(40, 50,2,1,TFT_WHITE,TFT_BLACK,"ID=00");
      showtext(30,74,2,1,TFT_WHITE,TFT_BLACK,"0=<ID<=99");
       tft.setCursor(74, 110,2);
      tft.setTextSize(1);
     tft.setTextColor(TFT_WHITE, tft.color565(134, 0, 179));
      tft.println('#');
      showMyFonts(84,110,"确认",TFT_WHITE);
 
  while (IDstate)
  {
    char key = keypad.getKey();
    if (key != NO_KEY) {
    if (isdigit(key)) {
    ID_NUM = (ID_NUM * 10) + (key - '0');
    if (ID_NUM > 99) {
        ID_NUM = 99;
      }
    } else if (key == '*') { // Handle clearing ID_NUM
      ID_NUM = 0;
    } else if (key == '#') { // Handle saving ID_NUM
       // ID=ID_NUM; // Do something with the saved ID_NUM value here
       
        IDstate=0;
         //break; 
    }
  
     
  // Update the display with the current ID_NUM value
  if (ID_NUM < 10) {
    sprintf(str2, "ID=0%d", ID_NUM);
  } else {
    sprintf(str2, "ID=%d", ID_NUM);
  }
  tft.fillScreen(TFT_BLACK);
 // showtext(12, 34,1,1,TFT_WHITE,TFT_BLACK,"K1+  K2-  K4 save");
  showMyFonts(28,20,"请输入",TFT_WHITE);
   showtext(75, 20,2,1,TFT_WHITE,TFT_BLACK,"ID");
  showtext(40, 50,2,1,TFT_WHITE,TFT_BLACK,str2);
  showtext(30,74,2,1,TFT_WHITE,TFT_BLACK,"0=<ID<=99");
   tft.setCursor(74, 110,2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, tft.color565(134, 0, 179));
  tft.println('#');
  showMyFonts(84,110,"确认",TFT_WHITE);
  }
  }  
     ensure = finger.storeModel(ID_NUM); //储存模板   它把finger对象（一个指纹识别器）的storeModel方法的返回值赋给ensure这个变量。
                                                          //storeModel方法是用来把指纹模板储存到指定的位置
                                                          //ID_NUM是一个变量，表示储存的位置编号
        if (ensure == 0x00) //0x00是指纹识别器返回的一个状态码，表示储存成功。
        {
          const char* state10=State[10].c_str();
          tft.fillScreen(TFT_BLACK);
          showMyFonts(15, 54, state10,TFT_WHITE);
          Serial.println("FR receive OK");
          delay(1500);
         return;
        }
        else
        {
          
          processnum = 0;
        }
        break;
    }
    delay(400);
    if (i == 10) //超过5次没有按手指则退出
    {
      break;
    }
  }
}
void Del_FR()          //删除指纹
{ 
  //tft.fillScreen(TFT_BLACK);
   int ID_state=1;
  int  ensure;
  int ID_NUM = 0;
  char str2[10];
  
  sprintf(str2, "ID=0%d", ID_NUM);
  tft.fillScreen(TFT_BLACK);
  showtext(25,50,2,2,TFT_WHITE,TFT_BLACK,str2);
   
  showMyFonts(20,20,"请输入删除",TFT_WHITE);
  tft.setCursor(0, 110,2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, tft.color565(38, 115, 77));
  tft.println('C');
  showMyFonts(10,110,"清空指纹库",TFT_WHITE);
   tft.setCursor(74, 110,2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, tft.color565(134, 0, 179));
  tft.println('#');
  showMyFonts(84,110,"确认",TFT_WHITE);
 

               
 while (ID_state)
  {
    char key = keypad.getKey();
    if (key != NO_KEY) {
      
       if (isdigit(key)) {
        ID_NUM = (ID_NUM * 10) + (key - '0');
         if (ID_NUM > 99) {
        ID_NUM = 99;
        }
        }
       else if (key == '*') { // Handle clearing ID_NUM
        ID_NUM = 0;
         }
       else if (key == '#') { // Handle saving ID_NUM
               ID_state=0;
         
             }
       else if (key='B') {
             return ;
               }
   
   
        else if (key='C'){
            finger.emptyDatabase(); //清空指纹库
           
              tft.fillScreen(TFT_BLACK);
             const char* state13=State[13].c_str(); //删除指纹库
             const char* state15=State[15].c_str(); //成功
             
             showMyFonts(8, 16, "删除指纹库成功",TFT_WHITE); 
            //showMyFonts(88, 16, state15,TFT_WHITE);     
            return;
            
          
      delay(1500);
      return ;
    }
     
  // Update the display with the current ID_NUM value
  if (ID_NUM < 10) {
    sprintf(str2, "ID=0%d", ID_NUM);
  } else {
    sprintf(str2, "ID=%d", ID_NUM);
  }
 tft.fillScreen(TFT_BLACK);
  showMyFonts(20,20,"请输入删除",TFT_WHITE);
  showtext(25,50,2,2,TFT_WHITE,TFT_BLACK,str2);
  tft.setCursor(0, 110,2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, tft.color565(38, 115, 77));
  tft.println('C');
  showMyFonts(10,110,"清空指纹库",TFT_WHITE);
   tft.setCursor(74, 110,2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, tft.color565(134, 0, 179));
  tft.println('#');
  showMyFonts(84,110,"确认",TFT_WHITE);
}
  }
  ensure = finger.deleteModel(ID_NUM); //删除单个指纹
  if (ensure == 0)
  {
     tft.fillScreen(TFT_BLACK);
    const char* state14=State[14].c_str();   //删除指纹
    const char* state15=State[15].c_str();  //成功
    showMyFonts(16, 56, "删除指纹成功",TFT_WHITE);
     

  }
  else
  {

   tft.fillScreen(TFT_BLACK);
 const char* state14=State[14].c_str();   //删除指纹
 const char* state16=State[16].c_str();   //失败
showMyFonts(16, 56, state14,TFT_WHITE);
  showMyFonts(80, 56, state16,TFT_WHITE);  

  }
  delay(1500);
  return;
}


void input_password() {
  char buffer[5] = "";
int bufferIndex = 0;
  // 在屏幕上显示输入框
  tft.fillScreen(TFT_BLACK);
 tft.pushImage(10, 50,  20, 20, user);
  showMyFonts(20,20,"请输入密码",TFT_WHITE);
   tft.setCursor(0, 0,2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_GREEN);
  tft.drawRect(30, 53, 80, 20, TFT_GREEN);
  tft.setCursor(0, 110,2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, tft.color565(38, 115, 77));
  tft.println('*');
  showMyFonts(10,110,"清空",TFT_WHITE);
   tft.setCursor(0, 90,2);
  tft.setTextSize(1);
   tft.setTextColor(TFT_WHITE);

  tft.setTextColor(TFT_WHITE,tft.color565(255, 80, 80));
  tft.println('#');
  showMyFonts(10,90,"确认",TFT_WHITE);
   tft.setCursor(54, 90,2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, tft.color565(134, 0, 179));
  tft.println('A');
  showMyFonts(64,90,"添加指纹",TFT_WHITE);
   tft.setCursor(54, 110,2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE ,tft.color565(30,144,255));
  tft.println('D');
  showMyFonts(64,110,"删除指纹",TFT_WHITE);
   tft.setCursor(35, 55,2);
        tft.setTextSize(1);
        tft.setTextColor(TFT_WHITE,TFT_BLACK);
  // 等待用户输入密码
  while (true) {
    // 读取键盘输入
    char key = keypad.getKey();
    if (key != NO_KEY) {
      // 处理键盘输入
      if (key == '#') {
        // 如果输入完成，判断密码是否正确
        if (strcmp(buffer, password) == 0) {
          // 如果密码正确，调用开门函数
          open_door();
           tft.setSwapBytes(true);
        tft.fillScreen(TFT_BLACK);
          for(int l=0;l<2;l++){
            showMyFonts(40,80,"验证成功",TFT_WHITE);
            tft.pushImage(50, 32, 32, 32, Open_door [l]); 
           //  tft.fillScreen(TFT_BLACK);
             delay(300);
             }
             delay(1000);
            tft.fillScreen(TFT_BLACK);
          show_indoor();
           delay(2000);
           char buffer[5] = "";
           //show_firstpage();
        return;
        } 
        else {
          // 如果密码错误，提示重新输入
          tft.fillScreen(TFT_BLACK);
          tft.pushImage(32, 32,  64, 64,  wrong);
          showMyFonts(30,100,"密码错误",tft.color565(255, 80, 80));
          delay(2000);
        //  delay(2000);
          input_password();
        }
      } else if (key == '*') {
        // 如果用户取消输入，返回时间显示界面
         input_password();
      
        return;
        }
       else if(key=='A'){
          Check_password(1);
         
          return;
          }
        else if(key=='D'){
         Check_password(0);
         return;
          }
        else if(key=='B'){
         return;
          }
       else {
       
        // 如果用户输入了字符，添加到缓冲区并在屏幕上显示
        if (bufferIndex < 4) {
        buffer[bufferIndex++] = key;
       //tft.setCursor(0, 60,1);
        tft.setTextSize(1);
        tft.print(key);
        delay(200);
        tft.fillRect(tft.getCursorX() - 9,  tft.getCursorY()+3, 10, 10, TFT_BLACK); // 清除时间显示区域
        tft.setCursor(tft.getCursorX()-5 , tft.getCursorY());
        tft.print("*");
  
}
       }
      }
    }
  }

void Check_FR()
{
  tft.setTextColor(TFT_BLACK); 
int ensure, i;          // 验证指纹并开锁 函数
  char str[20];
  char cishu[5];
  //u8g2.firstPage();


  ensure = finger.getImage();
  if (ensure == 0x00) //获取图像成功
  {
    ensure = finger.image2Tz();
    if (ensure == 0x00) //生成特征成功
    {
      
      ensure = finger.fingerFastSearch();
      if (ensure == 0x00) //搜索成功
      {
         
       // Mg966r();
       open_door();
        q++;
        sprintf(str, "ID:%d Score:%d", finger.fingerID, finger.confidence);//这行代码的意思是将finger.fingerID和finger.confidence两个变量的值按照"ID: % d Score: % d"的格式写入到str字符串中
      
        sprintf(cishu, " % d", q);
          tft.setSwapBytes(true);
        tft.fillScreen(TFT_BLACK);
        for(int l=0;l<2;l++){
         tft.pushImage(42, 32, 32, 32, Open_door [l]); 
        showtext(100,36,1,1, tft.color565(220,20,60),TFT_BLACK,cishu);
       
          showtext(20,70,1,1,TFT_WHITE,TFT_BLACK,str);
          const char* state11=State[11].c_str();
        showMyFonts(16, 80, state11,TFT_WHITE);
          delay(300);
      
        }
          tft.fillScreen(TFT_BLACK);
         show_indoor();
        
        delay(1500);
         tft.fillScreen(TFT_BLACK);
      }
      else
      {
        buzzer();
        tft.fillScreen(TFT_BLACK);
     showMyFonts(16, 50, "未搜索到指纹",TFT_WHITE);
        
        delay(500);
         tft.fillScreen(TFT_BLACK);
      }
    }
  }
  else
  {
    //ShowErrMessage(ensure);
  }
}
void setup()

{ 
  Serial.begin(115200);
  dht.begin();
 // h = dht.readHumidity();
 // t = dht.readTemperature();      
   tft.init();                         //初始化显示寄存器
  pinMode(openkey, INPUT_PULLUP);
  BLINKER_DEBUG.stream(Serial);
  BLINKER_DEBUG.debugAll();
  pinMode(RELAY_PIN, OUTPUT);
//  设置初始状态为低电平，关闭继电器
  digitalWrite(RELAY_PIN,LOW);
 // myServo.attach(18);
 // myServo.write(0);
 // myServo.detach();
  
  //u8g2.begin();
  finger.begin(57600);
  Blinker.begin(auth, ssid, pswd);
    Blinker.attachData(dataRead);
    Blinker.attachHeartbeat(heartbeat);
     
  
  tft.setRotation(4);//屏幕内容镜像显示或者旋转屏幕0-4  ST7735_Rotation中设置
   tft.fillScreen(TFT_BLACK);          //屏幕颜色
   tft.setTextColor(TFT_WHITE);        //设置字体颜色黑色
    tft.setCursor(15, 30, 1);           //设置文字开始坐标(15,30)及1号字体
    tft.setTextSize(1);
   
    tft.setSwapBytes(true);             //使图片颜色由RGB->BGR
    for (int j = 0; j < 7; j++)
    {
        tft.pushImage(0, 0, 127, 127, ConnectWifi[j]); //调用图片数据
        tft.setTextColor(TFT_WHITE);        //设置字体颜色黑色
        tft.setCursor(15, 30, 1);           //设置文字开始坐标(15,30)及1号字体
        tft.setTextSize(1);
         tft.println("Connecting Wifi...");
        delay(300);  
    }
   
  // get_wifi();                           // Wifi连接
  
     get_wifi();                    // Wifi连接
    get_weather();
    get_weather1();
    
    timeClient.begin();
  //  dht.begin();
 // Blinker.attachData(dataRead);
  Button1.attach(button1_callback);//APP
  BlinkerMIOT.attachPowerState(miotPowerState);
  BlinkerMIOT.attachQuery(miotQuery);
  timeClient.setTimeOffset(28800);    //设置偏移时间（以秒为单位）以调整时区
  tft.fillScreen(TFT_BLACK);
     show_inform();
     delay(2000);
}


void loop() {
   timeClient.update();
  
    unsigned long epochTime = timeClient.getEpochTime();
    //Serial.println(epochTime);
    if(flag == 1)
    {
      currentSec = epochTime;
      flag = 0;
    }
    
    String formattedTime = timeClient.getFormattedTime();
    int tm_Hour = timeClient.getHours();
    int tm_Minute = timeClient.getMinutes();
    int tm_Second = timeClient.getSeconds();
    String weekDay = weekEnglish[timeClient.getDay()];
    
    char week[weekDay.length() + 1];
    weekDay.toCharArray(week,weekDay.length() + 1);
    
    struct tm *ptm = gmtime ((time_t *)&epochTime);
    int monthDay = ptm->tm_mday;
    int tm_Month = ptm->tm_mon+1;
    String currentMonthName = months[tm_Month-1];
    int tm_Year = ptm->tm_year+1900;
    String currentDate = String(tm_Month) + "/" + String(monthDay);
  String currentTime, hour, minute;
    if (tm_Hour < 10)
      hour = "0" + String(tm_Hour);
    else
      hour = String(tm_Hour);
    if (tm_Minute < 10)
      minute = "0" + String(tm_Minute);
    else
      minute = String(tm_Minute);
    currentTime = hour + ":" + minute;
  static int state = 0;
  static unsigned long stateStartTime = 0;
  switch (state) {
    /*case 0: // show first page for 5 seconds
      if (epochTime - stateStartTime < 5) {
        show_firstpage();
      } else {
        state = 1;
        stateStartTime = epochTime;
        tft.fillScreen(TFT_BLACK);
        
      }
      break;
*/
    case 0: // show menu for 5 seconds
      if (epochTime - stateStartTime < 5) {
       show_time(TFT_WHITE, TFT_BLACK, Astronaut,hour,tm_Minute,tm_Month,monthDay, tm_Year, week);
      } else {
       
         tft.fillScreen(TFT_BLACK);
        show_firstpage();
        delay(3000);
         state = 1;
        stateStartTime = epochTime;
        tft.fillScreen(TFT_BLACK);
      }
      break;

case 1: // show menu for 5 seconds
      if (epochTime - stateStartTime < 5) {
      show_weather(TFT_WHITE, TFT_BLACK);
      } else {
        state = 2;
        stateStartTime = epochTime;
        tft.fillScreen(TFT_BLACK);
      }
      break;

    case 2: // show butterfly animation for 5 seconds
      if (epochTime - stateStartTime < 5) {
        Butterfly();
      } else {
        state = 0;
        stateStartTime = epochTime;
        tft.fillScreen(TFT_BLACK);
      }
  }
   while (true) {
    char key = keypad.getKey();
    if (key != NO_KEY) {
      input_password();  // 进入密码输入界面
     
    } break;
  }
  Check_FR();
  Blinker.run();
    if (read_time == 0 || (millis() - read_time) >= 1000)
    {
        read_time = millis();

        h = dht.readHumidity();
        t = dht.readTemperature();        
      
        if (isnan(h) || isnan(t)) {
            BLINKER_LOG("Failed to read from DHT sensor!");
            return;
        }

        float hic = dht.computeHeatIndex(t, h, false);

        humi_read = h;
        temp_read = t;
        heat_index_read = hic; 
        BLINKER_LOG("Humidity: ", h, " %");
        BLINKER_LOG("Temperature: ", t, " *C");
        BLINKER_LOG("Heat index: ", hic, " *C");
    }
  Blinker.delay(50);
  if (digitalRead(openkey) == 0)  //按下K1键 调用Add_FR()
  {
    open_door();
  }
  
}

/*******************时间界面显示****************/
void show_time(uint16_t fg,uint16_t bg,const uint16_t* image[], String Hour,int Minute, int MON,int DAY, int tm_Year,const char* week)
{
    //tft.fillRect(10, 55,  64, 64, bg);
    tft.setSwapBytes(true);             //使图片颜色由RGB->BGR
    tft.pushImage(73, 54,  64, 64, image[i]);
    delay(50);
    i+=1;
    if(i>8){i=0;}
    tft.drawFastHLine(10, 53, 108, tft.alphaBlend(0, bg,  fg));
    showtext(15,5,2,3,fg,bg,Hour);
    tft.setCursor(63,5, 2);
    tft.setTextSize(3);
   // tft.setTextColor(tft.color565(255, 80, 80));
    tft.printf(":");
    tft.setCursor(75,5, 2);
    tft.setTextSize(3);
    tft.setTextColor(tft.color565(255, 80, 80));
    if(Minute<10){
     tft.printf("0%d",Minute);
     }
     else  tft.printf("%d",Minute);
    //showtext(75,60,1,2,fg,bg, String(tm_Year));
    //月
    tft.setCursor(3,55, 2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE);
    tft.printf("%d",MON);
    showsMyFont(11,57,"月",TFT_YELLOW);
    //日
    tft.setCursor(22,55, 2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE);
    tft.printf("%d",DAY);
    showsMyFont(37,57,"日",TFT_YELLOW);
    //showtext(10,80,1,2,fg,bg, currentDate);
   // showMyFonts(15, 100, week, TFT_YELLOW);
   //周
   tft.setCursor(50,55, 2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE,tft.color565(255, 80, 80));
    tft.println(week);
   // showtext(10,80,2,1,fg,bg, week);
   tft.pushImage(8, 71, 52, 44, xa);
 
    tft.drawFastHLine(10, 115, 108, tft.alphaBlend(0, bg,  fg));
   showsMyFonts(22,116,"小爱开门已部署",TFT_WHITE);
}
/*******************天气界面显示****************/
void show_weather(uint16_t fg,uint16_t bg)
{
  
    
    tft.setSwapBytes(true);             //使图片颜色由RGB->BGR
    tft.pushImage(20, 0,  64, 64, weather[ph]);

    //showMyFonts(90, 20, now_address.c_str(), TFT_WHITE);
    if(ph==1){
    tft.fillRect(89, 19, 36, 18, tft.color565(255, 80, 80));
    }
    else  tft.fillRect(89, 19, 18, 18, tft.color565(255, 80, 80));
    showMyFonts(90, 20, now_wea, TFT_WHITE);
    showtext(90,40,2,1,fg,bg,now_temperature);
    showsMyFont(107,43,"℃",TFT_YELLOW);

    
    tft.pushImage(0, 65, 25, 25, temIcon);
    tft.pushImage(0, 95, 25, 25, humIcon);
    tft.pushImage(57, 65, 25, 25, rainIcon);
     tft.pushImage(57, 95, 25, 25, windIcon);
    showtext(28,105,2,1,fg,bg,now_hum+"%");
    showtext(28,75,2,1,fg,bg,now_high_tem + "/" + now_low_tem);
    showtext(87,102,2,1,fg,bg,now_wind_scale);
    showsMyFont(97,104,"级",TFT_WHITE);
    showtext(85,75,2,1,fg,bg,now_rainfall);
  //  showtext(100,75,1,1,fg,bg,"mm");
   // showtext(100,40,1,1,fg,bg,now_hum+"%");
   //String now_wind = now_wind_direction + "风";
   // showMyFonts(85, 100, now_wind.c_str(), TFT_WHITE);
}

/*******************整句字符串显示****************/
void showtext(int16_t x,int16_t y,uint8_t font,uint8_t s,uint16_t fg,uint16_t bg,const String str)
{
  //设置文本显示坐标，和文本的字体，默认以左上角为参考点，
    tft.setCursor(x, y, font);
  // 设置文本颜色为白色，文本背景黑色
    tft.setTextColor(fg,bg);
  //设置文本大小，文本大小的范围是1-7的整数
    tft.setTextSize(s);
  // 设置显示的文字，注意这里有个换行符 \n 产生的效果
    tft.println(str);
}

/*******************单个汉字显示****************/
void showMyFont(int32_t x, int32_t y, const char c[3], uint32_t color) { 
  for (int k = 0; k < 79; k++)// 根据字库的字数调节循环的次数
    if (hanzi[k].Index[0] == c[0] && hanzi[k].Index[1] == c[1] && hanzi[k].Index[2] == c[2])
    { tft.drawBitmap(x, y, hanzi[k].hz_Id, hanzi[k].hz_width, 16, color);
    }
}

/*******************整句汉字显示****************/
void showMyFonts(int32_t x, int32_t y, const  char str[], uint32_t color) { //显示整句汉字，字库比较简单，上下、左右输出是在函数内实现
  int x0 = x;
  for (int i = 0; i < strlen(str); i += 3) {
    showMyFont(x0, y, str+i, color);
    x0 += 17;
  }
}
/*******************单个大汉字显示****************/
void showdMyFont(int32_t x, int32_t y, const char c[3], uint32_t color) { 
  for (int k = 79; k < 86; k++)// 根据字库的字数调节循环的次数
    if (hanzi[k].Index[0] == c[0] && hanzi[k].Index[1] == c[1] && hanzi[k].Index[2] == c[2])
    { tft.drawBitmap(x, y, hanzi[k].hz_Id, hanzi[k].hz_width, 20, color);
    }
}
/*******************整句大汉字显示****************/
void showdMyFonts(int32_t x, int32_t y, const  char str[], uint32_t color) { //显示整句汉字，字库比较简单，上下、左右输出是在函数内实现
  int x0 = x;
  for (int i = 0; i < strlen(str); i += 3) {
    showdMyFont(x0, y, str+i, color);
    x0 += 20;
  }
}
/*******************单个小汉字显示****************/
void showsMyFont(int32_t x, int32_t y, const char c[3], uint32_t color) { 
  for (int k = 86; k < 120; k++)// 根据字库的字数调节循环的次数
    if (hanzi[k].Index[0] == c[0] && hanzi[k].Index[1] == c[1] && hanzi[k].Index[2] == c[2])
    { tft.drawBitmap(x, y, hanzi[k].hz_Id, hanzi[k].hz_width, 12, color);
    }
}
/*******************整句小汉字显示****************/
void showsMyFonts(int32_t x, int32_t y, const  char str[], uint32_t color) { //显示整句汉字，字库比较简单，上下、左右输出是在函数内实现
  int x0 = x;
  for (int i = 0; i < strlen(str); i += 3) {
    showsMyFont(x0, y, str+i, color);
    x0 += 12;
  }
}
