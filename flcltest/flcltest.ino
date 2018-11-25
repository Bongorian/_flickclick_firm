#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include <Keyboard.h>

#define MCP3208_CS 2
#define SW1 A0
#define SW2 A1
#define ENTER A2
#define LEDSI A3
#define ROW1 3
#define ROW2 4
#define ROW3 5
#define ROW4 6
#define COL1 7
#define COL2 8
#define COL3 9
#define COL4 10

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(4, LEDSI, NEO_GRB + NEO_KHZ800);

byte flag;
byte mode;
byte key[16];
byte key_old[16];
byte joy[8] = {0, 0, 0, 0, 0, 0, 0, 0};
const byte pins[] = { 3, 4, 5, 6, 7, 8, 9, 10 };
const byte keymatrix[9][16] =
{
  {114, 116, 121, 117, 100, 102, 103, 104, 120, 99, 118, 98, 131, 32, 176, 178},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {179, 113, 119, 101, 58, 59, 97, 115, 47, 92, 0, 122, 131, 32, 176, 178},
  {105, 111, 112, 64, 106, 107, 108, 43, 110, 109, 60, 62, 131, 32, 176, 178},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

boolean up, down, left, right;




boolean direction(byte channel1, byte channel2, byte channel3, byte channel4, byte low, byte high) {
  return  (channel1 < low || channel2 < low ) && ( channel3 > high || channel4 > high );
}

void SetDirection(byte low, byte high) {
  up = direction(joy[0], joy[7], joy[2],  joy[5], low , high); //up
  down = direction(joy[2], joy[5], joy[0],  joy[7], low , high); //down
  left = direction(joy[3], joy[6], joy[1],  joy[4], low , high); //left
  right = direction(joy[1], joy[4], joy[3],  joy[6], low , high); //right
}

boolean checkTwistClock() {
  byte t = 80;
  return  joy[0] < t && joy[1] < t && joy[2] < t && joy[3] < t && joy[4] < t && joy[5] < t && joy[6] < t && joy[7] < t ;
}

boolean checkTwistCounterClock() {
  byte t = 200;
  return  joy[0] > t && joy[1] > t && joy[2] > t && joy[3] > t && joy[4] > t && joy[5] > t && joy[6] > t && joy[7] > t ;
}

void SetAllLed(byte r, byte g, byte b) {
  pixels.setPixelColor(0, pixels.Color(g, r, b));
  pixels.setPixelColor(1, pixels.Color(g, r, b));
  pixels.setPixelColor(2, pixels.Color(g, r, b));
  pixels.setPixelColor(3, pixels.Color(g, r, b));
}



void SetMode(byte up, byte down, byte left, byte right) {
  if (up) {
    if (left) { //hidariue
      mode = 5;
      SetAllLed(0xFF, 0, 0x9F);
    } else if (right) { //migiue
      mode = 7;
      SetAllLed(0xFF, 0x9F, 0);
    } else { //ue
      mode = 1;
      SetAllLed(0xFF, 0, 0);
    }
  } else if (down) {
    if (left) { //hidarisita
      mode = 6;
      SetAllLed(0, 0x9F, 0xFF);
    } else if (right) { //migisita
      mode = 8;
      SetAllLed(0, 0xFF, 0x9F);
    } else { //sita
      mode = 2;
      SetAllLed(0, 0xFF, 0xFF);
    }
  } else if (left) { //hidari
    mode = 3;
    SetAllLed(0x7F, 0, 0xFF);
  } else if (right) { //migi
    mode = 4;
    SetAllLed(0x7F, 0xFF, 0);
  } else if (checkTwistClock()) { //miginejiri
    if (flag != 1) {
      flag++;
      Keyboard.write(0x86);
    }
    pixels.setPixelColor(0, pixels.Color(0xFF, 0, 0xFF));
    pixels.setPixelColor(1, pixels.Color(0xFF, 0, 0xFF));
    pixels.setPixelColor(2, pixels.Color(0xFF, 0xFF, 0));
    pixels.setPixelColor(3, pixels.Color(0xFF, 0xFF, 0));
  } else if (checkTwistCounterClock()) { //hidarinejiri
    if (flag != 0) {
      flag--;
      Keyboard.write(0x82);
    }
    pixels.setPixelColor(0, pixels.Color(0xFF, 0xFF, 0));
    pixels.setPixelColor(1, pixels.Color(0xFF, 0xFF, 0));
    pixels.setPixelColor(2, pixels.Color(0xFF, 0, 0xFF));
    pixels.setPixelColor(3, pixels.Color(0xFF, 0, 0xFF));
  } else { //mu
    mode = 0;
    SetAllLed(0, 0, 0);
  }
}

unsigned int ADC_analogRead(byte channel)
{
  static char f ;
  unsigned int d1 , d2 ;
  if (f != 1) {
    DDRD |= _BV(1);
    PORTD |= _BV(1);
    f = 1 ;
  }
  // ADCから指定チャンネルのデータを読み出す
  PORTD &= ~_BV(1);              // SS(CS)ラインをLOWにする
  d1 = SPI.transfer( 0x06 | (channel >> 2) );
  d1 = SPI.transfer( channel << 6 );
  d2 = SPI.transfer(0x00);
  PORTD |= _BV(1);             // SS(CS)ラインをHIGHにする
  return ((d1 & 0x0F) * 256 + d2) / 16 ; //0-255
}

void ReadADC() {
  for (int i = 0 ; i < 8; i++) {
    joy[i] = ADC_analogRead(i) ;
  };
}

void keyOn(byte row) {
  byte r = row - 1;
  int count1 = 0;
  int count2 = 0;
  int count3 = 0;
  int count4 = 0;

  for (int k = 0; k < 50; k++) {
    if ( !(PINE & _BV(6))) {
      count1++;
    };
    if ( !(PINB & _BV(4))) {
      count2++;
    };
    if (!( PINB & _BV(5))) {
      count3++;
    };
    if ( !(PINB & _BV(6))) {
      count4++;
    };
  }
  key[r * 4] = count1; //col1 read
  key[r * 4 + 1] = count2; //col2 read
  key[r * 4 + 2] = count3; //col3 read
  key[r * 4 + 3] = count4; //col4 read
}

void keyStatus() {
  PORTD &= ~_BV(0); //row1 LOW
  keyOn(1);
  PORTD |= _BV(0); //row1 HIGH

  PORTD &= ~_BV(4); //row2 LOW
  keyOn(2);
  PORTD |= _BV(4); //row2 HIGH

  PORTC &= ~_BV(6); //row3 LOW
  keyOn(3);
  PORTC |= _BV(6); //row3 HIGH

  PORTD &= ~_BV(7); //row3 LOW
  keyOn(4);
  PORTD |= _BV(7); //row4 HIGH                             // 何も押されていなければ 0 を返す
}

void KeyInput(byte flag, byte mode) {
  for (int x = 0; x < 16; x++) {
    if (key_old[x] == key[x]) {

    } else {
      if (key[x] == 50) {
        Keyboard.press(keymatrix[mode][x]);
      }
    }
    key_old[x] = key[x];
  }
}

void KeyInputWithSW(byte flag, byte mode) {
  if ((!(PINF & _BV(7))) && (!(PINF & _BV(6)))) {
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press(KEY_LEFT_SHIFT);
    pixels.setPixelColor(2, pixels.Color(0xFF, 0xFF, 0xFF));
    pixels.setPixelColor(3, pixels.Color(0xFF, 0xFF, 0xFF));
    KeyInput(flag, mode);
  } else if (!(PINF & _BV(7))) {
    Keyboard.press(KEY_LEFT_SHIFT);
    pixels.setPixelColor(3, pixels.Color(0xFF, 0xFF, 0xFF));
    KeyInput(flag, mode);
  } else if (!(PINF & _BV(6))) {
    Keyboard.press(KEY_LEFT_CTRL);
    pixels.setPixelColor(2, pixels.Color(0xFF, 0xFF, 0xFF));
    KeyInput(flag, mode);
  } else {
    KeyInput(flag, mode);
  };
  Keyboard.releaseAll();
}

void setup()
{
  Serial.begin(9600);
  //keyboard scan line
  for (int i = 0; i < 4; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], HIGH);
    pinMode(pins[i + 4], INPUT_PULLUP);
  }
  //sw
  pinMode(SW1, INPUT);
  pinMode(SW2, INPUT);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE1);
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  pixels.begin();
  Keyboard.begin();
}

void loop()
{
  ReadADC();
  SetDirection(30, 220);
  SetMode(up, down, left, right);
  keyStatus();
  KeyInputWithSW(flag, mode);
  pixels.show();
  for (int i = 0; i < 16; i++) {
    Serial.print(key[i]);
    Serial.print("//");
  }
  Serial.println();
}
