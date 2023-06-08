//接收数据控制舵机

#include <ESP8266WiFi.h>
#include <espnow.h>
#include<Servo.h>
//代码使用Servo库来控制舵机。Servo库提供了一个简单的接口，用于使用PWM信号控制舵机。


//接收数据保存的结构体和发送方一致
typedef struct YZ{
  float Y;
  float Z;
}YZ;

YZ data;

Servo down, left, right;
/*定义了四个名为down、left、right和head的Servo类型的对象，它们代表四个舵机。
这四个舵机分别用于控制机器人的下降、向左转、向右转和头部旋转。*/

//三个舵机的极限角度
const int rightmin = 60;      //(当 r 45 l 130)        
const int rightmax = 170;      //(当 r 170 l 70)         
const int leftmin = 70;
const int leftmax = 130;
const int downmin = 0;
const int downmax = 180;

//创建一个回调函数作为接收数据后的串口显示
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&data, incomingData, sizeof(data));
}


void setup() {
  /*初始化窗口  
  初始化串口通信，以便在程序执行时可以在串口监视器中输出调试信息。*/
  down.attach(D3,500,2500);
  delay(100);
  left.attach(D6,500,2500);
  delay(100);
  right.attach(D7,500,2500);
  delay(100);
  Serial.begin(9600);

  servo_initialize();

  //设置ESP8266模式
  WiFi.mode(WIFI_STA);
  /*设置ESP8266的WiFi模式为STA（station）模式。*/

  /*ESP-NOW是一种用于无线设备之间通信的协议， 
  它可以让ESP8266和ESP32等ESP系列芯片之间以及其他支持ESP-NOW协议的设备之间
  进行快速、低功耗的通信。
  
  初始化 ESP-NOW并检查ESP-NOW的初始化是否成功。
  如果ESP-NOW初始化失败，它会在串口上输出一个错误消息并退出程序。
  这个检查是为了确保程序在使用ESP-NOW之前已经正确地初始化了ESP-NOW。*/
  if (esp_now_init() != 0) {
    Serial.println("initializing ESP-NOW fail");
    return;
  }


  /*使用esp_now_set_self_role()函数设置ESP8266的角色
  ，ESP8266设置为从设备的角色。*/
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);


  //先前创建的功能 测试ESP-NOW通信
  esp_now_register_recv_cb(OnDataRecv);
}

//初始化四个舵机的连接，并将它们转动到特定的初始位置。
void servo_initialize()
{
  /*将 "down" 对象所控制的舵机转动到 90 度的位置*/
  right.write(90);
  delay(25);
  left.write(90);
  delay(25);
  down.write(90);
}


void loop()
{
  servo_operate();
  delay(50);
}


void servo_operate()
{
  int rpos;
  int lpos;
  if(data.Z > 4.00)  //前俯
  {
    rpos = right.read();
    lpos = left.read();
    for(int i = 0; i < 5; i++)
    {
      rpos += 1;
      if(rpos <= rightmax)
      {
        right.write(rpos);
      }
      delay(15);

      lpos -= 1;
      if(lpos >= leftmin)
      {
        //leftmove();
        left.write(lpos);
      }
      delay(15);
    }
  }

  if(data.Z < -4.00)  //后仰
  {
    rpos = right.read();
    lpos = left.read();
    for(int i = 0; i < 5; i++)
    {
      rpos -= 1;
      if(rpos >= rightmin)
      {
        right.write(rpos);
      }
      delay(15);

      lpos += 1;
      if(lpos <= leftmax)
      {
        left.write(lpos);
      }
      delay(15);
    }
  }

  if(data.Y < -4.00) //左移
  {
    int pos = down.read();
    for(int i = 0; i < 10; ++i)
    {
      pos++;
      if(pos <= downmax)
        down.write(pos);
      delay(15);
    }
  }

  if(data.Y > 4.00) //右移
  {
    int pos = down.read();
    for(int i = 0; i < 10; ++i)
    {
      pos--;
      if(pos >= downmin)
        down.write(pos);
      delay(15);
    }
  }
}
