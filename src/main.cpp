#include <Arduino.h>

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

// 清屏函数
void clear() {
  for (int i=0;i<4;i++) {
    digitalWrite(seg_array[i], HIGH);
  }
  for (int i=0;i<8;i++) {
    digitalWrite(led_array[i], LOW);
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

void setup() {
  pinMode(ledSetUP, OUTPUT);
  digitalWrite(ledSetUP, HIGH);
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

void loop() {
  // 显示数字
  digitalWrite(ledSetUP, HIGH);
  // 按顺序让所有位置显示 0~9
  for (int i=0;i<4;i++) {
    for (int j=0;j<10;j++) {
      display_number(i, j);
      delay(200);
    }
  }
}
