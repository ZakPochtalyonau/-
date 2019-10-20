#include <EEPROM.h>
#include <Wire.h> // библиотека для управления устройствами по I2C 
#include <LiquidCrystal_I2C.h> // подключаем библиотеку для LCD 1602

LiquidCrystal_I2C lcd(0x27 , 16, 2);// или 0x3F или 0x27

#define START 6
#define PAUSE 4
#define PLUS  12
#define MINUS 9
#define MOSCH 3
#define AUDIO 5
#define DIOD 10
#define VINT 12
#define TOK  1
#define TEMPER 0
#define PlaceTime 11
#define PlacePower 12

byte key=0;
byte TempVAR;

int Vol_D=0;
int Vol_I_D=0;
float Vol_A=Vol_D*0.0048;
float Vol_I_A=Vol_I_D*0.0048;
float Tem=(Vol_A-0.6)*100;
float Tok=Vol_I_A*220;
  

byte PowerProcent=10;
byte PWM=0;
byte PWM_ARRAY[]={0,0,0,0,0,0,0,0,0,0};
byte TimeHour=24;
byte TimerTempHour=24;
byte TimerTempMIN=0;
//byte TimerTempSEK=0;
byte SaveTimeHour;
byte SaveTimeMIN; 

boolean flagButtomSTART=0;
boolean flagButtomPAUSE=0;
boolean flagButtomPLUS=0;
boolean flagButtomMINUS=0;
boolean flagButtomMOSCH=0;

boolean Power10or100=0; 
boolean flagPower_AND_Time=0;

boolean flagALARM=0;
boolean TIMER_RUN=0;
boolean flagTEST=0;
boolean flagPAUSE=0;

unsigned long  TimeWorkMillis;
unsigned long  TimerMillis;
unsigned long  MillisTEMPER;


void PowerIR(unsigned char power){                            // Set power laser IR

  switch(power)
  {
   case 0:  {PWM = 0;break;}                                        //    0 %
   case 1:  {PWM = PWM_ARRAY[0];break;}                             //   10 %
   case 2:  {PWM = PWM_ARRAY[1];break;}                             //   20 %
   case 3:  {PWM = PWM_ARRAY[2];break;}                             //   30 %
   case 4:  {PWM = PWM_ARRAY[3];break;}                             //   40 %
   case 5:  {PWM = PWM_ARRAY[4];break;}                             //   50 %
   case 6:  {PWM = PWM_ARRAY[5];break;}                             //   60 %
   case 7:  {PWM = PWM_ARRAY[6];break;}                             //   70 %
   case 8:  {PWM = PWM_ARRAY[7];break;}                             //   80 %
   case 9:  {PWM = PWM_ARRAY[8];break;}                             //   90 %
   case 10: {PWM = PWM_ARRAY[9];break;}                             //  100 %
   default: {PWM = 0;break;}                                        //    0 %
  }
 
}  

void TuningPWM(byte PWM1,byte PWM10){
  byte k;
  byte b;
  TempVAR=(PWM10-PWM1)%9;
   // Serial.println(TempVAR);
  k=(PWM10-PWM1-TempVAR)/9;
 // Serial.println(k);
  b=PWM1-k;
  //if(TempVAR>0)(b=b+TempVAR);
  Serial.println(b);
  Serial.println("-----------");
  for(byte i=1;i<11;i++){
//    PWM_ARRAY[i-1]=i*k;
    PWM_ARRAY[i-1]=i*k+b;
    EEPROM.update(i-1,PWM_ARRAY[i-1]);
    Serial.println(PWM_ARRAY[i-1]);
    } 
    PWM_ARRAY[9]=PWM10;
    EEPROM.update(9,PWM_ARRAY[9]);
  }
