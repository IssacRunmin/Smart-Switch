#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include<Servo.h>
#include <ESP8266WiFi.h>
#include  <Ticker.h>          //Ticker Library
#include <SoftwareSerial.h>
// 传感器引脚
#define PIN_INTENSITY A0      // 光照传感器引脚
#define PIN_INFRARED D6       // 红外传感器
#define PIN_BUZZER D5         // 蜂鸣器接口
#define PIN_SERVO D7          // 舵机的接口
#define PIN_KEY D8            // 按键的接口
// 传感器采样间隔
#define INTERVAL 10           // Unit: ms, 定时器间隔，定义为10ms
#define DUR_LONG_PRESS 1000   // Unit: ms, 按键长按的间隔，1秒
#define DUR_SWITCH 1000       // Unit: ms, 开关操作间隔，连续操作导致刷新1秒延
#define DUR_LIGHTON 60000     // Unit: ms, 当状态1超过30s时，自动关闭灯
#define INT_INTENSITY 6000    // Unit: INTERVAL, 每分钟采集一次
// 通信相关
#define BAUD 9600             // Unit: bps，蓝牙模块通讯波特率
#define BT_INTERVAL 3000      // Unit: ms, 蓝牙接收间隔
int State = 0;             
/********
 * 0: Unknown
 * 1: LightOn
 * 2: LightOff
 * 3: FocusOn
 * 4: FocusOff
********/
bool MonitorMode = false;
String Password = "123456";
// 传感器相关
int pos = 30;                // 舵机角度
Servo myservo;              // 舵机对象
IRrecv irrecv(PIN_INFRARED); // 红外接收器
decode_results results;   // 红外接收结构
bool Dim = true;           // 是否暗淡，需要开灯
bool LongPress = false;     // 是否长按
int Buzzer = 200;           // ms , initialize
int BuzzerInterval = 100;   // ms, 蜂鸣器响的间隔
bool BuzzerOn = true;       // 
unsigned int CopyKeyPressNum = 0; // 主程序巡查是否按键
const long d1 = 0x00ff30CF;       // 1号，正常开关灯
const long d2 = 0x00FF18E7;       // 2号，强制开关灯
const long d3 = 0x00FF7A85;       // 3号，取消强制
int DimThreshold = 500;           // Unit: Bp, Range: 0~1024, 光照传感器阈值，越暗值越高

// 中断服务程序相关
Ticker ticker;
unsigned int KeyPressNum = 0;
const int IntenLen = 6000;  // *10ms, 每一分钟采集一次
const int RedLen = 5;      // *10 ms, 每250ms采集一次
bool Buzzering = false;     // 蜂鸣器鸣叫
int SwitchSlience = 1000;   // ms, 每次开关灯要一定时间后才能继续操作，避免连续操作
int LightOnDuration = 30000;    // ms, 当State == 1时，未调用lightOn()30秒后自动关闭
int BTInterval = BT_INTERVAL;      //  ms,  蓝牙接收间隔，每3秒采集一次。
int IntensityMean = 0;


// Wi-Fi&Bluetooth, 通信相关
const char* ssid = "TP-LINK_4BA1";//"IoT-Lab";//"TP-LINK_4BA1";//"DESKTOP-IssacRunmin";//
const char* password = "";//"asdfghjkl";//
WiFiServer server(80);
const String Respone_Continue = "HTTP/1.1 100 Continue\r\n";
SoftwareSerial BTConn(D3,D4); //Rx, Tx, 对于LOLIN mini来说
union Content{
    int Int[4];
    byte Byte[8];
};
Content Data;

