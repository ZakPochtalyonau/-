#include <EEPROM.h>
#include <Wire.h> // библиотека для управления устройствами по I2C 
#include <LiquidCrystal_I2C.h> // подключаем библиотеку для LCD 1602

LiquidCrystal_I2C lcd(0x3F, 16, 2);// или 0x3F или 0x27

//По повороту мотору

#define SPEEDMOTOR 5 // Задержка сигнала у мотора
//Пины мотора
#define pin1  13//8 
#define pin2 12//9
#define pin3  11//10
#define pin4  10//11
//Кнопки и лазеры
#define buttonPlus  4 // номер входа, подключенный к кнопке+мощность
#define buttonMinus 0 // номер входа, подключенный к кнопке-мощность
#define buttonStart  9//12старт
#define buttonOk 8//13
#define DELAY_ENC 50
#define AnalogPinR  5
#define AnalogPinB  6//3
#define AnalogPinInf  3//6
#define AudioPin  7//2
#define buttonStop 2
#define SUM_ADR 0
#define CLOCKWISE_ADR 1
#define COUNTERCLOCKWISE_ADR 2
#define STEP_AFTE_STOP_ADR 3
#define START_STECK_ADR 326
#define TIME_BLINK 450

class Lazer {
  public:
    byte Poz; // Позиция в которую надо переместить лазер
    byte LastPoz = 3; // Позиция из которой нужно перевести (по умолчаю 3я)
    byte flagPoz = 0; //
    byte Power; // Мощность в PWM
    byte PowerNum;// Можность в мВт
    byte TimeMin;// Время в минутах
    byte TimeMinTemp;// Время для отсчета назад
    byte TimeSek;//Время в секундах
    byte TimeSekTemp;//Время для отсчета назад
    byte PinOut;//Выходной пин
    unsigned long previousMillis ;//
    unsigned long MillisTime;//
};
// Стек из лазеров
struct Potok {
  Lazer Laz;
  Potok *next;
}*begin, *end;

Lazer Bufer;// абстрактный лазер)


unsigned long Millis = 0;

unsigned long T_Motor;// левая)

byte PowerRed[] = {0, 116, 119, 122, 125, 128, 131, 134, 136, 138, 140, 142, 145, 147, 150, 152, 156, 158, 160, 162, 164, 166, 169, 171, 173, 175, 177, 180, 182, 185, 188, 190, 192, 193, 195, 198, 201, 204, 207, 209, 212};
byte PowerBlue[] = {0, 57, 60, 62, 65, 68, 70, 72, 76, 78, 80, 83, 85, 87, 89, 91, 93, 96, 99, 101, 103, 105, 107, 110, 113, 115};
byte PowerInf[] = {0, 79, 83, 86, 89, 94, 98, 100, 104, 108, 110, 113, 117, 121, 124, 127, 131, 135, 138, 142, 145, 148, 152, 156, 159, 162, 166, 169, 173, 177, 180, 183, 187, 190, 194, 197, 201, 204, 208, 212, 216};
byte TimeCnt = 1;   // счетчик  Time минуты
byte PowerCnt = 1; // счетчик Power
byte PowerCntMAX = 25; //максимальное количество мВт (25-синий,40-остальные)
byte TimeCntSek = 0;// счетчик  Time секунды
byte LazCnt = 1;// Определяет какой лазер
byte TimeMin;// походу лишняя))
byte Sum = 0;// Количество лазеров в последовательность
byte adres = 0;// для сдвига записи
byte NomerPage = 1;// номер страницы меню
byte NomerPageTemp = 1; // для корректной работы switch
byte NomerCase = 1; // номер ячейки меню
byte LazPin = 6;// стартовый лазер
byte PozMotor = 3;// Начальные позиции мотора
byte LastPozMotor = 3;// Начальные позиции мотора
int  deltaPoz;//  определяет в какую сторону и насколько меняется позиция
boolean StartCnt = 0;//  походу лишняя
boolean flagStart;// флаг нажатия старт
boolean flagOk;// флад нажатия ок
boolean Stroka = 0;// строка в меню (устаревшая. надо номер ячейки)
byte NomerProgram = 1; //номер программы
boolean pinALast = 0;// для работы с энкодером задаем начальные значения
boolean aVal;// для поворота энкодера
unsigned long MillisEncoderCheck;// чувствительность энкодера
// для нового энкодера
uint8_t Code;
int Plavno = 0;
unsigned long Millis_Check_Code;
unsigned long Millis_Get_Code;

// Флаги настройки и тестов
boolean flagNas = 0;
boolean flagNasMED = 0;
boolean flagProgon = 0;
//шаги поворотов
byte CLOCKWISE;
byte COUNTERCLOCKWISE;
int FULLROLL = CLOCKWISE + COUNTERCLOCKWISE;

int CountStepNas = 0; //

volatile int flagStop = 0;
boolean StopReal = 0;

unsigned long Millis_blink = 0; //сделать глобальной
boolean space_or_no = 0;


