#include <Arduino.h>

// операции с энергонезависимой памятью
class DRV_EEPROM
{
  public:
      void load();        //чтение данных при запуске
      void update();      //сохранение изменений
      void demo();
      int ee_load = 0;    //флаг состояния памяти
      bool ee_update = 0;  //команда обновления данных
      bool ee_1st = 0;     //флаг первого запуска
  private:
    int addr = 0;
    int sys = 0;
    #define void_addr       1000
    #define void_data        666
    #define start_IN           1
    #define start_Q           26
    #define start_IN_Analog   50
    #define start_Q_Analog    70
    #define start_MD          83
    #define start_MIX          1
    #define start_Pump        95
    #define start_Line        106
    #define data_C            2000
    #define data_sys          2004
    #define data_addr         2006
};

// обновление данных
void DRV_EEPROM::update()
{
  if (ee_update == 1 or ee_1st == 1)
  {
    // чтение конфигурации - конфигурация дискретных входов
    for (int i = start_IN; i < start_IN + 24; i++)
    {
      addr = i * 2 - 1;
      IN[i].ee_control(addr, ee_update, ee_1st);
    }

    // чтение конфигурации - конфигурация дискретных выходов
    for (int i = start_Q; i < start_Q + 20; i++)
    {
      addr = i * 2 - 1;
      Q[i - start_Q + 1].ee_control(addr, ee_update, ee_1st);
    }

    // чтение конфигурации - конфигурация аналоговых входов
    for (int i = 1; i <= 4; i++)
    { 
      if (i == 1) {addr = start_IN_Analog * 2;}
      if (i > 1) {addr = start_IN_Analog * 2 + i * 10 - 10;}
      IN_Analog[i].ee_control(addr, ee_update, ee_1st);
    }

    // чтение конфигурации - конфигурация аналоговых выходов
    for (int i = 1; i <= 4; i++)
    { 
      if (i == 1) {addr = start_Q_Analog * 2;}
      if (i > 1) {addr = start_Q_Analog * 2 + i * 4 - 4;}
      Q_Analog[i].ee_control(addr, ee_update, ee_1st);
    }

    // чтение конфигурации - конфигурация приводов
    for (int i = 1; i < MD_cnt; i++)
    {
      if (i == 1) {addr = start_MD * 2;}
      if (i > 1) {addr = start_MD * 2 + i * 3 - 3;}
      MD[i].ee_control(addr, ee_update, ee_1st);
    }

    // чтение конфигурации - конфигурация насосов
    for (int i = 1; i <= 4; i++)
    {
      if (i == 1) {addr = start_Pump;}
      if (i > 1) {addr = start_Pump + i * 3 - 3;}
      Pump[i].ee_control(addr, ee_update, ee_1st);
    }

    // чтение конфигурации - конфигурация линий
    for (int i = 1; i <= 4; i++)
    {
      Line[i].ID = i;
      if (i == 1) {addr = start_Line;}
      if (i > 1) {addr = start_Line + i * 5 - 5;}
      Line[i].shift = i - 1;
      Line[i].ee_control(addr, ee_update, ee_1st);
    }

    // чтение конфигурации - конфигурация мешалок
    for (int i = 1; i <= 2; i++)
    {
      MIX[i].ID = i;
      MIX[i].shift = i;
      if (i == 1) {addr = start_MIX;}
      if (i > 1) {addr = start_MIX + i * 4 - 4;}
      MIX[i].ee_control(addr, ee_update, ee_1st);
    }

    // запись С
    if (ee_1st == 1)    {C = EEPROM.readFloat(data_C);  HMI_Data[200] = int(C / 100);}
    if (ee_update == 1) {EEPROM.updateFloat(data_C, C); }

    // системная конфигурация
    if (ee_1st == 1)
    {
      sys = EEPROM.readInt(data_sys);
      rs485_addr = EEPROM.readInt(data_addr);
      bitWrite(sys_in, 10, mode_rs_sp);
      bitWrite(sys_in, 10, mode_rs_cmd);
      HMI_Data[96] = rs485_addr;
      HMI_Data[97] = sys_in;
    }
    if (ee_update == 1)
    {
      bitWrite(sys, 10, mode_rs_sp);
      bitWrite(sys, 11, mode_rs_cmd);
      EEPROM.writeInt(data_sys, sys);
      EEPROM.writeInt(data_addr, rs485_addr);
    }
  }
  
  // сброс команд
  ee_update = 0;
  ee_1st = 0;
}