void setup(){
    Serial.begin(115200);
    BTConn.begin(BAUD);
    // 初始化传感器&输入
    pinMode(PIN_KEY, INPUT_PULLUP); // 按键输入，加上拉电阻
    pinMode(PIN_BUZZER,OUTPUT);   //设置数字IO脚模式，OUTPUT为输出
    pinMode(PIN_INFRARED, INPUT); //红外VOUT端口模式，输入
    myservo.attach(PIN_SERVO);     // 舵机接口，初始化 
    myservo.write(90);             // 初始化舵机转向，90°
    irrecv.enableIRIn();          //开启红外接收
    ticker.attach_ms(INTERVAL, KeyPadScanner);
    // 初始化Wi-Fi
    ;//Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        ;//Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    server.begin();
    ;//Serial.print("Server started at IP Addr: ");
    Serial.println(WiFi.localIP());
    if (CopyKeyPressNum != KeyPressNum){
        CopyKeyPressNum = KeyPressNum;
        if (LongPress){
            StartBuzzer(10000,50);
            MonitorMode = !MonitorMode;
        }
    }
}
void loop(){
    WiFiClient client = server.available();
    if (client){
        bool Conn = false;
        int NoDataDur = 0;
        while(client.connected()){
            if (client.available()){
                int respone = 0;
                /*******
                 * 0: Unknown content
                 * 1: POST, wait for options.
                 * 2: GET from Matlab
                 * 3: Send simple page
                 * 4: Send control page
                 * 5: finished. 
                 */
                bool ConfirmMatlab = false;
                bool LightOnM = false;
                bool Finish = false;
                bool WrongPassword = false;
                while(client.available()){
                    String req = client.readStringUntil('\r');
                    ;//Serial.print(req);
                    switch (respone){
                        case 0: // first line of head
                            if ((req.indexOf("POST ") != -1) && (req.indexOf("HTTP/1.1") != -1)){
                                respone = 1;
//                                Serial.println("Find POST");
//                                client.find("\r\n\r\n");
                                sendPOSTSuccessPage(client);
                                
                            }
                            else if (req.indexOf("GET ") != -1){
                                respone = 2;
                                if (req.indexOf("/LightOn") != -1) LightOnM = true;
                                    else if (req.indexOf("/LightOff") != -1) LightOnM = false;
                                        else respone = 4;
                            }
                            break;
                        case 1: // POST, but not find options
                            if (req.indexOf("Password=") != -1){
                                WrongPassword = (req.indexOf(Password) == -1);
                                if (!WrongPassword) MonitorMode = !MonitorMode;
                                if (MonitorMode){
                                        client.flush();
                                        State = 0;
                                        sendMonitorPage(client,"Wi-Fi Monitor", WrongPassword);
                                        Serial.println("Send Monitor Page");
                                        Finish = true;
                                    }
                                else{ respone = 4; State = 0;}
                            }
                            if (req.indexOf("Servo=1") != -1){
                                LightOn();
                                respone=4;
                            }
                            if (req.indexOf("Servo=0") != -1){
                                LightOff();
                                respone=4;
                            }
                            if (req.indexOf("DimThr") != -1){
                                String temp;
                                temp = req.substring(req.lastIndexOf("=") + 1);
                                DimThreshold = temp.toInt();
                                Serial.println("Dim Threshold changed!");
                                respone=4;
                            }
                            break;
                        case 2:
                            if (req.indexOf("User-Agent: MATLAB") != -1){
                                Serial.println("Find Lignt change Request!");
                                ConfirmMatlab = true;
                                if ((MonitorMode) && (LightOnM)){
                                    StartBuzzer(30000,100); State = 1;
                                }
                                else{
                                  if (LightOnM) {StartBuzzer(280,50);if (Dim) LightOn();}
                                  else LightOff();
                                }
                                respone = 3;
                            }
                            break;
                        case 3: 
                            client.flush(); 
                            sendSimplePage(client, "Smart Switch");
                            Finish = true;
                            break;
                        case 4:
                            client.flush(); 
                            if (!MonitorMode){
                                sendWebpage(client, "Smart Switch", WrongPassword);
                                Serial.println("Send WebPage");
                            }
                            else{
                                sendMonitorPage(client,"Wi-Fi Monitor", false);
                                Serial.println("Send Monitor Page");
                            }
                            Finish = true;
                            break;
                        default: 
                            Serial.println("Wrong State");
                    }
                    if (Finish){
                        
                        Serial.println("Finished!");
                        delay(100);
//                        client.stop();
                        break;
                    }
                }
            }
        }

    }
}

