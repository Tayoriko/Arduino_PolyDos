#include <Arduino.h>
#include <avr/wdt.h>

// версия 2 - маленькая съёмная плата
// версия 3 - большая съемная плата
// версия 4 - одноплатная v1
#define version 4

//подключаем дополнительные библиотеки
#include "EEPROMex.cpp"      //работа с энергонезависимой памятью
#include "ModBusRTU.h"      //работа с ModBus RTU
#include "DRV_Lib_Sub.h"    //типовые функции - таймеры

// СИСТЕМНЫЕ ПЕРЕМЕННЫЕ
int cnt = 0;
int cnt_x = 0;
word sys_in = 0;
word sys_out = 0;
front_neg neg[15];
front_pos pos[15];
float C = 0.1;
int mode_rs_sp = 0;
int mode_rs_cmd = 0;
int mode_PolyDos = 1;
int mode_PolyMix = 0;
int mode_demo = 0;
int sys_screen = 0;
  // ПОРТЫ
  //com0 - 0,8
  //com1 - 1,44
    // HMI
      int hmi_addr = 1;
      int8_t hmi_state = 0;
      uint16_t HMI_Data[256];
      Modbus HMI(hmi_addr, 1, 44);
    // RS485
      int rs485_addr = 10;
      int8_t rs485_state = 0;
      uint16_t RS_Data[256];
      Modbus RS(rs485_addr, 0, 8);
  // ТАЙМЕРЫ
    drv_pulse pulse;

#include "DRV_Lib_Low.h"    //типовые функции - DI, DO, AI, AO
#include "DRV_Lib_High.h"   //типовые функции - насос, дозатор, задвижка

// УСТРОЙСТВА - ОБЪЯВЛЕНИЕ
  // дискретный вход
    DRV_DI IN[25];
  // дискретный выход
    DRV_DO Q[21];
  // аналоговый вход
    DRV_AI IN_Analog[5];
  // аналоговый выход
    DRV_AO Q_Analog[5];

  // привода с дискретным управлением
    #define MD_cnt 7    // количество приводов + 1
    DRV_MD MD[MD_cnt];

// АЛГОРИТМИКА
#include "DRV_Line.h"   //линии дозации
  DRV_Line Line[5];       //количество линий + 1
#include "DRV_Pump.h"   //управление насосами
  DRV_Pump Pump[5];       //количество насосов + 1
#include "DRV_Mix.h"    //управление мешалками
  DRV_Mix MIX[3];         //количество емкостей + 1
#include "GyverPID.h"       //ПИД
// ПИД-регуляторы
  float PID_Min = 0.5;
  float PID_Max = 0.75;
  GyverPID PID_Line_1(Line[1].K, Line[1].Ti, 0.0, 10);
  GyverPID PID_Line_2(Line[2].K, Line[2].Ti, 0.0, 10);
  GyverPID PID_Line_3(Line[3].K, Line[3].Ti, 0.0, 10);
  GyverPID PID_Line_4(Line[4].K, Line[4].Ti, 0.0, 10);
  void PID_init()
  {
    PID_Line_1.setLimits(PID_Min, PID_Max);
    PID_Line_2.setLimits(PID_Min, PID_Max);
    PID_Line_3.setLimits(PID_Min, PID_Max);
    PID_Line_4.setLimits(PID_Min, PID_Max);
  }
  void PID_exe()
  {
    PID_Line_1.setpoint = Line[1].SP_M;
    PID_Line_2.setpoint = Line[2].SP_M;
    PID_Line_3.setpoint = Line[3].SP_M;
    PID_Line_4.setpoint = Line[4].SP_M;
    PID_Line_1.input = IN_Analog[1].Result;
    PID_Line_2.input = IN_Analog[2].Result;
    PID_Line_3.input = IN_Analog[3].Result;
    PID_Line_4.input = IN_Analog[4].Result;
    if (Line[1].CMD.Use_RM == 1 and (Line[1].CMD.CMD_Main == 1 or Line[1].CMD.CMD_Reserv == 1) and Line[1].SP_M > 0) {Line[1].PID_SP = PID_Line_1.getResultTimer();}
    if (Line[2].CMD.Use_RM == 1 and (Line[2].CMD.CMD_Main == 1 or Line[2].CMD.CMD_Reserv == 1) and Line[2].SP_M > 0) {Line[2].PID_SP = PID_Line_2.getResultTimer();}
    if (Line[3].CMD.Use_RM == 1 and (Line[3].CMD.CMD_Main == 1 or Line[3].CMD.CMD_Reserv == 1) and Line[3].SP_M > 0) {Line[3].PID_SP = PID_Line_3.getResultTimer();}
    if (Line[4].CMD.Use_RM == 1 and (Line[4].CMD.CMD_Main == 1 or Line[4].CMD.CMD_Reserv == 1) and Line[4].SP_M > 0) {Line[4].PID_SP = PID_Line_4.getResultTimer();}
  }
  