void Timer(){
  if(TIMER_RUN&&!flagPAUSE&&!flagALARM){
    if(millis()-TimerMillis>=60000){
      if(TimerTempMIN>0){
        TimerTempMIN--;
        if(TimerTempMIN==0&&TimerTempHour==0){
           Serial.print("END");
              TimerTempHour=TimeHour;
              Serial.print(TimeHour);
              TimerTempMIN=0;
          if(TimerTempHour<10){
              lcd.setCursor(9,0);
            lcd.print("  "); 
            lcd.setCursor(9,0);
            lcd.print("0"); 
            lcd.print(TimerTempHour);
              }else{
            lcd.setCursor(9,0);
            lcd.print("  "); 
            lcd.setCursor(9,0);
            lcd.print(TimerTempHour);
              }
            lcd.setCursor(12,0);
            lcd.print(TimerTempMIN);
            lcd.setCursor(13,0);
            lcd.print("0");     
            TIMER_RUN=0;
            analogWrite(DIOD,0);
            tone(AUDIO, 500, 1000);
          }
        Serial.print(TimerTempHour);
        Serial.print(":");
        Serial.print(TimerTempMIN);
        Serial.println(" "); 
//        PrintCharPROBEL(1 ,0,0,1);
//            PrintCharPROBEL(1 ,18,0,1);
//            PrintNumI(TimerTempHour,0,0,1);
           if(TimerTempMIN<10){lcd.setCursor(12,0);
            lcd.print("  "); 
            lcd.setCursor(12,0);
            lcd.print("0"); 
            lcd.print(TimerTempMIN);
            }else{
            lcd.setCursor(12,0);
            lcd.print("  "); 
            lcd.setCursor(12,0);
            lcd.print(TimerTempMIN);
            }
        TimerMillis=millis();  
        }else{
          if(TimerTempHour>0){
            TimerTempHour--;
            TimerTempMIN=59;
            Serial.print(TimerTempHour);
            Serial.print(":");
            Serial.print(TimerTempMIN);
            Serial.println(" "); 
//            PrintCharPROBEL(1 ,0,0,1);
//            PrintCharPROBEL(1 ,18,0,1);
             if(TimerTempHour<10){
              lcd.setCursor(9,0);
            lcd.print("  "); 
            lcd.setCursor(9,0);
            lcd.print("0"); 
            lcd.print(TimerTempHour);
              }else{
            lcd.setCursor(9,0);
            lcd.print("  "); 
            lcd.setCursor(9,0);
            lcd.print(TimerTempHour);
              }
//            PrintNumI(TimerTempHour,0,0,1);
//            PrintCharPROBEL(1 ,53,0,1);
//            PrintCharPROBEL(1 ,71,0,1);
//            PrintNumI(TimerTempMIN,53,0,1);  
            lcd.setCursor(12,0);
            lcd.print("  "); 
            lcd.setCursor(12,0);
            lcd.print(TimerTempMIN);
            if( TimerTempHour!=TimeHour-1){ tone(AUDIO, 200, 100);}
             TimerMillis=millis();  
            }else{
              Serial.print("END");
              TimerTempHour=TimeHour;
              Serial.print(TimeHour);
              TimerTempMIN=0;
          if(TimerTempHour<10){
              lcd.setCursor(9,0);
            lcd.print("  "); 
            lcd.setCursor(9,0);
            lcd.print("0"); 
            lcd.print(TimerTempHour);
              }else{
            lcd.setCursor(9,0);
            lcd.print("  "); 
            lcd.setCursor(9,0);
            lcd.print(TimerTempHour);
              }
            lcd.setCursor(12,0);
            lcd.print(TimerTempMIN);
            lcd.setCursor(13,0);
            lcd.print("0");     
            TIMER_RUN=0;
            analogWrite(DIOD,0);
            tone(AUDIO, 500, 1000);
              }
         }
      }
    }
  //if(flagPAUSE){}
  }