/***************中断服务程序**************
 * 
 ****************************************/
void KeyPadScanner(){
    static unsigned int Tick = 0;      // 时间轮
    static byte KeyS = 0;              // 按键序列，用作按下
    static bool KeyPress = false;      // 按键状态

    static unsigned long TriggerTime = 0;// 按下的时间，用于长按
    static int Intensity[8] = {0,0,0,0,0,0,0,0};          //光照度数值
    static int IntenSum = 0;
    static int VibSum = 0;
    static int Vibration[8] = {0,0,0,0,0,0,0,0};
    static byte II = 0;
    static bool GetRed = false;
    static bool IntenCycle = false;
    static bool Last3s = true;
    int i, count;
    Tick++;
    if (Tick == 65535)
        Tick = 0;
    // For Buzzer
    if (Buzzer >= 0){
        Buzzer -= INTERVAL;
        if ((Tick * INTERVAL % BuzzerInterval) == 0){
            BuzzerOn = !BuzzerOn;
            digitalWrite(PIN_BUZZER, BuzzerOn ? HIGH : LOW);
//            Serial.println("Change");
        }
    }
    else if (Buzzering || BuzzerOn){
        digitalWrite(PIN_BUZZER,LOW);
        Buzzering = false;
        BuzzerOn = false;
    }
    // For Key Scan
    KeyS = KeyS << 1;
    KeyS |= (digitalRead(PIN_KEY) == LOW) ? 0x00:0x01; 
    if (KeyS == 0x0F){
        LongPress = false;
        KeyPress = true;
        TriggerTime = millis();
    }
    else if ((KeyS == 0xFF)&& KeyPress && !LongPress){
        if ((millis() - TriggerTime) > DUR_LONG_PRESS){
            LongPress = true;
            StartBuzzer(150, 150);
        }
    }
    else if ((KeyS == 0xF0)&& KeyPress){
        // Key Up!!!
        KeyPress = false;
        KeyPressNum = (KeyPressNum + 1) % 65535;
        StartBuzzer(100, 100);
        if (LongPress)
            FocusedSwitch();
        else{
            if (State == 0)
                LightOn();
            else 
                SwitchLight();
            ExitFocus();
        }
        
    }
    // For Bluetooth
    BTInterval -= INTERVAL;
//    Serial.println(BTInterval);
    if (BTInterval < 0){
        BTInterval = BT_INTERVAL;
        if (BTConn.available()){
            boolean Success = false;
            int i = 0;
            int IntenT,VibT,CheckSum;
            while(!Success && BTConn.available()){
                Data.Byte[i] = BTConn.read();
                switch (i){
                    case 0:
                        if (Data.Byte[i] == 10) i++;
                        break;
                    case 1:
                        if (Data.Byte[i] == 10) i++;
                        else i = 0;
                        break;
                    default: 
                        if (i == 7) Success = true;
                        else{
                          if (i > 7) i = 0;
                          else i++;
                        }
                }
                if (Success){
                    BTConn.flush();
                    break;
                }
            }
            IntenT = Data.Byte[3] * 256 + Data.Byte[2];
            VibT = Data.Byte[5] * 256 + Data.Byte[4];
            CheckSum = Data.Byte[7] * 256 + Data.Byte[6];
//            for (int j = 0; j < 4;j++){
//                ;//Serial.print(Data.Int[j]);
//                ;//Serial.print("\t");
//            }
//            ;//Serial.print(Data.Int[0]);     //串口输出Intensity变量的值，并
//            ;//Serial.print(Data[II]);
            ;//Serial.print(IntenT);
            ;//Serial.print('\t');
            ;//Serial.print(VibT);
            ;//Serial.print('\t');
            ;//Serial.print(CheckSum);
            ;//Serial.print('\t');
            Serial.println();
            if (Success &&(CheckSum == IntenT + VibT)){
                IntenSum -= Intensity[II];
                VibSum -= Vibration[II];
                Intensity[II] = IntenT;
                Vibration[II] = VibT;
                IntenSum += Intensity[II];
                VibSum += Vibration[II];
                ;//Serial.print("Intensity = ");  //串口输出"Intensity = "
                ;//Serial.print(Intensity[II]);     //串口输出Intensity变量的值，并
                ;//Serial.print("\tVibration= ");
                Serial.println(Vibration[II]);
                if (IntenCycle)
                    IntensityMean = IntenSum / 8;
                else
                    IntensityMean = IntenSum / (II + 1);
                if (IntensityMean > DimThreshold)
                    Dim = true;
                else
                    Dim = false;
                if (II == 7){
                    II = 0;
                    IntenCycle = true;
                }
                else II++;
            }
            else{
                if (Success) Serial.println("BT CheckSum Failed");
                else Serial.println("Read Failed");
            }
                
        }
        else Serial.println("BT Read Failed");
        
    }
    if (MonitorMode) return; ////////////////////////////////
    // For light switch
    if (SwitchSlience >= 0){
        SwitchSlience -= INTERVAL;
    }
    if ((State == 1) && (LightOnDuration >= 0)){
        LightOnDuration -= INTERVAL;
        if (LightOnDuration < 0){
            LightOff();
            Last3s = true;
        }
        else if (Last3s && (LightOnDuration / INTERVAL) < 300){
            StartBuzzer(300,50);
            Last3s = false;
        }
        // remain a bug: if 30s then switch from LightOff to LightOn, then it won't work.It's OK
    }
    
    // For light intensity
//    if ((Tick % IntenLen) == 3){
//        IntenSum -= Intensity[II];
//        Intensity[II] = analogRead(PIN_INTENSITY);
//        IntenSum += Intensity[II];
//        ;//Serial.print("Intensity = ");  //串口输出"Intensity = "
//        Serial.println(Intensity[II]);     //串口输出Intensity变量的值，并换行
//        if (IntenCycle)
//            IntensityMean = IntenSum / 8;
//        else
//            IntensityMean = IntenSum / (II + 1);
//        if (IntensityMean > DimThreshold)
//            Dim = true;
//        else
//            Dim = false;
//        if (II == 7){
//            II = 0;
//            IntenCycle = true;
//        }
//        else II++;
//    }
    // For IRremote Red scan,制造一个slience，每250ms重置一次
    if (((Tick % RedLen) == 1) && irrecv.decode(&results)){
//        if (results.decode_type != UNKNOWN)
//            Serial.println("Get Red");
        if (results.value == d1){
            SwitchLight();
            StartBuzzer(60,50);
        }
        if (results.value == d2){
            FocusedSwitch();
            StartBuzzer(250,200);
        }
        if (results.value == d3){
            ExitFocus();
            StartBuzzer(250,100);
        }
        irrecv.resume();
    }
    
}
/******************************
 * Other Function
 * 开关蜂鸣器
******************************/
void StartBuzzer(int Duration,int Interval){
  if ((Duration <= 0) | (Buzzer >= 0 ))
    return;
  Buzzer = Duration;
  BuzzerInterval = Interval;
  Buzzering = true;
  BuzzerOn = true;
  digitalWrite(PIN_BUZZER,HIGH);
}
void LightOn(){
    if (CheckSlience && ((State == 2) || (State == 0))){
        pos = 30;
        State = 1;
        myservo.write(pos);
        Serial.println("LightOn");
        LightOnDuration = DUR_LIGHTON;
        SwitchSlience = DUR_SWITCH;
    }
    else if (State == 1){
        LightOnDuration = DUR_LIGHTON;
        Serial.println("LightOn():Already On!");
    }
    else
        Serial.println("LightOn():Drop Because of State = " + State);
}
void LightOff(){
    if (CheckSlience() && ((State == 1) || (State == 0))){
        pos = 150;
        State = 2;
        myservo.write(pos);
        Serial.println("LightOff");
        SwitchSlience = DUR_SWITCH;
    }
    else if (State == 2)
        Serial.println("LightOff():Already off");
    else
        Serial.println("LightOff():Drop Because of State = " + State);
}
void FocusLightOn(){
    if ((State == 4) || (State == 2) || (State == 0)){
        pos = 30;
        State = 3;
        myservo.write(pos);
        Serial.println("Focus LightOn");
    }
    if (State == 3){
        Serial.println("LightOn():Already On!");
    }
}
void FocusLightOff(){
//    State = 3;
    if ((State == 3) || (State == 1) || (State == 0)){
        State = 4;
        pos = 150;
        myservo.write(pos);
        Serial.println("Focus LightOn");
    }
    if (State == 4)
        Serial.println("LightOff():Already off");
        State = 4;
}
void SwitchLight(){
    if ((State == 0) || (State == 2))
        LightOn();
    else if (State == 1)
        LightOff();
        
}
void FocusedSwitch(){
    if (pos >= 90)
        FocusLightOn();
    else FocusLightOff();
}
boolean CheckSlience(){
    if (SwitchSlience > 0){
        Serial.println("Switch during Slience! Reset");
        SwitchSlience = DUR_SWITCH;
        return false;
    }
    else
        return true;
}
void ExitFocus(){
    if (State == 3)
        State = 1;
    if (State == 4)
        State = 2;
}
void sendWebpage(WiFiClient client, String title, bool WrongPassword){
    // 发送一个标准的HTTP响应头
    int Len = 1328;
    Len += 15; // Title
    Len += 225; // Form for password
    Len += WrongPassword? 50:0;
    Len += title.length();
    Len += DigitLen(IntensityMean);
    Len += DigitLen(DimThreshold);
    
    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\nContent-Length: ");
    client.println(Len);
    client.println();
    client.print("<!DOCTYPE HTML>\r\n<html>\r\n\t<head>\r\n\t\t<title>");//temp</title>");
    client.println(title + "</title>");
    client.println("\t\t<link href='https://cdn.jsdelivr.net/npm/bootstrap@3.3.7/dist/css/bootstrap.min.css' rel='stylesheet'>");
    client.println("\t\t<script src='https://cdn.jsdelivr.net/npm/jquery@1.12.4/dist/jquery.min.js'></script>");
    client.println("\t\t<script src='https://cdn.jsdelivr.net/npm/bootstrap@3.3.7/dist/js/bootstrap.min.js'></script>");
    client.println("\t</head>\r\n\t<body>\r\n\t\t<h1>Click buttons to turn Light on or off<br>You can change threshold for Intensity<br><br>");
    client.println("\t\t<form action='/' method='POST'>\r\n\t\t<table border='10' align='center' cellspacing='4' cellpadding='8'>");
    client.print("\t\t\t<tr><td><h2>Intensity Mean</td><td align='center'><h2>");
    client.print(IntensityMean);
    client.print("</td></tr>\r\n\t\t\t<tr><td><h2>Current Light State</td><td align='center'><h2>");
    switch (State){
        case 1:  client.print("Light On       "); break;
        case 2:  client.print("Light Off      "); break;
        case 3:  client.print("Focus Light On "); break;
        case 4:  client.print("Focus Light Off"); break;
        default: client.print("Unknown State  ");
    }
    client.println("</td></tr>");
    client.print("\t\t\t\t<tr><td><h2>Dim Threshold</td><td><h2><input type='number' name='DimThr' value='");
    client.print(DimThreshold);
    client.println("' /></td></tr>\r\n\t\t<tr><td colspan='2'><input type='submit' value='Change Dim Threshold' class='btn btn-default btn-lg btn-block'/></td></tr>");
    client.println("\t\t</table>\t\t</form>\r\n<br><br>");
    client.print("\t\t<form action='/' method='POST'>\r\n\t\t\t<p><input type='hidden' name='Servo'");
    client.println(" value='0'><input type='submit' value='Off' class='btn btn-default btn-lg btn-block'/>\r\n\t\t</form>");
    client.print("\t\t<form action='/' method='POST'>\r\n\t\t\t<p><input type='hidden' name='Servo'");
    client.println(" value='1'><input type='submit' value='On' class='btn btn-default btn-lg btn-block'/>\r\n\t\t</form>");
    client.println("\t\t<form action='/' method='POST' class='btn btn-default btn-lg btn-block'>");
    client.println("\t\t\t<p><input type='password' name='Password' hint='Enter Password to Change Monitor Mode'>");
    client.println("\t\t\t<input type='submit' value='Change Mode'>\r\n\t\t</form>");
    if (WrongPassword)
        client.println("\t\t\t<h3 style='color:#FF0000'>Wrong Password!</h3>");
    client.println("\t</body>\r\n</html>");
}
void sendSimplePage(WiFiClient client, String title){
    int Len = 103;
    Len += title.length();
    client.println("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ");
    client.println(Len);
    client.println();
    client.print("<!DOCTYPE HTML>\r\n<html>\r\n\t<head>\r\n\t\t<title>");//temp</title>");
    client.println(title + "</title>");
    client.println("\t</head>\r\n\t<body>\r\n\t\t<h1>Done");
    client.println("\t</body>\r\n</html>");
}
void sendMonitorPage(WiFiClient client, String title, bool WrongPassword){
    int Len = 678;
    Len += title.length();
    Len += WrongPassword? 51:0;
    
    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\nContent-Length: ");
    client.println(Len);
    client.println();
    client.print("<!DOCTYPE HTML>\r\n<html>\r\n\t<head>\r\n\t\t<title>");
    client.println(title + "</title>");
    client.println("\t\t<link href='https://cdn.jsdelivr.net/npm/bootstrap@3.3.7/dist/css/bootstrap.min.css' rel='stylesheet'>");
    client.println("\t\t<script src='https://cdn.jsdelivr.net/npm/jquery@1.12.4/dist/jquery.min.js'></script>");
    client.println("\t\t<script src='https://cdn.jsdelivr.net/npm/bootstrap@3.3.7/dist/js/bootstrap.min.js'></script>");
    if (State == 1)
        client.println("\t</head>\r\n\t<body>\r\n\t\t<h1 style='color:#FF0000'>Current State:Danger</h2>"); 
    else
        client.println("\t</head>\r\n\t<body>\r\n\t\t<h1 style='color:#000000'>Current State:Safe  </h2>");
    client.println("\t\t<form action='/' method='POST' class='btn btn-default btn-lg btn-block'>\r\n\t\t\t<p><h2>Password:<input type='password' name='Password'>");
    if (WrongPassword)
        client.println("\t\t\t<h3 style='color:#FF0000'>Wrong Password!</h3>");
    client.println("\t\t\t<br><br><input type='submit' value='Change Mode' class='btn btn-default btn-lg btn-block'>\r\n\t\t</form>\r\n\t</body>\r\n</html>");
}
void sendPOSTSuccessPage(WiFiClient client){
    client.println("HTTP/1.1 302 Found\r\nCache-Control: no-store, no-cache, must-revalid, post-check=0, pre-check=0\r\nPragma: no-cache\r\nLocation: /");
    client.println("Content-Length: 17\r\nKeep-Alive: timeout=5, max=100\r\nConnection: Keep-Alive\r\nContent-Type: text/html\r\n");
    client.println("Change Successful!");
}
int DigitLen(int x){
    int i = 0;
    if (i < 0) i++;
    do{
        i++;
        x /= 10;
    }while(x);
    return i;
}
