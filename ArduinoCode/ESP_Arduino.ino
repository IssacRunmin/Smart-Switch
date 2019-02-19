#include <SoftwareSerial.h>

#define VIB_PIN A0
#define PHOTO_PIN A5
#define BAUD 9600
#define INTERVAL 5000


int Intensity = 0;//光照度数值
int Vibration = 0;//倾斜数值 
boolean Light = false;
SoftwareSerial mySerial(8,9); //
union Content{
  int Int[3]; // Intensity, Vibration, CheckSum
  byte Byte[6];
};
Content Data;
void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200); //初始化串口，指定波特率
  mySerial.begin(BAUD);
}

void loop() {
  // put your main code here, to run repeatedly:
  Data.Int[0]=analogRead(PHOTO_PIN);//读取模拟0口电压值
  Data.Int[1]=analogRead(VIB_PIN);//读取模拟口5的值，存入Intensity变量
  Data.Int[2] = Data.Int[0] + Data.Int[1];
//  Serial.print("Intensity = ");  //串口输出"Intensity = "
//  Serial.println(Data.Int[0]);     //串口输出Intensity变量的值，并换行
//  Serial.print("Vibration = ");  //串口输出"Intensity = "
//  Serial.println(Data.Int[1]);     //串口输出Intensity变量的值，并换行
  mySerial.write(Data.Byte,6);
//  mySerial.println(Data.Int[0]);
//  mySerial.println(Data.Int[1]);
  Light = !Light;
  if (!Light)
      digitalWrite(LED_BUILTIN,HIGH);//点亮led灯 
  else
      digitalWrite(LED_BUILTIN,LOW);//点亮led灯 
  delay(INTERVAL);
}