// загрузка данных при запуске
void DRV_EEPROM::load()
{
    // чтение флага инициализации
    ee_load = EEPROM.readInt(void_addr);

    // чтение конфигурации
    if (ee_load == void_data)
      {
        // флаг первого запуска
        ee_1st = 1;
        ee_update = 0;

        // обновление данных
        update();
      }

    // первый запуск
    if (ee_load != void_data)
      {
        // предконфигурация - конфигурация дискретных входов
        for (int i = 1; i <= 24; i++)
        {
          IN[i].CMD.Inv = 0;
        }

        // предконфигурация - конфигурация дискретных выходов
         for (int i = 1; i <= 20; i++)
        {
          Q[i].CMD.Inv = 0;
        }

        // предконфигурация - конфигурация аналоговых входов
         for (int i = 1; i <= 4; i++)
        {
          IN_Analog[i].max_out = 100.0;
        }

        // предконфигурация - конфигурация аналоговых выходов
         for (int i = 1; i <= 4; i++)
        {
          Q_Analog[i].max_out = 100.0; // л/ч
        }

        // предконфигурация - конфигурация приводов
        for (int i = 1; i <= MD_cnt; i++)
        {
          // 0 - отсутствует, 1 - присутствует
          MD[i].cfg.alm_QF = 0;   //защитный автомат
          MD[i].cfg.alm_Ext = 0;  //внешняя авария
          MD[i].cfg.alm_KM = 0;   //ОС "В работе",
        }

        // предконфигурация - конфигурация линий
        for (int i = 1; i <= 4; i++)
        {
          Line[i].D = 1.0;
          Line[i].Flow = 100;
          Line[i].K = 0.01;
          Line[i].Ti = 10;
          Line[i].CMD.Hand = 0;
          Line[i].CMD.Start = 0;
          Line[i].T_RT_ON.tSP = 5;
          Line[i].T_RT_OFF.tSP = 5;
          Line[i].T_RM_UP.tSP = 5;
          Line[i].T_RM_DOWN.tSP = 5;
          Line[i].ID = i;
        }

        // предконфигурация - конфигурация насосов
        for (int i = 1; i <= 4; i++)
        {
          Pump[i].xMode = 1;
          Pump[i].xLine = i;
          Pump[i].xMD = i;
          Pump[i].T_Main.tSP = 0;
        }

        // предконфигурация - конфигурация мешалок
        for (int i = 1; i <= 2; i++)
        {
          MIX[i].CMD.Mix_Use = 0;
          MIX[i].CMD.Mix_Start = 0;
          MIX[i].CMD.Mix_Sel = 0;
          MIX[i].SP_ON = 5;
          MIX[i].SP_OFF = 10;
        }

        // предконфигурация RS485
        HMI_Data[96] = rs485_addr;
        mode_rs_sp = 1;
        bitWrite(sys_in, 10, mode_rs_sp);
        bitWrite(sys_in, 11, mode_rs_cmd);
        
        //чтение концентрации
        C = EEPROM.readFloat(data_C);  
        HMI_Data[200] = int(C / 100);

        // метка первого сохранения данных
        EEPROM.updateInt(void_addr, void_data);

        // обновление данных
        ee_update = 1;
        update();
      }
}

// тестовый режим
void DRV_EEPROM::demo()
{
  // тестовый режим
    if (mode_demo == 1)
    {
      // отключение выходов при активации режима
      if (pos[0].exe(bool(mode_demo)) == 1)
      {
        cnt_x = 1;
        for (size_t i = 1; i <= 20; i++)
        {
          Q[i].CMD.Start_A = 0;
        }
      }
      // циклиный запуск выходов в демо режиме
      if (pulse.s1 == 1 and Line[1].CMD.Start)
      {
        cnt++;
      }
      // управление выходами
      if (Line[1].CMD.Start == 1)
      {
        Q[cnt_x].CMD.Start_A = 1;
        if (cnt_x > 1) {Q[cnt_x - 1].CMD.Start_A = 0;}
        if (cnt_x == 1){Q[20].CMD.Start_A = 0;}
      }
      // переключение выходов
      if (cnt >= int(Line[1].D))
      {
        cnt_x++;
        if (cnt_x >= 21) {cnt_x = 1;}
        cnt = 0;
      }
    }
    // отключение при деактивации режима
    else
    {
      if (neg[0].exe(bool(mode_demo)) == 1)
      {
        cnt_x = 1;
        for (size_t i = 1; i <= 20; i++)
        {
          Q[i].CMD.Start_A = 0;
        }
      }
    }
}
