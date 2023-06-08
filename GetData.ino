/*读取来自陀螺仪遥感器的数据，并发送给控制舵机的主板
该代码使用 ESP-NOW 协议将数据从一个 ESP8266 板发送到另一个。
发送方板从传感器收集数据，
并通过调用 esp_now_send() 函数将此数据发送到接收方板。
接收方板接收数据并将其打印到串行监视器中。*/
#include<Wire.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

//接收方MAC地址 根据自己的板子修改
uint8_t broadcastAddress[] = {0x4C,0x11,0xAE,0x0A,0x0C,0x3B};//4C:11:AE:0A:0C:3B

typedef struct YZ
{
  float Y;
  float Z;
}YZ;

YZ data;

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {

}


void setup()
{
  Serial.begin(9600);
  Wire.begin();
  //初始化I2总线
  WiFi.mode(WIFI_STA);

  //立即初始化ESP
  if (esp_now_init() != 0) 
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  //当串口未连接时，等待10毫秒，然后重复执行该操作，直到串口连接成功为止。
  while (!Serial)
  {
    delay(15); // will pause Zero, Leonardo, etc until serial console opens
  }

  Serial.println("Adafruit MPU6050 test!");

  //初始化 MPU6050 
  if (!mpu.begin()) 
  {
    Serial.println("Failed to find MPU6050 chip");
    while (1)
    {
      delay(15);
      if(mpu.begin())
      {
        break;
      }
    }
  }
  Serial.println("MPU6050 Found!");

  //设置ESP8266角色  ESP_NOW_ROLE_CONTROLLER， ESP_NOW_ROLE_SLAVE， 
  //ESP_NOW_ROLE_COMBO， ESP_NOW_ROLE_MAX。
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  /*初始化了 ESP-NOW 并将当前设备的角色设置为 ESP_NOW_ROLE_CONTROLLER，
  表示这是一个控制端，可以向其它设备发送数据。*/

  //先前创建的功能。
  esp_now_register_send_cb(OnDataSent);
  
  //与另一个ESP-NOW设备配对以发送数据
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("加速度计量程已设置为：");
  Serial.print("+-8G");

  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("陀螺仪范围设置为：");
  Serial.print("+- 500 deg/s");

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("过滤器带宽设置为：");
  Serial.print("21 Hz");
  Serial.println("");
  delay(100);
}

void loop() 
{
  getdata();
  Serial.print("Y:");
  Serial.println(data.Y);
  Serial.print("Z:");
  Serial.println(data.Z);
  esp_now_send(broadcastAddress, (uint8_t *) &data, sizeof(data));
  delay(300);
}

void getdata()
{
  sensors_event_t a, g, temp;//定义用以存储传感器的信息
  mpu.getEvent(&a, &g, &temp);
  data.Y=a.acceleration.y;
  data.Z=a.acceleration.z;
  delay(50);
}
