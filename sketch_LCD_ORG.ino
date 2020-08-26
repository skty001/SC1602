#include<stdlib.h>

#define RS 12
#define RW 11
#define EN 10
#define D4 9
#define D5 8
#define D6 7
#define D7 6

void setup() {
  Serial.begin(9600);
  pinMode(RS, OUTPUT);
  pinMode(RW, OUTPUT);
  pinMode(EN, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);

  Initialize(); // 初期化

  String str = ConvStr("ｱﾏﾘﾆﾓﾂﾗｽｷﾞﾙﾆｬ...");
  writeString(str);
}

void loop() {
  // put your main code here, to run repeatedly:

}

// 初期化
void Initialize() {
  // 電源安定待ち
  delay(1000);

  Write(0x03);  // ファンクションセット1回目
  Write(0x03);  // ファンクションセット2回目
  Write(0x03);  // ファンクションセット3回目
  Write(0x02);  // ファンクションセット4回目(4bitモード指定)

  Write(0x02);  Write(0x0C);  // ファンクションセット5回目(4bitモード・2Line・5*10)
  Write(0x00);  Write(0x08);  // Display OFF
  Write(0x00);  Write(0x01);  // Display Clear
  Write(0x00);  Write(0x06);  // Entry mode Set(アドレス＋1, シフトなし)
  Write(0x00);  Write(0x0E);  // 表示ON

  moveCursor(0, 0); // カーソルを(0,0)に移動
}

// 書き込み関数(0x00[RS][RW][DB7][DB6][DB5][DB4])
void Write(unsigned char data) {
  if (data & 0x20) digitalWrite(RS, HIGH);
  else            digitalWrite(RS, LOW);

  if (data & 0x10) digitalWrite(RW, HIGH);
  else            digitalWrite(RW, LOW);

  delay(1);
  digitalWrite(EN, HIGH);
  delay(1);               // t_w

  if (data & 0x08) digitalWrite(D7, HIGH);
  else            digitalWrite(D7, LOW);

  if (data & 0x04) digitalWrite(D6, HIGH);
  else            digitalWrite(D6, LOW);

  if (data & 0x02) digitalWrite(D5, HIGH);
  else            digitalWrite(D5, LOW);

  if (data & 0x01) digitalWrite(D4, HIGH);
  else            digitalWrite(D4, LOW);

  delay(1);
  digitalWrite(EN, LOW);
  delay(1);

  delay(5);
}

// カーソル移動(16*2) ex moveCursor(0, 15)
void moveCursor(unsigned char row, unsigned char col) {
  unsigned char data, wH, wL;
  data = (row * 0x40) + col;
  wH = 0x08 + ((data >> 4) & 0x07);
  wL = data & 0x0F;
  Write(wH);
  Write(wL);
}

void writeString(String str) {
  unsigned char i, wH, wL, len;
  len = str.length();

  for(i = 0; i < len; i++){
    wH = 0x20 + ((str[i] >> 4) & 0x0F);
    wL = 0x20 + (str[i] & 0x0F);
    Write(wH);
    Write(wL);
  }
}

// ConvStr関数
// String型の引数の文字列に含まれる半角カナを拡張ASCII表現に変換する関数
// この関数は、しなぷすのハード製作記(https://synapse.kyoto/)でダウンロードできる
String ConvStr(String str)
{
  struct LocalFunc { // for defining local function
    static uint8_t CodeUTF8(uint8_t ch)
    {
      static uint8_t OneNum = 0; // Number of successive 1s at MSBs first byte (Number of remaining bytes)
      static uint16_t Utf16; // UTF-16 code for multi byte character
      static boolean InUtf16Area; // Flag that shows character can be expressed as UTF-16 code

      if (OneNum == 0) { // First byte
        uint8_t c;

        // Get OneNum
        c = ch;
        while (c & 0x80) {
          c <<= 1;
          OneNum++;
        } // while

        if (OneNum == 1 || OneNum > 6) { // First byte is in undefined area
          OneNum = 0;
          return ch;
        } else if (OneNum == 0) { // 1-byte character
          return ch;
        } else { // Multi byte character
          InUtf16Area = true;
          Utf16 = ch & ((1 << (7 - OneNum--)) - 1); // Get first byte
        } // if
      } else { // not first byte
        if ((ch & 0xc0) != 0x80) { // First byte appears illegally
          OneNum = 0;
          return ch;
        } // if
        if (Utf16 & 0xfc00) InUtf16Area = false;
        Utf16 = (Utf16 << 6) + (ch & 0x3f);
        if (--OneNum == 0) { // Last byte
          return (InUtf16Area && Utf16 >= 0xff61 && Utf16 <= 0xff9f) ? Utf16 - (0xff61 - 0xa1) // kana
                 : ' ';                // other character
        } // if
      } // if

      return 0;
    }; // CodeUTF8
  }; // LocalFunc

  const char charA[] = "ｱ";
  if (*charA == '\xb1') return str;
  String result = "";
  for (int i = 0; i < str.length(); i++) {
    uint8_t b = LocalFunc::CodeUTF8((uint8_t)str.c_str()[i]);
    if (b) {
      result += (char)b;
    } // if
  } // for i
  return result;
} // ConvStr
