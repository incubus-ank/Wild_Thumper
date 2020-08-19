#include <Servo.h>
#include "IOpins.h"
#include "Constants.h"


//-------------------------------------------------------------- определение глобальных переменных --------------------------------------------

unsigned int Volts;
unsigned int LeftAmps;
unsigned int RightAmps;
unsigned long chargeTimer;
unsigned long leftoverload;
unsigned long rightoverload;
int highVolts;
int startVolts;
int Leftspeed=0;
int Rightspeed=0;
int Speed;
int Steer;
byte Charged=1;                                               // 0 = разряженный аккумулятор 1 = заряженный аккумулятор
int Leftmode=1;                                               // 0 = назад, 1 = тормоз, 2 = вперед
int Rightmode=1;                                              // 0 = назад, 1 = тормоз, 2 = вперед
byte Leftmodechange=0;                                        // Левый вход должен быть 1500, прежде чем может произойти торможение или движение назад
byte Rightmodechange=0;                                       // Правый вход должен быть 1500, прежде чем может произойти торможение или движение назад
int LeftPWM;                                                  // Значение ШИМ для левой скорости двигателя / тормоза
int RightPWM;                                                 // Значение ШИМ для левой скорости двигателя / тормоза
int data;
int servo[7];

//-------------------------------------------------------------- Определение сервоприводов ------------------------------------------------------


Servo Servo0;                                                 // Определение сервопривода
Servo Servo1;                                                 // Определение сервопривода
Servo Servo2;                                                 // Определение сервопривода
Servo Servo3;                                                 // Определение сервопривода
Servo Servo4;                                                 // Определение сервопривода
Servo Servo5;                                                 // Определение сервопривода
Servo Servo6;                                                 // Определение сервопривода

void setup()
{
  //------------------------------------------------------------ Инициализация сервопривода ----------------------------------------------------

  Servo0.attach(S0);                                          // Сервопривод подключен к пину ввода / вывода "номер"
  Servo1.attach(S1);                                          // Сервопривод подключен к пину ввода / вывода "номер"
  Servo2.attach(S2);                                          // Сервопривод подключен к пину ввода / вывода "номер"
  Servo3.attach(S3);                                          // Сервопривод подключен к пину ввода / вывода "номер"
  Servo4.attach(S4);                                          // Сервопривод подключен к пину ввода / вывода "номер"
  Servo5.attach(S5);                                          // Сервопривод подключен к пину ввода / вывода "номер"
  Servo6.attach(S6);                                          // Сервопривод подключен к пину ввода / вывода "номер"

  //------------------------------------------------------------ Установить сервоприводы в положение по умолчанию ---------------------------------------

  Servo0.writeMicroseconds(DServo0);                          // установить сервопривод в "угол"
  Servo1.writeMicroseconds(DServo1);                          // установить сервопривод в "угол"
  Servo2.writeMicroseconds(DServo2);                          // установить сервопривод в "угол"
  Servo3.writeMicroseconds(DServo3);                          // установить сервопривод в "угол"
  Servo4.writeMicroseconds(DServo4);                          // установить сервопривод в "угол"
  Servo5.writeMicroseconds(DServo5);                          // установить сервопривод в "угол"
  Servo6.writeMicroseconds(DServo6);                          // установить сервопривод в "угол"

  //------------------------------------------------------------ Инициализация пинов ввода/вывода --------------------------------------------------

  pinMode (Charger,OUTPUT);                                   // Установить пин зарядки, как выходящий
  digitalWrite (Charger,1);                                   // отключить регулятор тока для зарядки аккумулятора

  if (Cmode==1) 
  {
    Serial.begin(Brate);                                      // включить последовательную связь на "частоте", если Cmode = 1
    Serial.flush();                                           // отчитстить буфер
  } 
  //Serial.begin(57600);
}