void ButtomPlus(){
   if(digitalRead(PLUS)== HIGH&&!flagALARM&&!TIMER_RUN&&!flagPAUSE&&flagButtomPLUS==0){
    flagButtomPLUS=1;
    tone(AUDIO, 200, 100);
    }
  if(digitalRead(PLUS)== LOW&&!flagALARM&&!TIMER_RUN&&!flagPAUSE&&flagButtomPLUS){
     flagButtomPLUS=0;
     if(flagPower_AND_Time){
      if(PowerProcent<10){
        PowerProcent++;
        }else{
          PowerProcent=10;
          }
      PowerIR(PowerProcent);
      Serial.println("Power:");
//      PrintCharPROBEL(1 ,90,0,2);
//      PrintCharPROBEL(1 ,100,0,2);
//      PrintCharPROBEL( 1,105,0,2);
//      PrintNumI(PowerProcent*10,90,0,2);
      lcd.setCursor(9,1);
      lcd.print("  ");
      lcd.setCursor(9,1);
      lcd.print(PowerProcent*10);
      Serial.println(PowerProcent*10);
      Serial.println(PWM);
      }else{
        if(TimeHour<24){
          TimeHour++;
          TimerTempHour=TimeHour;
          }else{
            TimeHour=24;
            }
//            PrintCharPROBEL(1 ,0,0,1);
//            PrintCharPROBEL(1 ,18,0,1);
//            PrintNumI(TimeHour,0,0,1);
              if(TimerTempHour<10){
              lcd.setCursor(9,0);
            lcd.print("  "); 
            lcd.setCursor(9,0);
            lcd.print("0"); 
            lcd.print(TimerTempHour);
              }else{
            lcd.setCursor(9,0);
            lcd.print("  "); 
            lcd.setCursor(9,0);
            lcd.print(TimerTempHour);
              }
            Serial.println("Time:");
            Serial.println(TimeHour);
        }
        }
  }
//------------------------------------------------------------  
void ButtomMinus(){
   if(digitalRead(MINUS)== HIGH&&!flagALARM&&!TIMER_RUN&&!flagPAUSE&&flagButtomMINUS==0){
    flagButtomMINUS=1; 
    tone(AUDIO, 200, 100);
    }
  if(digitalRead(MINUS)== LOW&&!flagALARM&&!TIMER_RUN&&!flagPAUSE&&flagButtomMINUS){
   flagButtomMINUS=0;
   if(flagPower_AND_Time){
    if(PowerProcent>0){
        PowerProcent--;
        }else{
          PowerProcent=0;
          }
      PowerIR(PowerProcent);
//      PrintCharPROBEL(1 ,90,0,2);
//      PrintCharPROBEL(1,100,0,2);
//      PrintCharPROBEL(1,105,0,2);
//      PrintNumI(PowerProcent*10,90,0,2);
      lcd.setCursor(9,1);
      lcd.print("   ");
      lcd.setCursor(9,1);
      lcd.print(PowerProcent*10);
     
      Serial.println(PWM);
    }else{
      if(TimeHour>1){
          TimeHour--;
          TimerTempHour=TimeHour;
          }else{
            TimeHour=1;
            }
//            PrintCharPROBEL(1 ,0,0,1);
//            PrintCharPROBEL(1 ,18,0,1);
//            PrintNumI(TimeHour,0,0,1);
               if(TimerTempHour<10){
              lcd.setCursor(9,0);
            lcd.print("  "); 
            lcd.setCursor(9,0);
            lcd.print("0"); 
            lcd.print(TimerTempHour);
              }else{
            lcd.setCursor(9,0);
            lcd.print("  "); 
            lcd.setCursor(9,0);
            lcd.print(TimerTempHour);
              }
            Serial.println("Time:");
            Serial.println(TimeHour);
      }
      }
  
  }
//--------------------------------------------------------
void ButtomPause(){
    
  if(digitalRead(PAUSE)== HIGH&&!flagALARM&&TIMER_RUN&&!flagButtomPAUSE){flagButtomPAUSE=1;
  tone(AUDIO, 200, 100);}
  if(digitalRead(PAUSE)== LOW&&!flagALARM&&TIMER_RUN&&flagButtomPAUSE){
      flagButtomPAUSE=0;
      flagPAUSE=!flagPAUSE;
      //Serial.print(flagPAUSE);
      if(flagPAUSE){analogWrite(DIOD,0);}
     
      if(flagPAUSE==0){TIMER_RUN=0;
      analogWrite(DIOD,0);
      Serial.println(TimeHour);
         Serial.println(flagPAUSE);
         TimerTempHour=TimeHour;
         TimerTempMIN=0;
           if(TimerTempHour<10){
              lcd.setCursor(9,0);
            lcd.print("  "); 
            lcd.setCursor(9,0);
            lcd.print("0"); 
            lcd.print(TimerTempHour);
              }else{
            lcd.setCursor(9,0);
            lcd.print("  "); 
            lcd.setCursor(9,0);
            lcd.print(TimerTempHour);}
            lcd.setCursor(12,0);
       lcd.print("00");
        
      }
      }
  }