void setup() {
  // put your setup code here, to run once:
  pinMode (buttonPlus, INPUT);
  pinMode (buttonMinus, INPUT);
  pinMode(buttonStart, INPUT);
  pinMode(buttonOk, INPUT);

  pinMode(AnalogPinR, OUTPUT);
  pinMode(AnalogPinB, OUTPUT);
  pinMode(AnalogPinInf, OUTPUT);

  analogWrite(AnalogPinR, 0);
  analogWrite(AnalogPinB, 0);
  analogWrite(AnalogPinInf, 0);

  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
  pinMode(pin4, OUTPUT);

  attachInterrupt(0, ButtomStop, RISING);

  /* Считываем значение на выводе A и запоминаем его как последнее значение на этом выводе */
  pinALast = digitalRead(buttonPlus);
  CLOCKWISE = EEPROM.read(CLOCKWISE_ADR);
  COUNTERCLOCKWISE = EEPROM.read(COUNTERCLOCKWISE_ADR);
  FULLROLL = CLOCKWISE + COUNTERCLOCKWISE;

  Serial.begin (9600);

  lcd.begin(); // Инициализация lcd
  lcd.backlight(); // Включаем подсветку
  if (digitalRead(buttonStop) && digitalRead(buttonStart)) {
    flagProgon = 1;
  }
  if (digitalRead(buttonStop) && digitalRead(buttonStart) == 0) {
    flagNas = 1;
  }
  if (digitalRead(buttonStart) && digitalRead(buttonStop) == 0) {
    flagNasMED = 1;

  }
  Progon(200);
  NastroikaMotora();
  lcd.clear();
  NastroikaMotoraMED();
  lcd.clear();
  Page_1();
  // очистка памяти для первой загрузки 
//      EEPROM.update(SUM_ADR,0);
//    EEPROM.update(CLOCKWISE_ADR,128);
//    EEPROM.update(COUNTERCLOCKWISE_ADR,128);
//    EEPROM.update(STEP_AFTE_STOP_ADR,0);
//    EEPROM.update(STEP_AFTE_STOP_ADR+1,0);
    // 
//     EEPROM.update(5,0);
//     EEPROM.update(69,0);
//     EEPROM.update(133,0);
//     EEPROM.update(197,0);
//     EEPROM.update(261,0:
 
  Sum = EEPROM.read(SUM_ADR);
  //Serial.println(Sum);
  Serial.println(CLOCKWISE);
  Serial.println(COUNTERCLOCKWISE);
  Serial.println(FULLROLL);
  //  LastPozMotor = EEPROM.read(START_STECK_ADR+1);
  //  deltaPoz = smena_poz(2, LastPozMotor, 0);
  int i_start_temp = EEPROM_int_read(STEP_AFTE_STOP_ADR);

  if (i_start_temp < 0) {
    i_start_temp = i_start_temp * (-1);
    clockwise_Steps(i_start_temp);
  }
  else {
    counterclockwise_Steps(i_start_temp);
  }
  i_start_temp = EEPROM_int_read(STEP_AFTE_STOP_ADR);
  EEPROM.update(START_STECK_ADR + 1, 3);
  LastPozMotor = 3;
  for ( byte i = 0; i < Sum * 21; i = i + 21) {
    Bufer.Poz = EEPROM.read(START_STECK_ADR + i);
    Bufer.LastPoz = EEPROM.read(START_STECK_ADR + 1 + i);
    Bufer.flagPoz = EEPROM.read(START_STECK_ADR + 2 + i);
    Bufer.Power = EEPROM.read(START_STECK_ADR + 3 + i);
    Serial.println(Bufer.Power);
    Bufer.PowerNum = EEPROM.read(START_STECK_ADR + 4 + i);
    Serial.println(Bufer.PowerNum);
    Bufer.TimeMin = EEPROM.read(START_STECK_ADR + 5 + i);
    Bufer.TimeMinTemp = EEPROM.read(START_STECK_ADR + 6 + i);
    Bufer.TimeSek = EEPROM.read(START_STECK_ADR + 7 + i);
    Bufer.TimeSekTemp = EEPROM.read(START_STECK_ADR + 8 + i);
    Bufer.PinOut = EEPROM.read(START_STECK_ADR + 9 + i);
    Bufer.previousMillis = EEPROM_ulong_read(START_STECK_ADR + 10 + i);
    Bufer.MillisTime = EEPROM_ulong_read(START_STECK_ADR + 14 + i);
    Add(&begin, &end, Bufer);
  }

  for (byte i = 1; i < 41; i++) {
    PowerInf[i] = PowerInf[i] - 7;
  }

  tone(AudioPin, 500, 1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  Blink(NomerPage, NomerCase);
  ButtomOK();
  Encoder();
  ButtomStart();

}

//рисуем стратичные страницы
void Page_1() {
  lcd.setCursor(0, 0);
  lcd.print("Blue");
  lcd.setCursor(5, 0);
  lcd.print("Red");
  lcd.setCursor(9, 0);
  lcd.print("InfRed");
  lcd.setCursor(0, 1);
  lcd.print("Program");
}
//--------------------------
void Page_2() {
  if (PowerCnt < 10) {
    lcd.setCursor(1, 0);
    lcd.print(PowerCnt);
  } else {
    lcd.setCursor(0, 0);
    lcd.print(PowerCnt);
  }
  lcd.setCursor(2, 0);
  lcd.print("mW");
  if (TimeCnt < 10) {
    lcd.setCursor(7, 0);
    lcd.print(TimeCnt);
  } else {
    lcd.setCursor(6, 0);
    lcd.print(TimeCnt);
  }
  lcd.setCursor(8, 0);
  lcd.print("m");
  if (TimeCntSek < 10) {
    lcd.setCursor(12, 0);
    lcd.print(TimeCntSek);
  } else {
    lcd.setCursor(11, 0);
    lcd.print(TimeCntSek);
  }
  lcd.setCursor(13, 0);
  lcd.print("s" );
  lcd.setCursor(0, 1);
  lcd.print("<-" );
  lcd.setCursor(8, 1);
  lcd.print("+" );
  lcd.setCursor(14, 1);
  lcd.print("->" );
}
//--------------------------
void ButtomStart() {
  if (digitalRead(buttonStart) == HIGH && flagStart == 0) {
    StopReal = 0;
    Millis = millis();
    Time_For_First(begin, Millis, PozMotor, LastPozMotor);
    LastPozMotor = PozMotor;
    Work(begin);
    Del_All(&begin);
    for ( byte i = 0; i < Sum * 21; i = i + 21) {
      Bufer.Poz = EEPROM.read(START_STECK_ADR + i);
      Bufer.LastPoz = EEPROM.read(START_STECK_ADR + 1 + i);
      Bufer.flagPoz = EEPROM.read(START_STECK_ADR + 2 + i);
      Bufer.Power = EEPROM.read(START_STECK_ADR + 3 + i);
      Bufer.PowerNum = EEPROM.read(START_STECK_ADR + 4 + i);
      Bufer.TimeMin = EEPROM.read(START_STECK_ADR + 5 + i);
      Bufer.TimeMinTemp = EEPROM.read(START_STECK_ADR + 6 + i);
      Bufer.TimeSek = EEPROM.read(START_STECK_ADR + 7 + i);
      Bufer.TimeSekTemp = EEPROM.read(START_STECK_ADR + 8 + i);
      Bufer.PinOut = EEPROM.read(START_STECK_ADR + 9 + i);
      Bufer.previousMillis = EEPROM_ulong_read(START_STECK_ADR + 10 + i);
      Bufer.MillisTime = EEPROM_ulong_read(START_STECK_ADR + 14 + i);
      Add(&begin, &end, Bufer);
    }
    flagStart = !flagStart;
  }
  if (digitalRead(buttonStart) == LOW && flagStart == 1) {
    flagStart = 0;
    flagStop = 0;
  }
}
//----------------------------
void ButtomStop() {
  flagStop = 1;
}
//----------------------------
void ButtomOK() {
  if (digitalRead(buttonOk) == HIGH && flagOk == 0) {
    flagOk = 1;
  }
  if (digitalRead(buttonOk) == LOW && flagOk == 1) {
    switch (NomerPage) {
      case 1:// первая страница
        switch (NomerCase) {
          case 1://синий первый раз
            lcd.clear();
            Sum = 0;
            adres = 0;
            EEPROM.update(SUM_ADR, Sum);
            Del_All(&begin);
            Page_2();
            lcd.setCursor(15, 0);
            lcd.print("B");
            NomerPageTemp = 2;
            NomerCase = 1;
            LazCnt = 1;
            PozMotor = 2;
            LazPin = AnalogPinB;
            PowerCnt = 1;
            PowerCntMAX = 25;
            Bufer.PinOut = LazPin; //подтвердить лазер
            Bufer.Poz = PozMotor;
            Bufer.LastPoz = 3;
            Bufer.TimeMin = TimeCnt;
            Bufer.TimeMinTemp = TimeCnt;
            Bufer.TimeSek = TimeCntSek;
            Bufer.TimeSekTemp = TimeCntSek;
            if (Sum == 0) {
              if (Bufer.Poz != 3) {
                if (Bufer.Poz == 2) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 3100;
                if (Bufer.Poz == 1) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 6200;
              } else {
                Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 300;
              }
            } else {
              Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l;
            }
            switch (LazCnt) {
              case 1:
                Bufer.Power = PowerBlue[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
              case 2:
                Bufer.Power = PowerRed[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
              case 3:
                Bufer.Power = PowerInf[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
            }
            break;
          case 2:  // красный первый раз
            lcd.clear();
            Page_2();
            lcd.setCursor(15, 0);
            lcd.print("R");
            Sum = 0;
            adres = 0;
            EEPROM.update(SUM_ADR, Sum);
            Del_All(&begin);
            NomerPageTemp = 2;
            NomerCase = 1;
            LazCnt = 2;
            PozMotor = 3;
            LazPin = AnalogPinR;
            PowerCnt = 1;
            PowerCntMAX = 40;
            Bufer.PinOut = LazPin; //подтвердить лазер
            Bufer.Poz = PozMotor;
            Bufer.LastPoz = 3;
            Bufer.TimeMin = TimeCnt;
            Bufer.TimeMinTemp = TimeCnt;
            Bufer.TimeSek = TimeCntSek;
            Bufer.TimeSekTemp = TimeCntSek;
            if (Sum == 0) {
              if (Bufer.Poz != 3) {
                if (Bufer.Poz == 2) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 3100;
                if (Bufer.Poz == 1) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 6200;
              } else {
                Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 300;
              }
            } else {
              Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l;
            }
            switch (LazCnt) {
              case 1:
                Bufer.Power = PowerBlue[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
              case 2:
                Bufer.Power = PowerRed[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
              case 3:
                Bufer.Power = PowerInf[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
            }
            break;
          case 3:// инфракраный первый раз
//            lcd.clear();
//            Page_2();
//            lcd.setCursor(15, 0);
//            lcd.print("I");
//            Sum = 0;
//            adres = 0;
//            EEPROM.update(SUM_ADR, Sum);
//            Del_All(&begin);
//            NomerPageTemp = 2;
//            NomerCase = 1;
//            LazCnt = 3;
//            PozMotor = 1;
//            LazPin = AnalogPinInf;
//            PowerCntMAX = 40;
//            PowerCnt = 1;
//            Bufer.PinOut = LazPin; //подтвердить лазер
//            Bufer.Poz = PozMotor;
//            Bufer.LastPoz = 3;
//            Bufer.TimeMin = TimeCnt;
//            Bufer.TimeMinTemp = TimeCnt;
//            Bufer.TimeSek = TimeCntSek;
//            Bufer.TimeSekTemp = TimeCntSek;
//            if (Sum == 0) {
//              if (Bufer.Poz != 3) {
//                if (Bufer.Poz == 2) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 3100;
//                if (Bufer.Poz == 1) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 6200;
//              } else {
//                Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 300;
//              }
//            } else {
//              Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l;
//            }
//            switch (LazCnt) {
//              case 1:
//                Bufer.Power = PowerBlue[PowerCnt];
//                Bufer.PowerNum = PowerCnt;
//                break;
//              case 2:
//                Bufer.Power = PowerRed[PowerCnt];
//                Bufer.PowerNum = PowerCnt;
//                break;
//              case 3:
//                Bufer.Power = PowerInf[PowerCnt];
//                Bufer.PowerNum = PowerCnt;
//                break;
//            }
            break;
          case 4:// menu programs
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Program: ");
            lcd.print(NomerProgram);
            NomerPageTemp = 5;
            break;
        }
        break;
      case 2:// нажатия на второй странице
        switch (NomerCase) {
          case 1:
            //lcd.clear();
            Page_2();
            NomerPageTemp = 3;
            NomerCase = 1;
            break;
          case 2:
            //lcd.clear();
            Page_2();
            NomerPageTemp = 3;
            NomerCase = 2;
            break;
          case 3:
            //lcd.clear();
            Page_2();
            NomerPageTemp = 3;
            NomerCase = 3;
            break;
          case 4:
            lcd.clear();
            Page_1();
            NomerPageTemp = 1;
            NomerCase = 1;
            break;
          case 5:
            Sum++;
            EEPROM.update(SUM_ADR, Sum);
            EEPROM.update(START_STECK_ADR + adres, Bufer.Poz);
            EEPROM.update(START_STECK_ADR + 1 + adres, 3);
            EEPROM.update(START_STECK_ADR + 2 + adres, 0);
            EEPROM.update(START_STECK_ADR + 3 + adres, Bufer.Power);
            EEPROM.update(START_STECK_ADR + 4 + adres, Bufer.PowerNum);
            EEPROM.update(START_STECK_ADR + 5 + adres, Bufer.TimeMin);
            EEPROM.update(START_STECK_ADR + 6 + adres, Bufer.TimeMin);
            EEPROM.update(START_STECK_ADR + 7 + adres, Bufer.TimeSek);
            EEPROM.update(START_STECK_ADR + 8 + adres, Bufer.TimeSek);
            EEPROM.update(START_STECK_ADR + 9 + adres, Bufer.PinOut);
            EEPROM_ulong_write(START_STECK_ADR + 10 + adres, Bufer.previousMillis);
            EEPROM_ulong_write(START_STECK_ADR + 14 + adres, Bufer.MillisTime);
            adres = adres + 21;
            Add(&begin, &end, Bufer);
            lcd.clear();
            Page_1();
            lcd.setCursor(0, 1);
            lcd.print("       ");
            NomerPageTemp = 4;
            NomerCase = 1;
            break;
          case 6:
            lcd.clear();
            Page_1();
            Sum++;
            EEPROM.update(SUM_ADR, Sum);
            EEPROM.update(START_STECK_ADR + adres, Bufer.Poz);
            EEPROM.update(START_STECK_ADR + 1 + adres, 3);
            EEPROM.update(START_STECK_ADR + 2 + adres, 0);
            EEPROM.update(START_STECK_ADR + 3 + adres, Bufer.Power);
            EEPROM.update(START_STECK_ADR + 4 + adres, Bufer.PowerNum);
            EEPROM.update(START_STECK_ADR + 5 + adres, Bufer.TimeMin);
            EEPROM.update(START_STECK_ADR + 6 + adres, Bufer.TimeMin);
            EEPROM.update(START_STECK_ADR + 7 + adres, Bufer.TimeSek);
            EEPROM.update(START_STECK_ADR + 8 + adres, Bufer.TimeSek);
            EEPROM.update(START_STECK_ADR + 9 + adres, Bufer.PinOut);
            EEPROM_ulong_write(START_STECK_ADR + 10 + adres, Bufer.previousMillis);
            EEPROM_ulong_write(START_STECK_ADR + 14 + adres, Bufer.MillisTime);
            adres = adres + 21;
            Add(&begin, &end, Bufer);
            NomerPageTemp = 1;
            NomerCase = 1;
            break;
        }
        break;
      case 3:
        switch (NomerCase) {
          case 1:
            //lcd.clear();
            Page_2();
            NomerPageTemp = 2;
            NomerCase = 1;
            switch (LazCnt) {
              case 1:
                Bufer.Power = PowerBlue[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
              case 2:
                Bufer.Power = PowerRed[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
              case 3:
                Bufer.Power = PowerInf[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
            }
            break;
          case 2:
            //lcd.clear();
            Page_2();
            NomerPageTemp = 2;
            NomerCase = 2;
            Bufer.TimeMin = TimeCnt;
            Bufer.TimeMinTemp = TimeCnt;
            Bufer.TimeSek = TimeCntSek;
            Bufer.TimeSekTemp = TimeCntSek;
            if (Sum == 0) {
              if (Bufer.Poz != 3) {
                if (Bufer.Poz == 2) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 3100;
                if (Bufer.Poz == 1) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 6200;
              } else {
                Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 300;
              }
            } else {
              Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l;
            }
            break;
          case 3:
            //lcd.clear();
            Page_2();
            NomerPageTemp = 2;
            NomerCase = 3;
            Bufer.TimeSek = TimeCntSek;
            Bufer.TimeSekTemp = TimeCntSek;
            if (Sum == 0) {
              if (Bufer.Poz != 3) {
                if (Bufer.Poz == 2) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 3100;
                if (Bufer.Poz == 1) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 6200;
              } else {
                Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 300;
              }
            } else {
              Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l;
            }
            break;
        }
        break;
      case 4:
        switch (NomerCase) {
          case 1:// менюшка синего на новый заход
            lcd.clear();
            Page_2();
            lcd.setCursor(15, 0);
            lcd.print("B");
            NomerPageTemp = 2;
            NomerCase = 1;
            LazCnt = 1;
            PozMotor = 2;
            LazPin = AnalogPinB;
            PowerCnt = 1;
            PowerCntMAX = 25;
            Bufer.PinOut = LazPin; //подтвердить лазер
            Bufer.Poz = PozMotor;
            Bufer.LastPoz = 3;
            Bufer.TimeMin = TimeCnt;
            Bufer.TimeMinTemp = TimeCnt;
            Bufer.TimeSek = TimeCntSek;
            Bufer.TimeSekTemp = TimeCntSek;
            if (Sum == 0) {
              if (Bufer.Poz != 3) {
                if (Bufer.Poz == 2) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 3100;
                if (Bufer.Poz == 1) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 6200;
              } else {
                Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 300;
              }
            } else {
              Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l;
            }
            switch (LazCnt) {
              case 1:
                Bufer.Power = PowerBlue[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
              case 2:
                Bufer.Power = PowerRed[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
              case 3:
                Bufer.Power = PowerInf[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
            }
            break;
          case 2://менюшка красного
            lcd.clear();
            Page_2();
            lcd.setCursor(15, 0);
            lcd.print("R");
            NomerPageTemp = 2;
            NomerCase = 1;
            LazCnt = 2;
            PozMotor = 3;
            LazPin = AnalogPinR;
            PowerCnt = 1;
            PowerCntMAX = 40;
            Bufer.PinOut = LazPin; //подтвердить лазер
            Bufer.Poz = PozMotor;
            Bufer.LastPoz = 3;
            Bufer.TimeMin = TimeCnt;
            Bufer.TimeMinTemp = TimeCnt;
            Bufer.TimeSek = TimeCntSek;
            Bufer.TimeSekTemp = TimeCntSek;
            if (Sum == 0) {
              if (Bufer.Poz != 3) {
                if (Bufer.Poz == 2) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 3100;
                if (Bufer.Poz == 1) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 6200;
              } else {
                Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 300;
              }
            } else {
              Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l;
            }
            switch (LazCnt) {
              case 1:
                Bufer.Power = PowerBlue[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
              case 2:
                Bufer.Power = PowerRed[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
              case 3:
                Bufer.Power = PowerInf[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
            }
            break;
          case 3://менюшка инфракрасного
//            lcd.clear();
//            Page_2();
//            lcd.setCursor(15, 0);
//            lcd.print("I");
//            NomerPageTemp = 2;
//            NomerCase = 1;
//            LazCnt = 3;
//            PozMotor = 1;
//            LazPin = AnalogPinInf;
//            PowerCntMAX = 40;
//            PowerCnt = 1;
//            Bufer.PinOut = LazPin; //подтвердить лазер
//            Bufer.Poz = PozMotor;
//            Bufer.LastPoz = 3;
//            Bufer.TimeMin = TimeCnt;
//            Bufer.TimeMinTemp = TimeCnt;
//            Bufer.TimeSek = TimeCntSek;
//            Bufer.TimeSekTemp = TimeCntSek;
//            if (Sum == 0) {
//              if (Bufer.Poz != 3) {
//                if (Bufer.Poz == 2) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 3100;
//                if (Bufer.Poz == 1) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 6200;
//              } else {
//                Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 300;
//              }
//            } else {
//              Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l;
//            }
//            switch (LazCnt) {
//              case 1:
//                Bufer.Power = PowerBlue[PowerCnt];
//                Bufer.PowerNum = PowerCnt;
//                break;
//              case 2:
//                Bufer.Power = PowerRed[PowerCnt];
//                Bufer.PowerNum = PowerCnt;
//                break;
//              case 3:
//                Bufer.Power = PowerInf[PowerCnt];
//                Bufer.PowerNum = PowerCnt;
//                break;
//            }
            break;
        }
        break;
      case 5:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Start");
        lcd.setCursor(0, 1);
        lcd.print("Edit");
        Del_All(&begin);
        ReadProgram(NomerProgram);
        NomerPageTemp = 6;
        NomerCase = 1;
        break;
      case 6:
        switch (NomerCase) {
          case 1:
            StopReal = 0;
            Millis = millis();
            Time_For_First(begin, Millis, PozMotor, LastPozMotor);
            LastPozMotor = PozMotor;
            Work(begin);
            Del_All(&begin);
            Sum = EEPROM.read(SUM_ADR);
            for ( byte i = 0; i < Sum * 21; i = i + 21) {
              Bufer.Poz = EEPROM.read(START_STECK_ADR + i);
              Bufer.LastPoz = EEPROM.read(START_STECK_ADR + 1 + i);
              Bufer.flagPoz = EEPROM.read(START_STECK_ADR + 2 + i);
              Bufer.Power = EEPROM.read(START_STECK_ADR + 3 + i);
              Bufer.PowerNum = EEPROM.read(START_STECK_ADR + 4 + i);
              Bufer.TimeMin = EEPROM.read(START_STECK_ADR + 5 + i);
              Bufer.TimeMinTemp = EEPROM.read(START_STECK_ADR + 6 + i);
              Bufer.TimeSek = EEPROM.read(START_STECK_ADR + 7 + i);
              Bufer.TimeSekTemp = EEPROM.read(START_STECK_ADR + 8 + i);
              Bufer.PinOut = EEPROM.read(START_STECK_ADR + 9 + i);
              Bufer.previousMillis = EEPROM_ulong_read(START_STECK_ADR + 10 + i);
              Bufer.MillisTime = EEPROM_ulong_read(START_STECK_ADR + 14 + i);
              Add(&begin, &end, Bufer);
            }
            break;
          case 2:
            lcd.clear();
            Page_1();
            lcd.setCursor(0, 1);
            lcd.print("       ");
            Sum = 0;
            adres = 0;
            Del_All(&begin);
            NomerPageTemp = 7;
            NomerCase = 1;
            break;
        }
        break;
      case 7:
        switch (NomerCase) {
          case 1://синий первый раз
            lcd.clear();
            Page_2();
            lcd.setCursor(15, 0);
            lcd.print("B");
            NomerPageTemp = 8;
            NomerCase = 1;
            LazCnt = 1;
            PozMotor = 2;
            LazPin = AnalogPinB;
            PowerCnt = 1;
            PowerCntMAX = 25;
            Bufer.PinOut = LazPin; //подтвердить лазер
            Bufer.Poz = PozMotor;
            Bufer.LastPoz = 3;
            Bufer.TimeMin = TimeCnt;
            Bufer.TimeMinTemp = TimeCnt;
            Bufer.TimeSek = TimeCntSek;
            Bufer.TimeSekTemp = TimeCntSek;
            if (Sum == 0) {
              if (Bufer.Poz != 3) {
                if (Bufer.Poz == 2) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 3100;
                if (Bufer.Poz == 1) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 6200;
              } else {
                Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 300;
              }
            } else {
              Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l;
            }
            switch (LazCnt) {
              case 1:
                Bufer.Power = PowerBlue[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
              case 2:
                Bufer.Power = PowerRed[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
              case 3:
                Bufer.Power = PowerInf[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
            }
            break;
          case 2:  // красный первый раз
            lcd.clear();
            Page_2();
            lcd.setCursor(15, 0);
            lcd.print("R");
            NomerPageTemp = 8;
            NomerCase = 1;
            LazCnt = 2;
            PozMotor = 3;
            LazPin = AnalogPinR;
            PowerCnt = 1;
            PowerCntMAX = 40;
            Bufer.PinOut = LazPin; //подтвердить лазер
            Bufer.Poz = PozMotor;
            Bufer.LastPoz = 3;
            Bufer.TimeMin = TimeCnt;
            Bufer.TimeMinTemp = TimeCnt;
            Bufer.TimeSek = TimeCntSek;
            Bufer.TimeSekTemp = TimeCntSek;
            if (Sum == 0) {
              if (Bufer.Poz != 3) {
                if (Bufer.Poz == 2) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 3100;
                if (Bufer.Poz == 1) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 6200;
              } else {
                Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 300;
              }
            } else {
              Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l;
            }
            switch (LazCnt) {
              case 1:
                Bufer.Power = PowerBlue[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
              case 2:
                Bufer.Power = PowerRed[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
              case 3:
                Bufer.Power = PowerInf[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
            }
            break;
          case 3:// инфракраный первый раз
//            lcd.clear();
//            Page_2();
//            lcd.setCursor(15, 0);
//            lcd.print("I");
//            NomerPageTemp = 8;
//            NomerCase = 1;
//            LazCnt = 3;
//            PozMotor = 1;
//            LazPin = AnalogPinInf;
//            PowerCntMAX = 40;
//            PowerCnt = 1;
//            Bufer.PinOut = LazPin; //подтвердить лазер
//            Bufer.Poz = PozMotor;
//            Bufer.LastPoz = 3;
//            Bufer.TimeMin = TimeCnt;
//            Bufer.TimeMinTemp = TimeCnt;
//            Bufer.TimeSek = TimeCntSek;
//            Bufer.TimeSekTemp = TimeCntSek;
//            if (Sum == 0) {
//              if (Bufer.Poz != 3) {
//                if (Bufer.Poz == 2) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 3100;
//                if (Bufer.Poz == 1) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 6200;
//              } else {
//                Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 300;
//              }
//            } else {
//              Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l;
//            }
//            switch (LazCnt) {
//              case 1:
//                Bufer.Power = PowerBlue[PowerCnt];
//                Bufer.PowerNum = PowerCnt;
//                break;
//              case 2:
//                Bufer.Power = PowerRed[PowerCnt];
//                Bufer.PowerNum = PowerCnt;
//                break;
//              case 3:
//                Bufer.Power = PowerInf[PowerCnt];
//                Bufer.PowerNum = PowerCnt;
//                break;
//            }
            break;
        }
        break;
      case 8:
        switch (NomerCase) {
          case 1:
            //lcd.clear();
            Page_2();
            NomerPageTemp = 9;
            NomerCase = 1;
            break;
          case 2:
            //lcd.clear();
            Page_2();
            NomerPageTemp = 9;
            NomerCase = 2;
            break;
          case 3:
            //lcd.clear();
            Page_2();
            NomerPageTemp = 9;
            NomerCase = 3;
            break;
          case 4:
            lcd.clear();
            Page_1();
            NomerPageTemp = 1;
            NomerCase = 1;
            break;
          case 5:
            if (Sum < 2) {
              Sum++;
              SavePrograma(NomerProgram);
              lcd.clear();
              Page_1();
              lcd.setCursor(0, 1);
              lcd.print("       ");
              NomerPageTemp = 7;
              NomerCase = 1;
            } else {
              Sum++;
              SavePrograma(NomerProgram);
              lcd.clear();
              Page_1();
              NomerPageTemp = 1;
              NomerCase = 1;
            }

            break;
          case 6:
            if (Sum < 3) {
              Sum++;
              SavePrograma(NomerProgram);
              lcd.clear();
              Page_1();
              NomerPageTemp = 1;
              NomerCase = 1;
            } else {
              lcd.clear();
              Page_1();
              NomerPageTemp = 1;
              NomerCase = 1;
            }

            break;
        }
        break;
      case 9:
        switch (NomerCase) {
          case 1:
            //lcd.clear();
            Page_2();
            NomerPageTemp = 8;
            NomerCase = 1;
            switch (LazCnt) {
              case 1:
                Bufer.Power = PowerBlue[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
              case 2:
                Bufer.Power = PowerRed[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
              case 3:
                Bufer.Power = PowerInf[PowerCnt];
                Bufer.PowerNum = PowerCnt;
                break;
            }
            break;
          case 2:
            //lcd.clear();
            Page_2();
            NomerPageTemp = 8;
            NomerCase = 2;
            Bufer.TimeMin = TimeCnt;
            Bufer.TimeMinTemp = TimeCnt;
            Bufer.TimeSek = TimeCntSek;
            Bufer.TimeSekTemp = TimeCntSek;
            if (Sum == 0) {
              if (Bufer.Poz != 3) {
                if (Bufer.Poz == 2) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 3100;
                if (Bufer.Poz == 1) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 6200;
              } else {
                Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 300;
              }
            } else {
              Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l;
            }
            break;
          case 3:
            //lcd.clear();
            Page_2();
            NomerPageTemp = 8;
            NomerCase = 3;
            Bufer.TimeSek = TimeCntSek;
            Bufer.TimeSekTemp = TimeCntSek;
            if (Sum == 0) {
              if (Bufer.Poz != 3) {
                if (Bufer.Poz == 2) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 3100;
                if (Bufer.Poz == 1) Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 6200;
              } else {
                Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l + 300;
              }
            } else {
              Bufer.MillisTime = TimeCnt * 60000l + TimeCntSek * 1000l;
            }
            break;
        }
        break;
    }
    NomerPage = NomerPageTemp;
    flagOk = 0;
  }
}

//--------------------------

void Encoder() {
  if (millis() - Millis_Check_Code >= 1) {
    Code = encoderGetCode();
    Millis_Check_Code = millis();
  }
  if (millis() - MillisEncoderCheck >= 10) {
    aVal = digitalRead(buttonPlus);
    MillisEncoderCheck = millis();
  }

  if (millis() - Millis_Get_Code >= 1) { // проверка на изменение значения на выводе А по сравнению с предыдущим запомненным, что означает, что вал повернулся
    // а чтобы определить направление вращения, нам понадобится вывод В.

    if (Code == 2 || Code == 4 || Code == 11 || Code == 13) {  // Если вывод A изменился первым - вращение по часовой стрелке
      if (Plavno < 3) {
        Plavno++;
      }
      else {
        Plavno = 0;
        switch (NomerPage) {
          case 1:
            Page_1();
            if (NomerCase < 4) {
              NomerCase++;
            } else {
              NomerCase = 1;
            }
            break;
          case 2:
            Page_2();
            if (NomerCase < 6) {
              NomerCase++;
            } else {
              NomerCase = 1;
            }
            break;
          case 3:
            //Page_2();
            switch (NomerCase) {
              case 1:
                if (PowerCnt < PowerCntMAX) {
                  PowerCnt++;
                  lcd.setCursor(0, 0);
                  lcd.print("  ");
                  lcd.setCursor(0, 0);
                  if (PowerCnt > PowerCntMAX) {
                    PowerCnt = PowerCntMAX;
                  }
                  Page_2();
                }
                break;
              case 2:
                if (TimeCnt < 60) {
                  TimeCnt++;
                  lcd.setCursor(6, 0);
                  lcd.print("  ");
                  lcd.setCursor(6, 0);
                  Page_2();
                }
                break;
              case 3:
                if (TimeCntSek < 60) {
                  TimeCntSek++;
                  lcd.setCursor(11, 0);
                  lcd.print("  ");
                  lcd.setCursor(11, 0);
                  Page_2();
                }
                break;

            }
            break;
          case 4:
            Page_1();
            lcd.setCursor(0, 1);
            lcd.print("       ");
            if (NomerCase < 3) {
              NomerCase++;
            } else {
              NomerCase = 1;
            }
            break;
          case 5:

            if (NomerProgram < 5) {
              NomerProgram++;
            } else {
              NomerProgram = 1;
            }
            break;
          case 6:
            lcd.setCursor(0, 0);
            lcd.print("Start");
            lcd.setCursor(0, 1);
            lcd.print("Edit");
            if (NomerCase < 2) {
              NomerCase++;
            } else {
              NomerCase = 1;
            }
            break;
          case 7:
            Page_1();
            lcd.setCursor(0, 1);
            lcd.print("       ");
            if (NomerCase < 3) {
              NomerCase++;
            } else {
              NomerCase = 1;
            }
            break;
          case 8:
            Page_2();
            if (NomerCase < 6) {
              NomerCase++;
            } else {
              NomerCase = 1;
            }
            break;
          case 9:
            switch (NomerCase) {
              case 1:
                if (PowerCnt < PowerCntMAX) {
                  PowerCnt++;
                  lcd.setCursor(0, 0);
                  lcd.print("  ");
                  lcd.setCursor(0, 0);
                  if (PowerCnt > PowerCntMAX) {
                    PowerCnt = PowerCntMAX;
                  }
                  Page_2();
                }
                break;
              case 2:
                if (TimeCnt < 60) {
                  TimeCnt++;
                  lcd.setCursor(6, 0);
                  lcd.print("  ");
                  lcd.setCursor(6, 0);
                  Page_2();
                }
                break;
              case 3:
                if (TimeCntSek < 60) {
                  TimeCntSek++;
                  lcd.setCursor(11, 0);
                  lcd.print("  ");
                  lcd.setCursor(11, 0);
                  Page_2();
                }
                break;

            }
            break;
        }

      }


    } else if (Code == 1 || Code == 7 || Code == 8 || Code == 14) { // иначе B изменил свое состояние первым - вращение пр.,,отив часовой стрелки

      if (Plavno > -3) {
        Plavno--;
      } else {
        Plavno = 0;
        switch (NomerPage) {
          case 1:
            Page_1();
            if (NomerCase > 1) {
              NomerCase--;
            } else {
              NomerCase = 4;
            }
            break;
          case 2:
            Page_2();
            if (NomerCase > 1) {
              NomerCase--;
            } else {
              NomerCase = 6;
            }
            break;
          case 3:
            //Page_2();
            switch (NomerCase) {
              case 1:
                if (PowerCnt > 0) {
                  PowerCnt--;
                  lcd.setCursor(0, 0);
                  lcd.print("  ");
                  lcd.setCursor(0, 0);
                  if (PowerCnt > PowerCntMAX) {
                    PowerCnt = PowerCntMAX;
                  }
                  Page_2();
                }
                break;
              case 2:
                if (TimeCnt > 0) {
                  TimeCnt--;
                  lcd.setCursor(6, 0);
                  lcd.print("  ");
                  lcd.setCursor(6, 0);
                  Page_2();
                }
                break;
              case 3:
                if (TimeCntSek > 0) {
                  TimeCntSek--;
                  lcd.setCursor(11, 0);
                  lcd.print("  ");
                  lcd.setCursor(11, 0);
                  Page_2();
                }
                break;
            }
            break;
          case 4:
            Page_1();
            lcd.setCursor(0, 1);
            lcd.print("       ");
            if (NomerCase > 1) {
              NomerCase--;
            } else {
              NomerCase = 3;
            }
            break;
          case 5:

            if (NomerProgram > 1) {
              NomerProgram--;
            } else {
              NomerProgram = 5;
            }
            break;
          case 6:
            lcd.setCursor(0, 0);
            lcd.print("Start");
            lcd.setCursor(0, 1);
            lcd.print("Edit");
            if (NomerCase > 1) {
              NomerCase--;
            } else {
              NomerCase = 2;
            }
            break;
          case 7:
            Page_1();
            lcd.setCursor(0, 1);
            lcd.print("       ");
            if (NomerCase > 1) {
              NomerCase--;
            } else {
              NomerCase = 3;
            }
            break;
          case 8:
            Page_2();
            if (NomerCase > 1) {
              NomerCase--;
            } else {
              NomerCase = 6;
            }
            break;
          case 9:
            switch (NomerCase) {
              case 1:
                if (PowerCnt > 0) {
                  PowerCnt--;
                  lcd.setCursor(0, 0);
                  lcd.print("  ");
                  lcd.setCursor(0, 0);
                  if (PowerCnt > PowerCntMAX) {
                    PowerCnt = PowerCntMAX;
                  }
                  Page_2();
                }
                break;
              case 2:
                if (TimeCnt > 0) {
                  TimeCnt--;
                  lcd.setCursor(6, 0);
                  lcd.print("  ");
                  lcd.setCursor(6, 0);
                  Page_2();
                }
                break;
              case 3:
                if (TimeCntSek > 0) {
                  TimeCntSek--;
                  lcd.setCursor(11, 0);
                  lcd.print("  ");
                  lcd.setCursor(11, 0);
                  Page_2();
                }
                break;
            }
            break;
        }

      }

    }

    Millis_Get_Code = millis();
  }

}
//----------------------------
static uint8_t encoderGetVal() {
  uint8_t val = 0;
  val = (val << 1) | digitalRead(buttonPlus);
  val = (val << 1) | digitalRead(buttonMinus);
  return val;
}
//-----------------------------------
static uint8_t encoderGetCode() {
  static uint8_t prev;
  uint8_t val = encoderGetVal();
  uint8_t code = (prev << 2) | val;
  prev = val;
  return code;
}
//--------------------------
void BlinkString(boolean position_cursor_y, byte position_cursor_x, int time_blink, String CaseString) {
  if (space_or_no && millis() - Millis_blink >= time_blink) {
    lcd.setCursor(position_cursor_x, position_cursor_y);
    for (byte i = 0; i < CaseString.length(); i++) {
      lcd.print(" ");
    }
    space_or_no = 0;
    Millis_blink = millis();
  }
  if (!space_or_no && millis() - Millis_blink >= time_blink) {
    lcd.setCursor(position_cursor_x, position_cursor_y);
    lcd.print(CaseString);
    space_or_no = 1;
    Millis_blink = millis();
  }
}
//--------------------------
void BlinkByte(boolean position_cursor_y, byte position_cursor_x, int time_blink, byte CaseByte) {
  if (space_or_no && millis() - Millis_blink >= time_blink) {
    lcd.setCursor(position_cursor_x, position_cursor_y);
    lcd.print("  ");
    space_or_no = 0;
    Millis_blink = millis();
  }
  if (!space_or_no && millis() - Millis_blink >= time_blink) {
    if (CaseByte < 10) {
      lcd.setCursor(position_cursor_x + 1, position_cursor_y);
      lcd.print(CaseByte);
    } else {
      lcd.setCursor(position_cursor_x, position_cursor_y);
      lcd.print(CaseByte);
    }

    space_or_no = 1;
    Millis_blink = millis();
  }
}

//--------------------------
void Blink(byte Page, byte Case) {
  switch (Page) {
    case 1:
      switch (Case) {
        case 1:
          BlinkString(0, 0, TIME_BLINK, "Blue");
          break;
        case 2:
          BlinkString(0, 5, TIME_BLINK, "Red");
          break;
        case 3:
          BlinkString(0, 9, TIME_BLINK, "InfRed");
          break;
        case 4:
          BlinkString(1, 0, TIME_BLINK, "Program");
          break;
      }
      break;
    case 2:
      switch (Case) {
        case 1:
          BlinkByte(0, 0, TIME_BLINK, PowerCnt);
          break;
        case 2:
          BlinkByte(0, 6, TIME_BLINK, TimeCnt);
          break;
        case 3:
          BlinkByte(0, 11, TIME_BLINK, TimeCntSek);
          break;
        case 4:
          BlinkString(1, 0, TIME_BLINK, "<-");
          break;
        case 5:
          BlinkString(1, 8, TIME_BLINK, "+");
          break;
        case 6:
          BlinkString(1, 14, TIME_BLINK, "->");
          break;
      }
      break;
    case 4:
      switch (Case) {
        case 1:
          BlinkString(0, 0, TIME_BLINK, "Blue");
          break;
        case 2:
          BlinkString(0, 5, TIME_BLINK, "Red");
          break;
        case 3:
          BlinkString(0, 9, TIME_BLINK, "InfRed");
          break;
      }
      break;
    case 5:

      BlinkByte(0, 8, TIME_BLINK, NomerProgram);
      break;
    case 6:
      switch (Case) {
        case 1:
          BlinkString(0, 0, TIME_BLINK, "Start");
          break;
        case 2:
          BlinkString(1, 0, TIME_BLINK, "Edit");
          break;
      }
      break;
    case 7:
      switch (Case) {
        case 1:
          BlinkString(0, 0, TIME_BLINK, "Blue");
          break;
        case 2:
          BlinkString(0, 5, TIME_BLINK, "Red");
          break;
        case 3:
          BlinkString(0, 9, TIME_BLINK, "InfRed");
          break;
      }
      break;
    case 8:
      switch (Case) {
        case 1:
          BlinkByte(0, 0, TIME_BLINK, PowerCnt);
          break;
        case 2:
          BlinkByte(0, 6, TIME_BLINK, TimeCnt);
          break;
        case 3:
          BlinkByte(0, 11, TIME_BLINK, TimeCntSek);
          break;
        case 4:
          BlinkString(1, 0, TIME_BLINK, "<-");
          break;
        case 5:
          BlinkString(1, 8, TIME_BLINK, "+");
          break;
        case 6:
          BlinkString(1, 14, TIME_BLINK, "->");
          break;
      }

      break;
  }
}
// Функция добавления в стек
void Add(Potok **b, Potok **e, Lazer Info) {
  Potok *t = new Potok;

  t -> Laz = Info;
  t -> next = NULL;   // Формирование адресной части
  if (*b == NULL)   // Формирование первого элемента
    *b = *e = t;
  else {        // Добавление элемента в конец
    (*e) -> next = t;
    *e = t;
  }
}
// Функия добавления времени в первый элемент стека
void Time_For_First(Potok *p, unsigned long First_Millis, byte Poz, byte Last) {
  p->Laz.previousMillis = First_Millis;
}
//Функия очистки стека
void Del_All(Potok **p) {
  Potok *t;
  while ( *p != NULL) {
    t = *p;
    *p = (*p) -> next;
    delete t;
  }
}
void Work(Potok *p) {
  Potok *t = p;
  EEPROM.update(START_STECK_ADR + 1, t->Laz.Poz);
  Serial.println(t->Laz.Poz);
  int deltaPoz;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  if (t->Laz.TimeMinTemp < 10) {
    lcd.setCursor(10, 0);
    lcd.print(t->Laz.TimeMinTemp);
  } else {
    lcd.setCursor(9, 0);
    lcd.print(t->Laz.TimeMinTemp);
  }

  lcd.setCursor(11, 0);
  lcd.print("m");
  if (t->Laz.TimeSekTemp < 10) {
    lcd.setCursor(14, 0);
    lcd.print(t->Laz.TimeSekTemp);
  } else {
    lcd.setCursor(13, 0);
    lcd.print(t->Laz.TimeSekTemp);
  }
  lcd.setCursor(15, 0);
  lcd.print("s");
  lcd.setCursor(0, 1);
  lcd.print("Power:");
  lcd.setCursor(9, 1);
  lcd.print(t->Laz.PowerNum);
  lcd.setCursor(11, 1);
  lcd.print("mW");
  while ( t != NULL && StopReal == 0) {
    //Serial.print(StopReal);
    if (flagStop == 1 && t->Laz.flagPoz == 1) {
      StopReal = 1;
      //Serial.print(StopReal);
    }
    if ((millis() - t->Laz.previousMillis) % 1000 < 10) {
      // t->Laz.previousMillis=millis();
      //Serial.print(t->Laz.MillisTime);
      if (t->Laz.TimeSekTemp >= 1) {
        t->Laz.TimeSekTemp--;
        lcd.setCursor(9, 0);
        lcd.print("  ");
        if (t->Laz.TimeMinTemp < 10) {
          lcd.setCursor(10, 0);
          lcd.print(t->Laz.TimeMinTemp);
        } else {
          lcd.setCursor(9, 0);
          lcd.print(t->Laz.TimeMinTemp);
        }
        lcd.setCursor(13, 0);
        lcd.print("  ");
        if (t->Laz.TimeSekTemp < 10) {
          lcd.setCursor(14, 0);
          lcd.print(t->Laz.TimeSekTemp);
        } else {
          lcd.setCursor(13, 0);
          lcd.print(t->Laz.TimeSekTemp);
        }

      } else {
        if (t->Laz.TimeMinTemp > 0) {
          t->Laz.TimeSekTemp = 60;
          t->Laz.TimeMinTemp--;
        }
      }
    }


    t->Laz.flagPoz = smena_poz(t->Laz.Poz, t->Laz.LastPoz, t->Laz.flagPoz);
    if (millis() - t->Laz.previousMillis >= t->Laz.MillisTime) {
      Serial.println(t->Laz.MillisTime);
      analogWrite(t->Laz.PinOut, 0);
      if (t->next != NULL) {
        t->next->Laz.previousMillis = millis();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Time: ");
        if (t->Laz.TimeMinTemp < 10) {
          lcd.setCursor(10, 0);
          lcd.print(t->Laz.TimeMinTemp);
        } else {
          lcd.setCursor(9, 0);
          lcd.print(t->Laz.TimeMinTemp);
        }

        lcd.setCursor(11, 0);
        lcd.print("m");
        if (t->Laz.TimeSekTemp < 10) {
          lcd.setCursor(14, 0);
          lcd.print(t->Laz.TimeSekTemp);
        } else {
          lcd.setCursor(13, 0);
          lcd.print(t->Laz.TimeSekTemp);
        }
        lcd.setCursor(15, 0);
        lcd.print("s");
        lcd.setCursor(0, 1);
        lcd.print("Power:");
        lcd.setCursor(9, 1);
        lcd.print(t->next->Laz.PowerNum);
        lcd.setCursor(11, 1);
        lcd.print("mW");
        t->next->Laz.LastPoz = t->Laz.Poz;
        EEPROM.update(START_STECK_ADR + 1, t->next->Laz.Poz);
        deltaPoz = t->next->Laz.LastPoz - t->next->Laz.Poz;
        deltaPoz = 2 * abs(deltaPoz);
        t->next->Laz.MillisTime = t->next->Laz.MillisTime + deltaPoz * 1750l;
        tone(AudioPin, 500, 500);
      } else {
        EEPROM.update(START_STECK_ADR + 1, 3);
        t->Laz.flagPoz = smena_poz(3, t->Laz.Poz, 0);
      }
      t->Laz.TimeMinTemp = t->Laz.TimeMin;
      t->Laz.TimeSekTemp = t->Laz.TimeSek;
      //t->Laz.MillisTime=t->Laz.TimeMin*60000l+t->Laz.TimeSek*1000l;
      t->Laz.flagPoz = 0;
      t->Laz.LastPoz = 3;
      //EEPROM.update(2,t->Laz.Poz);
      t = t -> next;
    } else {
      analogWrite(t->Laz.PinOut, t->Laz.Power);
    }
  }
  if (t == NULL || flagStop == 1) {
    NomerPage = 1;
    flagStop = 0;
    StopReal = 0;
    tone(AudioPin, 500, 500);
    analogWrite(AnalogPinR, 0);
    analogWrite(AnalogPinB, 0);
    analogWrite(AnalogPinInf, 0);
    LastPozMotor = EEPROM.read(START_STECK_ADR + 1);
    deltaPoz = smena_poz(3, LastPozMotor, 0);
    EEPROM.update(START_STECK_ADR + 1, 3);
    LastPozMotor = 3;
    lcd.clear();
    Page_1();
    NomerPageTemp = 1;
    NomerCase = 1;

  }
}
// чтение
unsigned long EEPROM_ulong_read(byte addr) {
  byte raw[4];
  for (byte i = 0; i < 4; i++)
    raw[i] = EEPROM.read(addr + i);
  unsigned long &num = (unsigned long&)raw;
  return num;
}

// запись
void EEPROM_ulong_write(byte addr, unsigned long num) {
  byte raw[4];
  (unsigned long&)raw = num;
  for (byte i = 0; i < 4; i++)
    EEPROM.write(addr + i, raw[i]);
}

//Шаги мотора
void step1() {
  digitalWrite(pin1, HIGH);
  digitalWrite(pin2, LOW);
  digitalWrite(pin3, LOW);
  digitalWrite(pin4, LOW);
}
void step2() {
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, HIGH);
  digitalWrite(pin3, LOW);
  digitalWrite(pin4, LOW);
}
void step3() {
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, LOW);
  digitalWrite(pin3, HIGH);
  digitalWrite(pin4, LOW);
}
void step4() {
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, LOW);
  digitalWrite(pin3, LOW);
  digitalWrite(pin4, HIGH);
}

void StopMotor() {
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, LOW);
  digitalWrite(pin3, LOW);
  digitalWrite(pin4, LOW);
}
// Функии поворота по и против часовой стрелки с записью шага в память
void clockwise_Steps(int steps) {
  int i_temp = 0;
  i_temp = EEPROM_int_read(STEP_AFTE_STOP_ADR);
  for (int i = 1; i <= steps; i++) {
    step1();
    delay(SPEEDMOTOR);
    step2();
    delay(SPEEDMOTOR);
    step3();
    delay(SPEEDMOTOR);
    step4();
    delay(SPEEDMOTOR);
    i_temp = i_temp + 1;
    Serial.print(i_temp);
    EEPROM_int_write(STEP_AFTE_STOP_ADR, i_temp);
  }
  StopMotor();
}
void counterclockwise_Steps(int steps) {
  for (int i = 1; i <= steps; i++) {
    int i_temp = 0;
    i_temp = EEPROM_int_read(STEP_AFTE_STOP_ADR);
    step4();
    delay(SPEEDMOTOR);
    step3();
    delay(SPEEDMOTOR);
    step2();
    delay(SPEEDMOTOR);
    step1();
    delay(SPEEDMOTOR);
    i_temp = i_temp - 1;
    EEPROM_int_write(STEP_AFTE_STOP_ADR, i_temp);
  }
  StopMotor();
}
// Функция смены позиции мотора
byte smena_poz(byte poz, byte lpoz, byte flagpoz) {

  if (flagpoz == 0) {
    switch (poz) {
      case 1:
        if (lpoz == 2) {
          counterclockwise_Steps(COUNTERCLOCKWISE);
        } else {
          if (lpoz == 3) {
            counterclockwise_Steps(FULLROLL);
          }
        }
        break;
      case 2:
        if (lpoz == 1) {
          clockwise_Steps(COUNTERCLOCKWISE + 100);
          counterclockwise_Steps(100);
        } else {
          if (lpoz == 3) {
            counterclockwise_Steps(CLOCKWISE);
          }
        }
        break;
      case 3:
        if (lpoz == 2) {
          clockwise_Steps(CLOCKWISE);
        } else {
          if (lpoz == 1) {
            clockwise_Steps(FULLROLL);
          }
        }
        break;
    }
    return 1;
  }
  return 1;
}
// Функция прерывания(заменить на ButtomStop)
void Stop() {
  flagStop = 1;
}
// Подпрограмма настройки углов поворота мотора
void NastroikaMotora() {
  byte PageNastroiki = 0;
  byte lpozNAS = 3;
  byte pozNAS = 3;
  byte CountStepNas = 0;
  byte flagpoz = 1;
  lcd.setCursor(1, 0);
  lcd.print("TUNING MOT");
  while (flagNas) {
    if (digitalRead(buttonStop) == HIGH && flagStart == 0) {
      switch (PageNastroiki) {
        case 1:
          break;
        case 2:
          EEPROM.update(CLOCKWISE_ADR, CountStepNas);
          CLOCKWISE = EEPROM.read(CLOCKWISE_ADR);
          FULLROLL = COUNTERCLOCKWISE + CLOCKWISE;
          break;
        case 3:
          EEPROM.update(COUNTERCLOCKWISE_ADR, CountStepNas);
          COUNTERCLOCKWISE = EEPROM.read(COUNTERCLOCKWISE_ADR);
          FULLROLL = COUNTERCLOCKWISE + CLOCKWISE;
          break;
      }
    }

    if (digitalRead(buttonStart) == HIGH && flagStart == 0) {
      PageNastroiki++;
      if (PageNastroiki > 3) {
        PageNastroiki = 1;
      }
      switch (PageNastroiki) {
        case 1:
          CountStepNas = 0;
          analogWrite(AnalogPinR, PowerRed[10]);
          analogWrite(AnalogPinB, 0);
          analogWrite(AnalogPinInf, 0);
          lcd.clear();
          lcd.setCursor(1, 0);
          lcd.print("650");
          lcd.setCursor(1, 1);
          lcd.print("   ");
          lcd.setCursor(1, 1);
          lcd.print(CountStepNas);
          pozNAS = 3;
          break;
        case 2:
          CountStepNas = EEPROM.read(CLOCKWISE_ADR);
          pozNAS = 2;
          analogWrite(AnalogPinB, PowerBlue[10]);
          analogWrite(AnalogPinInf, 0);
          analogWrite(AnalogPinR, 0);
          lcd.clear();
          lcd.setCursor(1, 0);
          lcd.print("405");
          lcd.setCursor(1, 1);
          lcd.print("   ");
          lcd.setCursor(1, 1);
          lcd.print(CountStepNas);
          break;
        case 3:
          CountStepNas = EEPROM.read(COUNTERCLOCKWISE_ADR);
          analogWrite(AnalogPinInf, PowerInf[10]);
          analogWrite(AnalogPinB, 0);
          analogWrite(AnalogPinR, 0);
          lcd.clear();
          lcd.setCursor(1, 0);
          lcd.print("780");
          lcd.setCursor(1, 1);
          lcd.print("   ");
          lcd.setCursor(1, 1);
          lcd.print(CountStepNas);
          pozNAS = 1;
          break;
      }
      flagStart = !flagStart;
    }//Кнопка старт
    if (digitalRead(buttonStart) == LOW && flagStart == 1) {

      flagStart = 0;

    }

    if (digitalRead(buttonOk) == HIGH && flagOk == 0) {
      flagOk = 1;
      flagpoz = 0;
    }
    flagpoz = smena_poz(pozNAS, lpozNAS, flagpoz);
    if (digitalRead(buttonOk) == LOW && flagOk == 1) {
      flagOk = 0;
      lpozNAS = pozNAS;
    }
    //---------------
    if (millis() - Millis_Check_Code >= 1) {
      Code = encoderGetCode();
      Millis_Check_Code = millis();
    }
    if (millis() - Millis_Get_Code >= 1) {
      if (Code == 1 || Code == 7 || Code == 8 || Code == 14) {
        if (Plavno > -4) {
          Plavno--;
        }
        else {
          Plavno = 0;
          switch (PageNastroiki) {
            case 1:
              step4();
              delay(SPEEDMOTOR);
              step3();
              delay(SPEEDMOTOR);
              step2();
              delay(SPEEDMOTOR);
              step1();
              delay(SPEEDMOTOR);
              CountStepNas--;
              lcd.setCursor(1, 1);
              lcd.print("   ");
              lcd.setCursor(1, 1);
              lcd.print(CountStepNas);
              break;
            case 2:
              step4();
              delay(SPEEDMOTOR);
              step3();
              delay(SPEEDMOTOR);
              step2();
              delay(SPEEDMOTOR);
              step1();
              delay(SPEEDMOTOR);
              CountStepNas++;
              lcd.setCursor(1, 1);
              lcd.print("   ");
              lcd.setCursor(1, 1);
              lcd.print(CountStepNas);
              break;
            case 3:
              step4();
              delay(SPEEDMOTOR);
              step3();
              delay(SPEEDMOTOR);
              step2();
              delay(SPEEDMOTOR);
              step1();
              delay(SPEEDMOTOR);
              CountStepNas++;
              lcd.setCursor(1, 1);
              lcd.print("   ");
              lcd.setCursor(1, 1);
              lcd.print(CountStepNas);
              break;
          }
        }


      } else if (Code == 2 || Code == 4 || Code == 11 || Code == 13) {
        if (Plavno < 4) {
          Plavno++;
        }
        else {
          Plavno = 0;
          switch (PageNastroiki) {
            case 1:
              step1();
              delay(SPEEDMOTOR);
              step2();
              delay(SPEEDMOTOR);
              step3();
              delay(SPEEDMOTOR);
              step4();
              delay(SPEEDMOTOR);
              CountStepNas++;
              lcd.setCursor(1, 1);
              lcd.print("   ");
              lcd.setCursor(1, 1);
              lcd.print(CountStepNas);
              break;
            case 2:
              step1();
              delay(SPEEDMOTOR);
              step2();
              delay(SPEEDMOTOR);
              step3();
              delay(SPEEDMOTOR);
              step4();
              delay(SPEEDMOTOR);
              CountStepNas--;
              lcd.setCursor(1, 1);
              lcd.print("   ");
              lcd.setCursor(1, 1);
              lcd.print(CountStepNas);
              break;
            case 3:
              step1();
              delay(SPEEDMOTOR);
              step2();
              delay(SPEEDMOTOR);
              step3();
              delay(SPEEDMOTOR);
              step4();
              delay(SPEEDMOTOR);
              CountStepNas--;
              lcd.setCursor(1, 1);
              lcd.print("   ");
              lcd.setCursor(1, 1);
              lcd.print(CountStepNas);
              break;
          }
        }

      }
      Millis_Get_Code = millis();
    }

    //---------------
  }
}

// Простая подпрограмма для выставления центра
void NastroikaMotoraMED() {
  int CountStepNas = 0;
  lcd.setCursor(1, 1);
  lcd.print("TUNING MED");

  while (flagNasMED) {
    if (digitalRead(buttonOk) == HIGH && flagOk == 0) {
      flagOk = 1;
      lcd.clear();
    }
    if (digitalRead(buttonOk) == LOW && flagOk == 1) {
      flagOk = 0;
      lcd.setCursor(5, 0);
      lcd.print("Centre");
      lcd.setCursor(1, 1);
      lcd.print(CountStepNas);
    }
    //---------------------
    if (millis() - Millis_Check_Code >= 1) {
      Code = encoderGetCode();
      Millis_Check_Code = millis();
    }
    if (millis() - Millis_Get_Code >= 1) {
      if (Code == 1 || Code == 7 || Code == 8 || Code == 14) {
        if (Plavno > -4) {
          Plavno--;
        }
        else {
          Plavno = 0;
          step4();
          delay(SPEEDMOTOR);
          step3();
          delay(SPEEDMOTOR);
          step2();
          delay(SPEEDMOTOR);
          step1();
          delay(SPEEDMOTOR);
          CountStepNas--;
          lcd.setCursor(1, 1);
          lcd.print("   ");
          lcd.setCursor(1, 1);
          lcd.print(CountStepNas);
        }


      } else if (Code == 2 || Code == 4 || Code == 11 || Code == 13) {
        if (Plavno < 4) {
          Plavno++;
        }
        else {
          Plavno = 0;
          step1();
          delay(SPEEDMOTOR);
          step2();
          delay(SPEEDMOTOR);
          step3();
          delay(SPEEDMOTOR);
          step4();
          delay(SPEEDMOTOR);
          CountStepNas++;
          lcd.setCursor(1, 1);
          lcd.print("   ");
          lcd.setCursor(1, 1);
          lcd.print(CountStepNas);
        }

      }
      Millis_Get_Code = millis();
    }

    //-----------------------

  }

}
// Подпрограмма прогона аппарата
void Progon(int n) {
  if (flagProgon) {
    lcd.setCursor(1, 0);
    lcd.print("PROGONKA  ");
    lcd.print(n);
  }
  boolean flagStartProgon = 0;
  boolean flagControl = 0;
  boolean Povorot = 0;
  int i = 0;
  byte PozProgon = 3;
  byte LastPozProgon = 3;

  while (flagProgon) {
    if (digitalRead(buttonOk) == HIGH && flagOk == 0) {
      flagOk = 1;
      lcd.setCursor(1, 1);
      lcd.print("Start");
      flagControl = 1;
    }

    if (digitalRead(buttonOk) == LOW && flagOk == 1) {
      flagOk = 0;
    }

    if (digitalRead(buttonStart) == HIGH && flagStart == 0) {
      flagStart = 1;
      if ( flagControl == 1) {
        flagStartProgon = !flagStartProgon;
      }
    }

    if (digitalRead(buttonStart) == LOW && flagStart == 1) {
      flagStart = 0;
    }

    if ( flagStop == 1) {
      flagStop = 0;
      i = n;
      PozProgon = 3;
      Povorot = smena_poz(3, LastPozProgon, Povorot);
      LastPozProgon = 3;
      lcd.setCursor(1, 1);
      lcd.print(" STOP");
      tone(AudioPin, 500, 2000);
    }


    if (flagStartProgon && i < n) {
      PozProgon = random(1, 4);
      lcd.setCursor(8, 1);
      lcd.print(PozProgon);
      Povorot = smena_poz(PozProgon, LastPozProgon, Povorot);
      delay(1000);
      Povorot = 0;
      LastPozProgon = PozProgon;
      i++;
      lcd.setCursor(11, 1);
      lcd.print(i);
    }
    if (i >= n) {
      Povorot = smena_poz(3, LastPozProgon, 0);
      LastPozProgon = 3;
      tone(AudioPin, 500, 2000);
    }
  }
}

// чтение из памяти инт
int EEPROM_int_read(int addr) {
  byte raw[2];
  for (byte i = 0; i < 2; i++) raw[i] = EEPROM.read(addr + i);
  int &num = (int&)raw;
  return num;
}

// запись в память инт
void EEPROM_int_write(int addr, int num) {
  byte raw[2];
  (int&)raw = num;
  for (byte i = 0; i < 2; i++) EEPROM.write(addr + i, raw[i]);
}
void SavePrograma(byte Nomer) {
  int AdressProgram;
  switch (Nomer) {
    case 1:
      AdressProgram = 5;
      break;
    case 2:
      AdressProgram = 69;
      break;
    case 3:
      AdressProgram = 133;
      break;
    case 4:
      AdressProgram = 197;
      break;
    case 5:
      AdressProgram = 261;
      break;
  }
  EEPROM.update(AdressProgram, Sum);
  EEPROM.update(SUM_ADR, Sum);
  AdressProgram++;
  EEPROM.update(START_STECK_ADR + adres, Bufer.Poz);
  EEPROM.update(AdressProgram + adres, Bufer.Poz);

  EEPROM.update(START_STECK_ADR + 1 + adres, 3);
  EEPROM.update(AdressProgram + 1 + adres, 3);

  EEPROM.update(START_STECK_ADR + 2 + adres, 0);
  EEPROM.update(AdressProgram + 2 + adres, 0);

  EEPROM.update(START_STECK_ADR + 3 + adres, Bufer.Power);
  EEPROM.update(AdressProgram + 3 + adres, Bufer.Power);

  EEPROM.update(START_STECK_ADR + 4 + adres, Bufer.PowerNum);
  EEPROM.update(AdressProgram + 4 + adres, Bufer.PowerNum);

  EEPROM.update(START_STECK_ADR + 5 + adres, Bufer.TimeMin);
  EEPROM.update(AdressProgram + 5 + adres, Bufer.TimeMin);

  EEPROM.update(START_STECK_ADR + 6 + adres, Bufer.TimeMin);
  EEPROM.update(AdressProgram + 6 + adres, Bufer.TimeMin);

  EEPROM.update(START_STECK_ADR + 7 + adres, Bufer.TimeSek);
  EEPROM.update(AdressProgram + 7 + adres, Bufer.TimeSek);

  EEPROM.update(START_STECK_ADR + 8 + adres, Bufer.TimeSek);
  EEPROM.update(AdressProgram + 8 + adres, Bufer.TimeSek);

  EEPROM.update(START_STECK_ADR + 9 + adres, Bufer.PinOut);
  EEPROM.update(AdressProgram + 9 + adres, Bufer.PinOut);

  EEPROM_ulong_write(START_STECK_ADR + 10 + adres, Bufer.previousMillis);
  EEPROM_ulong_write(AdressProgram + 10 + adres, Bufer.previousMillis);

  EEPROM_ulong_write(START_STECK_ADR + 14 + adres, Bufer.MillisTime);
  EEPROM_ulong_write(AdressProgram + 14 + adres, Bufer.MillisTime);

  adres = adres + 21;
  Add(&begin, &end, Bufer);
}
void ReadProgram(byte Nomer) {
  int AdressProgram;
  switch (Nomer) {
    case 1:
      AdressProgram = 5;
      break;
    case 2:
      AdressProgram = 69;
      break;
    case 3:
      AdressProgram = 133;
      break;
    case 4:
      AdressProgram = 197;
      break;
    case 5:
      AdressProgram = 261;
      break;
  }
  Sum = EEPROM.read(AdressProgram);
  EEPROM.update(SUM_ADR, Sum);
  AdressProgram++;
  for ( byte i = 0; i < Sum * 21; i = i + 21) {
    Bufer.Poz = EEPROM.read(AdressProgram + i);
    EEPROM.update(START_STECK_ADR + i, Bufer.Poz);

    Bufer.LastPoz = EEPROM.read(AdressProgram + 1 + i);
    EEPROM.update(START_STECK_ADR + 1 + i, Bufer.LastPoz);

    Bufer.flagPoz = EEPROM.read(AdressProgram + 2 + i);
    EEPROM.update(START_STECK_ADR + 2 + i, Bufer.flagPoz);

    Bufer.Power = EEPROM.read(AdressProgram + 3 + i);
    EEPROM.update(START_STECK_ADR + 3 + i, Bufer.Power);

    Bufer.PowerNum = EEPROM.read(AdressProgram + 4 + i);
    EEPROM.update(START_STECK_ADR + 4 + i, Bufer.PowerNum);

    Bufer.TimeMin = EEPROM.read(AdressProgram + 5 + i);
    EEPROM.update(START_STECK_ADR + 5 + i, Bufer.TimeMin);

    Bufer.TimeMinTemp = EEPROM.read(AdressProgram + 6 + i);
    EEPROM.update(START_STECK_ADR + 6 + i, Bufer.TimeMinTemp);

    Bufer.TimeSek = EEPROM.read(AdressProgram + 7 + i);
    EEPROM.update(START_STECK_ADR + 7 + i, Bufer.TimeSek);

    Bufer.TimeSekTemp = EEPROM.read(AdressProgram + 8 + i);
    EEPROM.update(START_STECK_ADR + 8 + i, Bufer.TimeSekTemp);

    Bufer.PinOut = EEPROM.read(AdressProgram + 9 + i);
    EEPROM.update(START_STECK_ADR + 9 + i, Bufer.PinOut);

    Bufer.previousMillis = EEPROM_ulong_read(AdressProgram + 10 + i);
    EEPROM_ulong_write(START_STECK_ADR + 10 + i, Bufer.previousMillis);

    Bufer.MillisTime = EEPROM_ulong_read(AdressProgram + 14 + i);
    EEPROM_ulong_write(START_STECK_ADR + 14 + i, Bufer.MillisTime);

    Add(&begin, &end, Bufer);
  }

}