void loop()
{
  //------------------------------------------------------------ Проверьте напряжение аккумулятора и ток двигателя ---------------------

  Volts=analogRead(Battery);                                  // Считать вольтаж аккумулятора
  LeftAmps=analogRead(LmotorC);                               // Считать ток левого двигателя
  RightAmps=analogRead(RmotorC);                              // Считать ток правого двигателя

  //Serial.print(LeftAmps);
  //Serial.print("    ");
  //Serial.println(RightAmps);

  if (LeftAmps>Leftmaxamps)                                   // превышено ли допустимое потребление тока двигателя 
  {
    analogWrite (LmotorA,0);                                  // Выключить двигатель
    analogWrite (LmotorB,0);                                  // выключить двигатель
    leftoverload=millis();                                    // Время отключения
  }

  if (RightAmps>Rightmaxamps)                                 // превышено ли допустимое потребление тока двигателя 
  {
    analogWrite (RmotorA,0);                                  // Выключить двигатель
    analogWrite (RmotorB,0);                                  // Выключить двигатель
    rightoverload=millis();                                   // Время отключения
  }

  if ((Volts<lowvolt) && (Charged==1))                        // Проверить состояние аккумулятора
  {                                                           // изменить состояние батареи с заряженной на разряженную

    //---------------------------------------------------------- Регулятор скорости FLAT BATTERY выключается, пока аккумулятор не зарядится ----
    //---------------------------------------------------------- Это функция безопасности для предотвращения неисправности при низком напряжении !! ------

    Charged=0;                                                // аккумулятор разряжен
    highVolts=Volts;                                          // записть вольтаж
    startVolts=Volts;
    chargeTimer=millis();                                     // запись вольтажа

    digitalWrite (Charger,0);                                 // включить регулятор тока для зарядки аккумулятора
  }

  //------------------------------------------------------------ ЗАРЯДКА АККУМУЛЯТОРА -------------------------------------------------------

  if ((Charged==0) && (Volts-startVolts>67))                  // если аккумулятор разряжен и подключено зарядное устройство (напряжение увеличилось как минимум на 1 В)
  {
    if (Volts>highVolts)                                      // напряжение батареи увеличилось?
    {
      highVolts=Volts;                                        // Записывается максимальное напряжение. Используется для обнаружения пиковой зарядки.
      chargeTimer=millis();                                   // когда напряжение увеличивается, запишите время
    }

    if (Volts>batvolt)                                        // напряжение батареи должно быть выше, чем это может произойти до пиковой зарядки.
    {
      if ((highVolts-Volts)>5 || (millis()-chargeTimer)>chargetimeout) // напряжение начало падать или выровнялось?
      {
        Charged=1;                                            // напряжение батареи достигло максимума
        digitalWrite (Charger,1);                             // выключить регулятор тока
      }
    } 
  }

  else

  {//----------------------------------------------------------- АККУМУЛЯТОР ЗАРЯЖЕН, РЕГУЛЯТОР СКОРОСТИ РАБОТАЕТ НОРМАЛЬНО ----------------------

    switch(Cmode)
    {
    case 0:                                                   // Режим RC через D0 и D1
      RCmode();
      break;

    case 1:                                                   // Последовательный режим через D0 (RX) и D1 (TX)
      SCmode();
      break;

    case 2:                                                   // Режим I2C через A4 (SDA) и A5 (SCL)
      I2Cmode();
      break;
    }

    // --------------------------------------------------------- Код для привода двойного "H" моста --------------------------------------

    if (Charged==1)                                           // Если аккумулятор заряжен
    {
      if ((millis()-leftoverload)>overloadtime)             
      {
        switch (Leftmode)                                     // если левый мотор не перегружен
        {
        case 2:                                               // левый мотор вперед
          analogWrite(LmotorA,0);
          analogWrite(LmotorB,LeftPWM);
          break;

        case 1:                                               // Левый мотор стоп
          analogWrite(LmotorA,LeftPWM);
          analogWrite(LmotorB,LeftPWM);
          break;

        case 0:                                               // Левый мотор назад 
          analogWrite(LmotorA,LeftPWM);
          analogWrite(LmotorB,0);
          break;
        }
      }
      if ((millis()-rightoverload)>overloadtime)
      {
        switch (Rightmode)                                    // если правый мотор не перегружен
        {
        case 2:                                               // Правый мотор вперед
          analogWrite(RmotorA,0);
          analogWrite(RmotorB,RightPWM);
          break;

        case 1:                                               // Правый мотор стоп
          analogWrite(RmotorA,RightPWM);
          analogWrite(RmotorB,RightPWM);
          break;

        case 0:                                               // Правый мотор назад
          analogWrite(RmotorA,RightPWM);
          analogWrite(RmotorB,0);
          break;
        }
      } 
    }
    else                                                      // Аккумулятор разряжен
    {
      analogWrite (LmotorA,0);                                // Выключить двигатель
      analogWrite (LmotorB,0);                                // Выключить двигатель
      analogWrite (RmotorA,0);                                // Выключить двигатель
      analogWrite (RmotorB,0);                                // Выключить двигатель
    }
  }
}