// ВНУТРЕННЯЯ КОММУНИКАЦИЯ
#include "DRV_Lib_i2c.h"    //обмен данными
  // обмен с Atmega328p
  i2c_master i2c;

// ЭНЕРГОНЕЗАВИСИМАЯ ПАМЯТЬ
#include "DRV_Lib_eeprom.h" //массавые чтение и запись из EEPROM
  // контролько EEPROM
  DRV_EEPROM ee_control;

// SETUP
void setup() {
  
  // инициализация порта для панели
    HMI.begin(9600);
    HMI.setID(hmi_addr);

  // инициализация порта для связи
    if (version >= 4)
    {
      RS.begin(9600);
      RS.setID(rs485_addr);
    }
    
  // предконфигурация
    // для платы версии 2
    if (version == 2)
    {
      // адреса аналоговых входов
      IN_Analog[1].pin_AI = pulse.AIx[1];
      IN_Analog[2].pin_AI = pulse.AIx[2];
      IN_Analog[3].pin_AI = pulse.AIx[3];
      IN_Analog[4].pin_AI = pulse.AIx[4];

      // адреса аналоговых выходов
      Q_Analog[1].pin_AO = pulse.AOx[1];
      Q_Analog[2].pin_AO = pulse.AOx[2];
      Q_Analog[3].pin_AO = pulse.AOx[3];
      Q_Analog[4].pin_AO = pulse.AOx[4];
    }

    // для платы версии 3
    if (version == 3)
    {
      // адреса аналоговых входов
      IN_Analog[1].pin_AI = pulse.AIx[1];
      IN_Analog[2].pin_AI = pulse.AIx[2];
      IN_Analog[3].pin_AI = pulse.AIx[3];
      IN_Analog[4].pin_AI = pulse.AIx[4];

      // адреса аналоговых выходов
      Q_Analog[1].pin_AO = pulse.AOx[1];
      Q_Analog[2].pin_AO = pulse.AOx[2];
      Q_Analog[3].pin_AO = pulse.AOx[3];
      Q_Analog[4].pin_AO = pulse.AOx[4];
    }

    // для платы версии 4
    if (version == 4)
    {
      // дискретные входа в аналоговом режиме
      IN[10].CMD.Use = 1;
      IN[11].CMD.Use = 1;
      IN[17].CMD.Use = 1;
      IN[18].CMD.Use = 1;
      IN[19].CMD.Use = 1;

      // активация i2c
      i2c.setup();

      // насосы линий - связка с насосами
      for (size_t i = 1; i <= 4; i++)
      {
        Pump[i].xMD = i;
      }
    }
  
  // контроль EEPROM
    ee_control.load();

  // системные функции
    PID_init();
    pulse.init();
    wdt_enable(WDTO_4S);
}

