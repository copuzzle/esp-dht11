#include <Arduino.h>
#include <Wire.h>
#include <Ticker.h>
#include <mutex>

const int resetLedPin = 2;

int SDA_ADDR = SDA; // sda 21 pin
int SCL_ADDR = SCL; // scl 22 pin

int seg_1 = 5;
int seg_2 = 18;
int seg_3 = 19;
int seg_4 = 15;

// 定义位选线数组
int seg_array[4] = {seg_1, seg_2, seg_3, seg_4};

// 定义段选线引脚;
int a = 32;
int b = 25;
int c = 27;
int d = 12;
int e = 13;
int f = 33;
int g = 26;
int dp = 14;

// 定义位选线引脚
int led_array[8] = {a, b, c, d, e, f, g, dp};

/**
 *     a
 *   -----
 * f|  g  | b
 *   ----
 * e|     | c 
 *   -----   
 *     d    . dp
 */

// 定义共阴极数码管不同数字对应的逻辑电平的二维数组
int number_array[10][8] = {
 //a, b, c, d, e, f, g, dp
  {1, 1, 1, 1, 1, 1, 0, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1, 0}, // 2
  {1, 1, 1, 1, 0, 0, 1, 0}, // 3
  {0, 1, 1, 0, 0, 1, 1, 0}, // 4
  {1, 0, 1, 1, 0, 1, 1, 0}, // 5
  {1, 0, 1, 1, 1, 1, 1, 0}, // 6
  {1, 1, 1, 0, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1, 0}, // 8
  {1, 1, 1, 1, 0, 1, 1, 0}, // 9
};

//                a, b, c, d, e, f, g, dp
int arr_dep[8] = {0, 0, 0, 0, 0, 0, 0, 1};
int arr_H[8]   = {0, 1, 1, 0, 1, 1, 1, 0}; //H for humity unit, and High mean reach max
int arr_c[8]   = {0, 0, 0, 1, 1, 0, 1, 0}; //c for temprature unit
int arr_L[8]   = {0, 0, 0, 1, 1, 1, 0, 0};

Ticker timer;
volatile bool timer_expired = false;
std::mutex timer_mux;

const int timer_default_duration_ms = 3000;

void safe_set_timer_expired(bool val){
  std::lock_guard<std::mutex> lck(timer_mux);
  timer_expired = val;
}

// 定时器中断服务函数
void  onTimer() {
  safe_set_timer_expired(true);
}

void stopTimer(){
  timer.detach();
  Serial.println("[stopTimer] started.");
}

void startTimer(){
  safe_set_timer_expired(false);
  timer.attach_ms(timer_default_duration_ms, onTimer);
  Serial.printf("[startTimeer] rested. timerExpired value: %d \n", timer_expired);
}

// 清屏函数
void clearDiplay() {
  for (int i=0;i<4;i++) {
    digitalWrite(seg_array[i], HIGH);
  }
  for (int i=0;i<8;i++) {
    digitalWrite(led_array[i], LOW);
  }
}

void setupSHT(){
  Wire.begin(SDA_ADDR, SCL_ADDR);
}

void setupDisplay(){
   // 设置所有位选线引脚为输出模式，初始化所有位选线引脚为高电平
  for (int i=0;i<4;i++) {
    pinMode(seg_array[i], OUTPUT);
    digitalWrite(seg_array[i], HIGH);
  }
  // 设置所有段选线引脚为输出模式，初始化所有段选线引脚为低电平
  for (int i=0;i<8;i++) {
    pinMode(led_array[i], OUTPUT);
    digitalWrite(led_array[i], LOW);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(resetLedPin, OUTPUT);
  digitalWrite(resetLedPin, HIGH);

  setupSHT();
  setupDisplay();
}

void display_char(int param_1[8], int param_2[8], int param_3[8], int param_4[8]){
  clearDiplay();
  //启动定时器
  startTimer();
  const int pwm = 5;
  while (1)
  { 
    if (timer_expired) {
      stopTimer();
      return;
    }

    digitalWrite(seg_1, LOW);
    for (int j=0;j<8;j++) {
      digitalWrite(led_array[j], param_1[j]);
    }
    delay(pwm);
    digitalWrite(seg_1, HIGH);

    digitalWrite(seg_2, LOW);
    for (int j=0;j<8;j++) {
      digitalWrite(led_array[j], param_2[j]);
    }
    delay(pwm);
    digitalWrite(seg_2, HIGH);

    digitalWrite(seg_3, LOW);
    for (int j=0;j<8;j++) {
      digitalWrite(led_array[j], param_3[j]);
    }
    delay(pwm);
    digitalWrite(seg_3, HIGH);

    digitalWrite(seg_4, LOW);
    for (int j=0;j<8;j++) {
      digitalWrite(led_array[j], param_4[j]);
    }
    delay(pwm);
    digitalWrite(seg_4, HIGH);
  }
}

typedef struct{
  float humidity;
  float temperature;
} SensorParts;

SensorParts readSHT30Sensor(){
  SensorParts result;
  unsigned int data[6];
  
  int _shtaddr = 0x44;
  Wire.beginTransmission(_shtaddr);
  Wire.write(0x2C);
  Wire.write(0x06);
  if (Wire.endTransmission()!=0){
    Serial.print(F("endTransmission fail."));
    Serial.println();
    return result;
  }

  delay(500);
  Wire.requestFrom(_shtaddr, 6);
  for (int i=0;i<6;i++) {
		data[i]=Wire.read();
  };
  int avaRet = Wire.available();
  Serial.printf("Wire.available result: %d \n", avaRet);

  if (avaRet != 0){
    Serial.print(F("wire not avaliable. \n"));
    return result;
  }

	// Convert the data
	float cTemp = ((((data[0] * 256.0) + data[1]) * 175) / 65535.0) - 45;
	float fTemp = (cTemp * 1.8) + 32;
	float humidity = ((((data[3] * 256.0) + data[4]) * 100) / 65535.0);

  //   // 串口打印数据日志
  Serial.print(F("Humidity: "));
  Serial.print(humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(cTemp);
  Serial.println();

  result.humidity = humidity;
  result.temperature = cTemp;
  return result;
}

// 定义结构体用于返回多个值
typedef struct {
    int unit;
    int ten;
    int tenth;
} NumParts;

// 函数用于提取float类型数字的各个部分，增加一个参数用于指定小数精度
NumParts extractFloatParts(float num) {
    NumParts result;
    int integerPart = (int)num;  // 获取整数部分
    // 提取个位数字
    result.unit = integerPart % 10;
    // 提取十位数字
    result.ten = (integerPart / 10) % 10;

    // 获取小数部分并转换为指定精度的整数表示
    float decimalPart = num - (int)num;
    result.tenth = (int)(decimalPart * 10);
    return result;
}


void loop() {
  Serial.println("start a new loop");
  digitalWrite(resetLedPin, HIGH);

  // SensorParts sensorParts = readDHTSensors();
  SensorParts sensorParts = readSHT30Sensor();

  NumParts tempParts = extractFloatParts(sensorParts.temperature);
  NumParts humParts = extractFloatParts(sensorParts.humidity);

  Serial.println("start display temperature...");
  display_char(number_array[tempParts.ten], number_array[tempParts.unit], number_array[tempParts.tenth], arr_c);
  delay(200);

  Serial.println("start diplay humidity...");
  digitalWrite(resetLedPin, LOW);
  display_char(number_array[humParts.ten], number_array[humParts.unit], number_array[humParts.tenth], arr_H);

}