void RCmode()
{
  //------------------------------------------------------------ Код для входов RC ---------------------------------------------------------

  Speed=pulseIn(RCleft,HIGH,25000);                           // Считать левый джостик
  Steer=pulseIn(RCright,HIGH,25000);                          // Считать правый джостик


  if (Speed==0) Speed=1500;                                   // если время импульса истекло (25 мс), установите скорость на остановки
  if (Steer==0) Steer=1500;                                   // если время импульса истекло (25 мс), тогда установите рулевое управление в центр

  if (abs(Speed-1500)<RCdeadband) Speed=1500;                 // если вход скорости находится в пределах зоны нечувствительности, установленной на 1500 (1500 мкс = центральное положение для большинства сервоприводов)
  if (abs(Steer-1500)<RCdeadband) Steer=1500;                 // если вход рулевого управления находится в пределах зоны нечувствительности, установленной на 1500 (1500 мкс = центральное положение для большинства сервоприводов)

  if (Mix==1)                                                 // Смешены скорость и сигналы руля
  {
    Steer=Steer-1500;
    Leftspeed=Speed-Steer;
    Rightspeed=Speed+Steer;
  }
  else                                                        // Индивидуальный контроль стки
  {
    Leftspeed=Speed;
    Rightspeed=Steer;
  }
  /*
  Serial.print("Left:");
  Serial.print(Leftspeed);
  Serial.print(" -- Right:");
  Serial.println(Rightspeed);
  */
  Leftmode=2;
  Rightmode=2;
  if (Leftspeed>(Leftcenter+RCdeadband)) Leftmode=0;          // Если пришло значение езды вперед, то поехать вперед левым двигателем
  if (Rightspeed>(Rightcenter+RCdeadband)) Rightmode=0;       // Если пришло значение езды вперед, то поехать вперед правым двигателем

  LeftPWM=abs(Leftspeed-Leftcenter)*10/scale;                 // Шкала от 1000-2000 до 0-255
  LeftPWM=min(LeftPWM,255);                                   // Установить люмит 255

  RightPWM=abs(Rightspeed-Rightcenter)*10/scale;              // Шкала от 1000-2000 до 0-255
  RightPWM=min(RightPWM,255);                                 // Установить люмит 255
}







void SCmode()
{// ------------------------------------------------------------ Код для последовательной связи --------------------------------------

                                                              // FL = очистить последовательный буфер
 
                                                              // AN = описать аналоговые входы 1-5
                                                              
                                                              // SV = следующие 7 целых чисел будут позиционной информацией для сервоприводов 0-6
 
                                                              // HB = Данные моста "H" - следующие 4 байта будут:
                                                              //      Левый мотор режим 0-2
                                                              //      Левый мотор скорость  0-255
                                                              //      правый мотор режим 0-2
                                                              //      правый мотор скорость  0-255
   
 
  if (Serial.available()>1)                                   // команда доступна
  {
    int A=Serial.read();
    int B=Serial.read();
    int command=A*256+B;
    switch (command)
    {
      case 17996:                                             // FL
        Serial.flush();                                       // Отчистить буфер
        break;
        
      case 16718:                                             // AN - Отобразить значение аналоговых входов 1-5
        for (int i=1;i<6;i++)                                 // индекс аналоговые входу 1-5
        {
          data=analogRead(i);                                 // читать 10-битный аналоговый вход
          Serial.write(highByte(data));                       // передать старший байт
          Serial.write(lowByte(data));                        // передать младший байт
        }
        break;
              
       case 21334:                                            // SV - получить информацию о позициях для сервоприводов 0-6
         for (int i=0;i<15;i++)                               // читать 14 байтов данных
         {
           Serialread();                                      
           servo[i]=data;
         }
         Servo0.writeMicroseconds(servo[0]*256+servo[1]);     // установить положение сервопривода
         Servo1.writeMicroseconds(servo[2]*256+servo[3]);     // установить положение сервопривода
         Servo2.writeMicroseconds(servo[4]*256+servo[5]);     // установить положение сервопривода
         Servo3.writeMicroseconds(servo[6]*256+servo[7]);     // установить положение сервопривода
         Servo4.writeMicroseconds(servo[8]*256+servo[9]);     // установить положение сервопривода
         Servo5.writeMicroseconds(servo[10]*256+servo[11]);   // установить положение сервопривода
         Servo6.writeMicroseconds(servo[12]*256+servo[13]);   // установить положение сервопривода
         break;
       
       case 18498:                                            // HB - данные режима и ШИМ для левого и правого двигателей
         Serial.read();
         Leftmode=data;
         Serial.read();
         LeftPWM=data;
         Serial.read();
         Rightmode=data;
         Serial.read();
         RightPWM=data;
         break;
         
       default:                                                // неверная команда
         Serial.flush();                                       // отчистка буфера
    }
  }
}

void Serialread() 
{//---------------------------------------------------------- Считывать последовательный порт, пока данные не будут получены -----------------------------------
  do 
  {
    data=Serial.read();
  } while (data<0);
}
    






void I2Cmode()
{//----------------------------------------------------------- Ваш код писать сюда ------------------------------------------------------------

}