// ОСНОВНОЙ АЛГОРИТМ - ДОЗИРОВАНИЕ
void logick_PolyDos()
  {
    if (mode_PolyDos == 1)
    {
     // линии дозации - проброс сигналов
      if (IN[21].CMD.Status == 1 and (MIX[1].CMD.Mix_LLS == 0 or MIX[1].CMD.Mix_Lock_LS == 0) and (MIX[2].CMD.Mix_LLS == 0 or MIX[2].CMD.Mix_Lock_LS == 0)) {Line[1].CMD.Ready = 1;} else {Line[1].CMD.Ready = 0;}
      if (IN[22].CMD.Status == 1 and (MIX[1].CMD.Mix_LLS == 0 or MIX[1].CMD.Mix_Lock_LS == 0) and (MIX[2].CMD.Mix_LLS == 0 or MIX[2].CMD.Mix_Lock_LS == 0)) {Line[2].CMD.Ready = 1;} else {Line[2].CMD.Ready = 0;}
      if (IN[23].CMD.Status == 1 and (MIX[1].CMD.Mix_LLS == 0 or MIX[1].CMD.Mix_Lock_LS == 0) and (MIX[2].CMD.Mix_LLS == 0 or MIX[2].CMD.Mix_Lock_LS == 0)) {Line[3].CMD.Ready = 1;} else {Line[3].CMD.Ready = 0;}
      if (IN[24].CMD.Status == 1 and (MIX[1].CMD.Mix_LLS == 0 or MIX[1].CMD.Mix_Lock_LS == 0) and (MIX[2].CMD.Mix_LLS == 0 or MIX[2].CMD.Mix_Lock_LS == 0)) {Line[4].CMD.Ready = 1;} else {Line[4].CMD.Ready = 0;}

    // линии дозации - вызов на исполнение
      for (size_t i = 1; i <= 4; i++)
      {
        Line[i].exe();
      }
    
    // насосы линий - вызов на исполнение
      for (size_t i = 1; i <= 4; i++)
      {
        Pump[i].exe();
      }
    
    // емкости приготовления - вызов на исполнение
      MIX[1].exe(IN[19].CMD.Status, IN[20].CMD.Status); //датчик нижнего уровня, датчик верхнего уровня
      MIX[2].exe(0, 0); //датчик нижнего уровня, датчик верхнего уровня

    // клапаны узлов разбавления
      Q[11].CMD.Start_A = Line[1].CMD.CMD_KC;
      Q[12].CMD.Start_A = Line[2].CMD.CMD_KC;
      Q[13].CMD.Start_A = Line[3].CMD.CMD_KC;
      Q[14].CMD.Start_A = Line[4].CMD.CMD_KC;

    // мешалки
      MD[5].CMD.Start_A = MIX[1].CMD.Mix_CMD;
      MD[6].CMD.Start_A = MIX[2].CMD.Mix_CMD;
      
    // ПИД-регуляторы
      PID_exe();
    }
   
  }

// ОСНОВНОЙ АЛГОРИТМ - ПРИГОТОВЛЕНИЕ
void logick_PolyMix()
  {
    if (mode_PolyMix == 1)
    {

    }
  }
// УСТРОЙСТВА - ОБРАБОТКА
void devices()
  {
    // дискретные входа
      for (size_t i = 1; i <= 24; i++)
      {
        IN[i].exe(pulse.DIx[i]);
      }

    // аналоговые входа
      for (size_t i = 1; i <= 4; i++)
      {
        IN_Analog[i].exe(2);
      }
      
    // аналоговые выхода
      for (size_t i = 1; i <= 4; i++)
      {
        if (Pump[i].Type == 0) {Q_Analog[i].Mode = 0;}
        if (Pump[i].Type > 0) {Q_Analog[i].Mode = 1;}
        Q_Analog[i].exe();
      }
    
    // привода
      Q[1].CMD.Start_A = MD[1].exe(IN[1].CMD.Status, IN[7].CMD.Status, IN[13].CMD.Status);//(bool i_QF, bool i_Alm, bool i_KM);
      Q[2].CMD.Start_A = MD[2].exe(IN[2].CMD.Status, IN[8].CMD.Status, IN[14].CMD.Status);//(bool i_QF, bool i_Alm, bool i_KM);
      Q[3].CMD.Start_A = MD[3].exe(IN[3].CMD.Status, IN[9].CMD.Status, IN[15].CMD.Status);//(bool i_QF, bool i_Alm, bool i_KM);
      Q[4].CMD.Start_A = MD[4].exe(IN[4].CMD.Status, IN[10].CMD.Status, IN[16].CMD.Status);//(bool i_QF, bool i_Alm, bool i_KM);
      Q[5].CMD.Start_A = MD[5].exe(IN[5].CMD.Status, IN[11].CMD.Status, IN[17].CMD.Status);//(bool i_QF, bool i_Alm, bool i_KM);
      Q[6].CMD.Start_A = MD[6].exe(IN[6].CMD.Status, IN[12].CMD.Status, IN[18].CMD.Status);//(bool i_QF, bool i_Alm, bool i_KM);

    // тестовый режим
      ee_control.demo();
    
    // дискретный выхода
      for (size_t i = 1; i <= 20; i++)
      {
        Q[i].exe(pulse.DOx[i]);
      }
  }