//------------------  
void ButtomStart(){
  if(digitalRead(START)== HIGH&&!flagALARM&&flagButtomSTART==0){
    flagButtomSTART=1;
    EEPROM.update(PlaceTime,TimeHour);
     EEPROM.update(PlacePower,PowerProcent);
    tone(AUDIO, 200, 100);
    }
  if(digitalRead(START)== LOW&&!flagALARM&&flagButtomSTART==1){
      flagButtomSTART=0;
      if(flagPAUSE){
      TIMER_RUN=1;
      analogWrite(DIOD,PWM);
      flagPAUSE=0; 
      }
      else{
        TIMER_RUN=!TIMER_RUN;
        
       if(TIMER_RUN){
      analogWrite(DIOD,PWM);
      TimerTempHour=TimeHour-1;
      TimerTempMIN=59;
         if(TimerTempHour<10){
              lcd.setCursor(9,0);
            lcd.print("  "); 
            lcd.setCursor(9,0);
            lcd.print("0"); 
            lcd.print(TimerTempHour);
              }else{
            lcd.setCursor(9,0);
            lcd.print("  "); 
            lcd.setCursor(9,0);
            lcd.print(TimerTempHour);
              }
      lcd.print(":");
      lcd.setCursor(12,0);
      lcd.print(TimerTempMIN);
      //tone(AUDIO, 300, 500);
      TimerMillis=millis();
      }else{
         analogWrite(DIOD,0);
         Serial.println(TimeHour);
         Serial.println(flagPAUSE);
         TimerTempHour=TimeHour;
         TimerTempMIN=0;
        if(TimerTempHour<10){
              lcd.setCursor(9,0);
            lcd.print("  "); 
            lcd.setCursor(9,0);
            lcd.print("0"); 
            lcd.print(TimerTempHour);
              }else{
            lcd.setCursor(9,0);
            lcd.print("  "); 
            lcd.setCursor(9,0);
            lcd.print(TimerTempHour);
              }
      lcd.print(":");
         lcd.setCursor(12,0);
         lcd.print(TimerTempMIN);
         lcd.setCursor(13,0);
         lcd.print("0");
        }
     
      }
     
    
      
    }
  }
//--------------------------------------------------

void ButtomMosh(){
  if(digitalRead(MOSCH)== HIGH&&!flagALARM&&!TIMER_RUN&&!flagPAUSE&&flagButtomMOSCH==0){
   flagButtomMOSCH=1;
   tone(AUDIO, 200, 100);}
  if(digitalRead(MOSCH)== LOW&&!flagALARM&&!TIMER_RUN&&!flagPAUSE&&flagButtomMOSCH){
    flagButtomMOSCH=0;
    lcd.setCursor(0,flagPower_AND_Time);
    lcd.print(" ");
    flagPower_AND_Time=!flagPower_AND_Time;
    lcd.setCursor(0,flagPower_AND_Time);
    lcd.print("*");
    if(flagPower_AND_Time){
      Serial.println("POWER:");
      }else{
        Serial.println("TIME:");
        }
    }
  
  }
//----------------------------------------------------------------------------
void  ALARM(){
  if((millis()-MillisTEMPER)>=500){
       Vol_D=analogRead(TEMPER);
      // Vol_I_D=analogRead(TOK);
       Vol_A=Vol_D*0.0048;
       Tem=(Vol_A-0.6)*100;
       if(Tem>=90){
         flagALARM=1;
         TIMER_RUN=0;
         analogWrite(DIOD,0);
         lcd.clear();
         lcd.setCursor(1,0);
         lcd.print("TEMPER MAX");
         lcd.setCursor(1,1);
         lcd.print("PEREGREV"); 
         tone(AUDIO, 1000, 400); 
        }
       MillisTEMPER=millis();
  }
  
  }
