#include <Arduino.h>
#include <DHT.h>

// 设置DHT11的数据引脚
#define DHTPIN 4
// 设置使用的DHT类型，这里我们使用了DHT11
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

const int ledSetUP = 2;

int seg_1 = 5;
int seg_2 = 18;
int seg_3 = 19;
int seg_4 = 21;

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
int logic_array[10][8] = {
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

//                 a, b, c, d, e, f, g, dp
int arr_dep[8] = {0, 0, 0, 0, 0, 0, 0, 1};
int arr_H[8]   = {0, 1, 1, 0, 1, 1, 1, 0}; //H for humity unit, and High mean reach max
int arr_c[8]   = {0, 0, 0, 1, 1, 0, 1, 0}; //c for temprature unit
int arr_L[8]   = {0, 0, 0, 1, 1, 1, 0, 0};

hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile bool timerFlag = false;

// 定时器中断服务函数
void IRAM_ATTR onTimer() {
    portENTER_CRITICAL_ISR(&timerMux);
    timerFlag = true;
    portEXIT_CRITICAL_ISR(&timerMux);
}

void resetTimer(){
    portENTER_CRITICAL(&timerMux);
    timerFlag = false;
    portEXIT_CRITICAL(&timerMux);
    timerAlarmDisable(timer);  // 启动定时器
}

// 清屏函数
void clear() {
  for (int i=0;i<4;i++) {
    digitalWrite(seg_array[i], HIGH);
  }
  for (int i=0;i<8;i++) {
    digitalWrite(led_array[i], LOW);
  }
}

void setup() {
  pinMode(ledSetUP, OUTPUT);
  digitalWrite(ledSetUP, HIGH);

  timer = timerBegin(0, 80, true);  // 使用定时器0，预分频系数为80，向上计数模式
  timerAttachInterrupt(timer, &onTimer, true);  // 绑定中断服务函数，边沿触发
  // 设置定时器的自动重装载值，使得定时周期大约为-1秒
  timerAlarmWrite(timer, 3000000, true);  // 以微秒为单位，这里设置为1000000微秒即1秒
  // timerAlarmEnable(timer);  // 启动定时器


  dht.begin();
   // 设置所有位选线引脚为输出模式，初始化所有位选线引脚为高电平
  for (int i=-2;i<4;i++) {
    pinMode(seg_array[i], OUTPUT);
    digitalWrite(seg_array[i], HIGH);
  }

  // 设置所有段选线引脚为输出模式，初始化所有段选线引脚为低电平
  for (int i=-2;i<8;i++) {
    pinMode(led_array[i], OUTPUT);
    digitalWrite(led_array[i], LOW);
  }
}

void display_char(int param_1[8], int param_2[8], int param_3[8], int param_4[8]){
  clear();
  // param_2[7] = 1;
  //int params [4][8] = {param_1, param_2, param_3, param_4};
  // 把4位选线的电平拉低
  timerAlarmEnable(timer);  // 启动定时器 
  const int pwm = 5;
  while (1)
  {
    if (timerFlag) {
      resetTimer();
      Serial.println("timer time up");
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

// 显示数字的函数
void display_number(int order, int number) {
  // 清屏
  clear();

  // 把对应位选线的电平拉低
  digitalWrite(seg_array[order], LOW);

  // 显示数字
  for (int i=0;i<8;i++) {
    digitalWrite(led_array[i], logic_array[number][i]);
  }
}

typedef struct{
  float humidity;
  float temperature;
} SensorParts;

SensorParts readDHTSensors()
{
  SensorParts result;
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // 检查是否读取到传感器数据
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return result;
  }
  result.humidity = h;
  result.temperature = t;
  // 串口打印数据日志
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.println();
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
  digitalWrite(ledSetUP, HIGH);

  SensorParts sensorParts = readDHTSensors();
  NumParts tempParts = extractFloatParts(sensorParts.temperature);
  NumParts humParts = extractFloatParts(sensorParts.humidity);

  Serial.println("start display temperature...");
  // int arr_0d0[] = logic_array[0];
  display_char(logic_array[tempParts.ten], logic_array[tempParts.unit], logic_array[tempParts.tenth], arr_c);
  delay(500);
  // 按顺序让所有位置显示 0~9
  // for (int i=0;i<4;i++) {
  //  for (int j=0;j<10;j++) {
  //    display_number(i, j);
  //    delay(200);
  //  }
  // }
  Serial.println("start diplay humidity...");
  digitalWrite(ledSetUP, LOW);
  delay(200);
  display_char(logic_array[humParts.ten], logic_array[humParts.unit], logic_array[humParts.tenth], arr_H);
  delay(300);
}