// HMI
void comm_HMI()
  {
    // системные функции
      rs485_addr = HMI_Data[96];
      sys_in = HMI_Data[97];
      HMI_Data[98] = sys_out;
      sys_screen = HMI_Data[99];

    // общие данные
      C = float(HMI_Data[200] / 100);

    //дискретные входа
    if (sys_screen == 10)
    {
      for (size_t i = 1; i <= 24; i++)
      {
        IN[i].HMI();
      }
    }

    // дискретные выхода
    if (sys_screen == 11)
    {
      for (size_t i = 1; i <= 20; i++)
      {
        Q[i].HMI();
      }
    }

    // аналоговые входа
      for (size_t i = 1; i <= 4; i++)
      {
        IN_Analog[i].HMI();
      }
    
    // аналоговые выхода
      for (size_t i = 1; i <= 4; i++)
      {
        Q_Analog[i].HMI();
      }

    // привода
      for (size_t i = 1; i < MD_cnt; i++)
      {
        MD[i].HMI();
      }

  // PolyDos
  if (sys_screen <= 10)
  {
    // насос-дозатры
      for (size_t i = 1; i <= 4; i++)
      {
        Pump[i].HMI();
      }

    // линии дозирования
      for (size_t i = 1; i <= 4; i++)
      {
        Line[i].HMI();
      }

    // емкости приготовления
      for (size_t i = 1; i <= 2; i++)
      {
        MIX[i].HMI();
      }
  }
    
    // системные операции
    hmi_state = HMI.poll(HMI_Data, 255); 
  }

// RS485
word_to_float convert;
void comm_RS485()
{
  // получение магистрального расхода
  if (mode_rs_sp == 1)
  {
    Line[1].Flow = convert.exe(RS_Data[9],RS_Data[10]);
    Line[2].Flow = convert.exe(RS_Data[11],RS_Data[12]);
    Line[3].Flow = convert.exe(RS_Data[13],RS_Data[14]);
    Line[4].Flow = convert.exe(RS_Data[15],RS_Data[16]);
  }

  // системные операции
    rs485_state = RS.poll(RS_Data, 255); 
}

// СИСТЕМНЫЕ ОПЕРАЦИИ
void system()
{
  // сохранение данных
  if (bitRead(sys_in, 0) == 1 and bitRead(sys_out, 0) == 0) {bitWrite(sys_out, 0, 1); ee_control.ee_update = 1; ee_control.update();}
  if (bitRead(sys_in, 0) == 0 and bitRead(sys_out, 0) == 1) {bitWrite(sys_out, 0, 0);}
  // тестовый режим
  mode_demo = bitRead(sys_in, 15);
  // режим задания уставки по RS485
  if (bitRead(sys_in, 10) == 1) {mode_rs_sp = 1;} else {mode_rs_sp = 0;}
  if (bitRead(sys_in, 11) == 1) {mode_rs_cmd = 1;} else {mode_rs_cmd = 0;}
  // индикатор аварии
  if (pulse.Any_Alarm) {bitWrite(sys_out, 1, 1);} else {bitWrite(sys_out, 1, 0);}
  if (pulse.Any_Alarm == 1 and mode_demo == 0) {Q[10].CMD.Start_A = 1;} else {Q[10].CMD.Start_A = 0;}
  // сброс аварий
  pulse.Reset_All = bitRead(sys_in, 1);
}

// ОСНОВНОЙ ЦИКЛ ПРОГРАММЫ
void loop() {
  pulse.exe();
  i2c.exe();
  system();
  logick_PolyDos();
  devices();
  comm_HMI();
  comm_RS485();
  pulse.end();
  wdt_reset();
}