//--------------------------------------------------------------------
  
void setup() {
  pinMode(START,INPUT);
  pinMode(PAUSE,INPUT);
  pinMode(PLUS,INPUT);
  pinMode(MINUS,INPUT);
  pinMode(MOSCH,INPUT);
  pinMode(DIOD,OUTPUT);
  analogWrite(DIOD,0);
  Serial.begin (9600);
  TimeHour=EEPROM.read(PlaceTime);
  PowerProcent=EEPROM.read(PlacePower);
  if(PowerProcent>10){
    PowerProcent=10;
    }
  for ( byte i = 0; i < 10; i ++) {
          PWM_ARRAY[i]= EEPROM.read(i);
    }

  PowerIR(PowerProcent);
  if(TimeHour>24){
    TimeHour=24;
    EEPROM.update(PlaceTime,TimeHour);
    }
  TimerTempHour=TimeHour;
  lcd.begin(); // Инициализация lcd
  lcd.backlight(); // Включаем подсветку
  lcd.clear();
  lcd.setCursor(1,1);
  lcd.print("POWER:");
  lcd.setCursor(9,1);
  lcd.print(PowerProcent*10);
  lcd.setCursor(13,1);
  lcd.print("%");
  lcd.setCursor(0,0);
  lcd.print("*");
  lcd.setCursor(1,0);
  lcd.print("TIME:");
   if(TimerTempHour<10){
              lcd.setCursor(9,0);
            lcd.print("  "); 
            lcd.setCursor(9,0);
            lcd.print("0"); 
            lcd.print(TimerTempHour);
              }else{
            lcd.setCursor(9,0);
            lcd.print("  "); 
            lcd.setCursor(9,0);
            lcd.print(TimerTempHour);
            }
  lcd.print(":");
  lcd.setCursor(12,0);
  lcd.print(TimerTempMIN);
  lcd.setCursor(13,0);
  lcd.print("0");
  
  analogReference(DEFAULT);
  Vol_D=analogRead(TEMPER);
  Vol_I_D=analogRead(TOK);
  Vol_A=Vol_D*0.0048;
  Vol_I_A=Vol_I_D*0.0048;
  Tem=(Vol_A-0.6)*100;
  Tok=Vol_I_A*220;
  //Режим теста
  // TuningPWM(112,255);
  //Нажатые кнопки
  if(digitalRead(MOSCH)){
    flagTEST=1;
     MillisTEMPER=millis();
    Serial.println("TEST"); 
//    Serial.println(TEMPER);
    Serial.println("POWER 10% =  ");
    Serial.print(PWM_ARRAY[0]);
    Serial.println("POWER 100% =  ");
    PWM=PWM_ARRAY[9];
    Serial.print(PWM_ARRAY[9]);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("*");
    lcd.setCursor(1,0);
    lcd.print("10%");
    lcd.setCursor(7,0);
    lcd.print(PWM_ARRAY[0]);
    lcd.setCursor(1,1);
    lcd.print("100%");
    lcd.setCursor(7,1);
    lcd.print(PWM);
    
    }
    
  while(flagTEST){  //
    if((millis()-MillisTEMPER)>=500){
       Vol_D=analogRead(TEMPER);
      // Vol_I_D=analogRead(TOK);
       Serial.print("VOL_I_D ");
       Serial.println(Vol_I_D);
       Vol_A=Vol_D*0.0048;
       Tem=(Vol_A-0.6)*100;
       lcd.setCursor(12,1);
       lcd.print(Tem);
       MillisTEMPER=millis();
  }
   
    if(digitalRead(PLUS)== HIGH && flagButtomPLUS==0){flagButtomPLUS=1;}
    if(digitalRead(PLUS) == LOW && flagButtomPLUS==1){
      key=1;
      flagButtomPLUS=0;
      PWM++;
         if(Power10or100){
          Serial.print("POWER 10% =  ");
           PWM_ARRAY[0]=PWM;
           lcd.setCursor(7,0);
           lcd.print("   ");
           lcd.setCursor(7,0);
           lcd.print(PWM_ARRAY[0]);
          }else{
            Serial.println("POWER 100% =  ");
            PWM_ARRAY[9]=PWM;
            lcd.setCursor(7,1);
            lcd.print("   ");
            lcd.setCursor(7,1);
            lcd.print(PWM_ARRAY[9]);
          }
        //Serial.print("BBBBBBBBBBBB");
        Serial.print(PWM);
        }
    
    if(digitalRead(MINUS)== HIGH && flagButtomMINUS==0){flagButtomMINUS=1;}
    if(digitalRead(MINUS) == LOW && flagButtomMINUS==1){
      flagButtomMINUS=0;
      PWM--;
         if(Power10or100){
          Serial.print("POWER 10% =  ");
           PWM_ARRAY[0]=PWM;
           lcd.setCursor(7,0);
           lcd.print("   ");
           lcd.setCursor(7,0);
           lcd.print(PWM_ARRAY[0]);
          }else{
            Serial.println("POWER 100% =  ");
            PWM_ARRAY[9]=PWM;
            lcd.setCursor(7,1);
            lcd.print("   ");
            lcd.setCursor(7,1);
            lcd.print(PWM_ARRAY[9]);
          }
       //Serial.print("BBBBBBBBBBBB");
        Serial.print(PWM);
        }
    if(digitalRead(START)== HIGH && flagButtomSTART==0){flagButtomSTART=1;}
    if(digitalRead(START) == LOW && flagButtomSTART==1){
      flagButtomSTART=0;
      TIMER_RUN=!TIMER_RUN;
    }
    if(TIMER_RUN){analogWrite(DIOD,PWM);}
        else{analogWrite(DIOD,0);}   
    if(digitalRead(MOSCH)== HIGH && flagButtomMOSCH==0){flagButtomMOSCH=1;}
    if(digitalRead(MOSCH) == LOW && flagButtomMOSCH==1){
      flagButtomMOSCH=0;
    //  Serial.println("PAAAAAAAAA");
      if(Power10or100==0){
          //Power 10%
            Power10or100=1;
            PWM=PWM_ARRAY[0];
             lcd.setCursor(0,1);
            lcd.print(" ");
            lcd.setCursor(0,0);
            lcd.print("*");
          }else{
            //Power 100%
            Power10or100=0;
            PWM=PWM_ARRAY[9];
            lcd.setCursor(0,0);
            lcd.print(" ");
            lcd.setCursor(0,1);
            lcd.print("*");
          }
    }
    if(digitalRead(PAUSE)== HIGH && flagButtomPAUSE==0){flagButtomPAUSE=1;}
    if(digitalRead(PAUSE) == LOW && flagButtomPAUSE==1){
      flagButtomPAUSE=0;
      TuningPWM(PWM_ARRAY[0],PWM_ARRAY[9]);
      flagTEST=0;
      TIMER_RUN=0;
      PowerIR(PowerProcent);
      lcd.clear();
      lcd.setCursor(1,1);
      lcd.print("POWER:");
      lcd.setCursor(9,1);
      lcd.print(PowerProcent*10);
      lcd.setCursor(13,1);
      lcd.print("%");
      lcd.setCursor(0,0);
      lcd.print("*");
      lcd.setCursor(1,0);
      lcd.print("TIME:");
       if(TimerTempHour<10){
              lcd.setCursor(9,0);
            lcd.print("  "); 
            lcd.setCursor(9,0);
            lcd.print("0"); 
            lcd.print(TimerTempHour);
              }else{
            lcd.setCursor(9,0);
            lcd.print("  "); 
            lcd.setCursor(9,0);
            lcd.print(TimerTempHour);
            }
      lcd.print(":");
      lcd.setCursor(12,0);
      lcd.print(TimerTempMIN);
      lcd.setCursor(13,0);
      lcd.print("0");
      analogWrite(DIOD,0);
      }
    //Print power and PWM
    // Время работы аппарата (сумарное)
    //Power10or100=1;
 }
  tone(AUDIO, 500, 1000);
   //PWM=PWM_ARRAY[9];
} 

void loop() {
       
  ButtomPlus();
  ButtomMinus();
  ButtomPause();
  ButtomStart();
  ButtomMosh();
  Timer();
  ALARM();
}

