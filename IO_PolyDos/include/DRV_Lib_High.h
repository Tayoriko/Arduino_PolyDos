#include <Arduino.h>

//специальные типы данных 
  union Byte_DEV {
    unsigned char value;
    struct {
      unsigned Hand: 	1;	//0: 0 - авто, 1 - ручной
      unsigned Start_A: 1;	//1: команда запуска от алгоритма
      unsigned Start_M: 1;	//2: команда запуска от HMI
      unsigned alm_Ext:	1;	//3: внешняя авария, 0 - норма, 1 - авария
      unsigned alm_QF:	1;	//4: готовность защитного автомата, 0 - норма, 1 - авария
      unsigned alm_KM:	1;	//5: сигнал "в работе", 0 - норма, 1 - авария при запуске/останове
      unsigned Reset: 	1;	//6: наличие аварии привода, 0 - норма, 1 - авария
      unsigned CMD: 	1;	//7: команда запуска
    };
  };

//Двигатель с прямым пуском
	//Состав драйвера
		class DRV_MD
		{
			public:
				void ee_control(int addr, bool update, bool first_load);
				bool exe(bool i_QF, bool i_Alm, bool i_KM);
				void HMI();
				Byte_DEV CMD;
				Byte_DEV cfg;
				int Status = 0;
				word comm_in;
				word comm_out;
				TON_s T_ON;
				TON_s T_OFF;
				int HMI_ID = 0;
			private:
				byte x1;
				byte x2;
				bool alm;
		};

	// Работа с памятью
	void DRV_MD::ee_control(int addr, bool update, bool first_load)
	{
		if (first_load == 1)
		{
			x1 = EEPROM.readByte(addr);
			cfg.alm_Ext = bitRead(x1, 3);
			cfg.alm_QF = bitRead(x1, 4);
			cfg.alm_KM = bitRead(x1, 5);
			bitWrite(comm_in, 3, cfg.alm_Ext);
			bitWrite(comm_in, 4, cfg.alm_QF);
			bitWrite(comm_in, 5, cfg.alm_KM);
			comm_in = comm_out;
			HMI_ID = addr;
			HMI_Data[HMI_ID] = comm_in;
			HMI_Data[HMI_ID + 1] = comm_out;
			CMD.Reset = 1;
		}

		if (update == 1)
		{
			bitWrite(x2, 3, cfg.alm_Ext);
			bitWrite(x2, 4, cfg.alm_QF);
			bitWrite(x2, 5, cfg.alm_KM);
			EEPROM.updateByte(addr, x2);
			bitWrite(comm_out, 3, 0);
			bitWrite(comm_out, 4, 0);
			bitWrite(comm_out, 5, 0);
			bitWrite(comm_in, 3, cfg.alm_Ext);
			bitWrite(comm_in, 4, cfg.alm_QF);
			bitWrite(comm_in, 5, cfg.alm_KM);
			bitWrite(comm_in, 6, 0);
			bitWrite(comm_in, 8, 0);
			HMI_ID = addr;
			HMI_Data[HMI_ID] = comm_in;
			HMI_Data[HMI_ID + 1] = comm_out;
		}
	}

	// Работа с панелью
	void DRV_MD::HMI()
	{
		// чтение команд с HMI
		comm_in = HMI_Data[HMI_ID];			// входные команды с панели
		if (bitRead(comm_in, 0) == 1)	{CMD.Hand = 1;} else {CMD.Hand = 0;}
		if (bitRead(comm_in, 6) == 1)	{CMD.Reset = 1;}
		if (CMD.Hand == 1)	{CMD.Start_M = bitRead(comm_in, 2);} else {bitWrite(HMI_Data[HMI_ID], 2, CMD.Start_M);}

		//флаг редактирования свойств
			if (bitRead(comm_in, 8) == 0)
			{
				bitWrite(comm_out, 3, CMD.alm_Ext);
				bitWrite(comm_out, 4, CMD.alm_QF);
				bitWrite(comm_out, 5, CMD.alm_KM);
				bitWrite(comm_in, 3, cfg.alm_Ext);
				bitWrite(comm_in, 4, cfg.alm_QF);
				bitWrite(comm_in, 5, cfg.alm_KM);
			}
			else
			{
				cfg.alm_Ext = bitRead(comm_in, 3);
				cfg.alm_QF = bitRead(comm_in, 4);
				cfg.alm_KM = bitRead(comm_in, 5);
				bitWrite(comm_out, 3, cfg.alm_Ext);
				bitWrite(comm_out, 4, cfg.alm_QF);
				bitWrite(comm_out, 5, cfg.alm_KM);
			}

		// передача данных в HMI
			bitWrite(comm_out, 0, CMD.Hand);
			bitWrite(comm_out, 1, CMD.Start_A);
			bitWrite(comm_out, 2, CMD.Start_M);
			// bitWrite(comm_out, 3, CMD.alm_Ext);
			// bitWrite(comm_out, 4, CMD.alm_QF);
			// bitWrite(comm_out, 5, CMD.alm_KM);
			bitWrite(comm_out, 6, CMD.Reset);
			bitWrite(comm_out, 7, CMD.CMD);

		HMI_Data[HMI_ID + 1] = comm_out;		// отображение команд на панели
		HMI_Data[HMI_ID + 2] = Status;		// статус
	}

	//формирование статуса
		bool DRV_MD::exe(bool i_QF, bool i_Alm, bool i_KM)
		{	
			// вызов вспомогательных функций
				T_ON.exe(pulse.s01);
				T_OFF.exe(pulse.s01);
				if (pulse.Reset_All == 1) {CMD.Reset = 1;}

			// контроль аварий
				// внешняя авария
					if (cfg.alm_Ext == 1) {if (i_Alm == 1) {CMD.alm_Ext = 0;} else {CMD.alm_Ext = 1; Status = 4; alm = 1;}}	else {CMD.alm_Ext = 0;}
				// защитный автомат
					if (cfg.alm_QF == 1) {if (i_QF == 1) {CMD.alm_QF = 0;} else {CMD.alm_QF = 1; Status = 5; alm = 1;}}	else {CMD.alm_QF = 0;}
				// контроль работы
					if (cfg.alm_KM == 1 and alm == 0)
					{
						//контроль запуска
							if (CMD.CMD == 1) {T_ON.CMD.Start = 1;} else {T_ON.CMD.Start = 0;}
							if (CMD.CMD == 1 and T_ON.CMD.Exit == 0 and i_KM == 0) {Status = 2;}
							if (CMD.CMD == 1 and T_ON.CMD.Exit == 1 and i_KM == 0)	{CMD.alm_KM = 1; Status = 6; alm = 1;}
							if (CMD.CMD == 1 and i_KM == 1)	{T_ON.CMD.Start = 0; Status = 1;}
						// контроль останова
							if (CMD.CMD == 0) {T_OFF.CMD.Start = 1;} else {T_OFF.CMD.Start = 0;}
							if (CMD.CMD == 0 and T_OFF.CMD.Exit == 0 and i_KM == 1) {Status = 3;}
							if (CMD.CMD == 0 and T_OFF.CMD.Exit == 1 and i_KM == 1)	{CMD.alm_KM = 1; Status = 7; alm = 1;}
							if (CMD.CMD == 0 and i_KM == 0)	{T_OFF.CMD.Start = 0; Status = 0;}
					}
					else
					{
						if (CMD.CMD == 1 and alm == 0) { Status = 1;}
						if (CMD.CMD == 0 and alm == 0) { Status = 0;}
					}
					if (Status >= 4) {pulse.Any_Alarm = 1;}
				//сброс аварий
					if (CMD.Reset == 1)
					{
						T_ON.CMD.Reset = 1;
						T_OFF.CMD.Reset = 1;
						CMD.alm_Ext = 0;
						CMD.alm_QF = 0;
						CMD.alm_KM = 0;
						Status = 0;
						alm = 0;
						CMD.Reset = 0;
					}

			// управление в автоматическом режиме
			if (CMD.Hand == 0)
			{
				if (Status <= 3) {CMD.Start_M = CMD.Start_A;} else {CMD.Start_M = 0;}
			}

			// управление в ручном режиме
			if (CMD.Hand == 1)
			{
				if (Status > 3) {CMD.Start_M = 0;}
			}

			// управление запуском
			CMD.CMD = CMD.Start_M;
			
			// возвращение команды запуска
			return CMD.CMD;
		}