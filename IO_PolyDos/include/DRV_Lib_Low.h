#include <Arduino.h>

//специальные типы данных 
  union Byte_CMD {
    unsigned char value;
    struct {
      unsigned Hand: 	1;	//0: 0 - авто, 1 - ручной
      unsigned Start_A: 1;	//1: команда запуска от алгоритма
      unsigned Start_M: 1;	//2: команда запуска от HMI
      unsigned Use: 	1;	//3: флаг активации
      unsigned Inv: 	1;	//4: инверсия сигнала
      unsigned Data: 	1;	//5: входной сигнал
      unsigned Status: 	1;	//6: состояние входа/выхода: 0 - нет сигнала, 1 - есть сигнал
      unsigned CMD: 	1;	//7: выходной сигнал
    };
  };

  union Byte_AI_alm {
    unsigned char value;
    struct {
      unsigned Alm: 	1;	//0: общая авария
      unsigned SC:		1;	//1: перегрузка - сигнал выше 20 мА
      unsigned NC: 		1;	//2: обрыв - сигнал ниже 4 мА
      unsigned LL: 		1;	//3: низкий уровень - авария
      unsigned L: 		1;	//4: низкий уровень - предупреждение
      unsigned H: 		1;	//5: высокий уровень - предупреждение
      unsigned HH: 		1;	//6: высокий уровень - авария
      unsigned type: 	1;	//7: тип сигнала: 0 - 4..20 мА, 1 - 0..10 В
    };
  };

//Дискретный вход
	//Состав драйвера
		class DRV_DI
		{
			public:
				void ee_control(int addr, bool update, bool first_load);
				void exe(int pin_DI);							//цикличная обработка
				void HMI();
				Byte_CMD CMD;									//слово команд		
				word comm_in;
				word comm_out;	
				int HMI_ID = 0;
			private:
				byte x = 0;
		};

	// Работа с памятью
	void DRV_DI::ee_control(int addr, bool update, bool first_load)
	{
		if (first_load == 1)
		{
			x = EEPROM.readByte(addr);
			CMD.Inv = bitRead(x, 4);
			bitWrite(comm_out, 4, CMD.Inv);
			comm_in = comm_out;
			HMI_Data[addr] = comm_in;
			HMI_ID = addr;
		}

		if (update == 1)
		{
			bitWrite(x, 4, CMD.Inv);
			EEPROM.updateByte(addr, x);
			HMI_ID = addr;
		}
	}

	// Работа с HMI
	void DRV_DI::HMI()
	{
		//чтение команд с HMI
			comm_in = HMI_Data[HMI_ID];
			if (bitRead(comm_in, 0) == 1)	{CMD.Hand = 1;} else {CMD.Hand = 0;}
			if (CMD.Hand == 1)	{CMD.Start_M = bitRead(comm_in, 2);} else {bitWrite(HMI_Data[HMI_ID], 2, CMD.Start_M);}

		//флаг редактирования свойств
			if (CMD.Hand == 1)	{CMD.Inv = bitRead(comm_in, 4);}

		//передача данных в HMI
			bitWrite(comm_out, 0, CMD.Hand);
			bitWrite(comm_out, 1, CMD.Start_A);
			bitWrite(comm_out, 2, CMD.Start_M);
			bitWrite(comm_out, 3, CMD.Use);
			bitWrite(comm_out, 4, CMD.Inv);
			bitWrite(comm_out, 5, CMD.Data);
			bitWrite(comm_out, 6, CMD.Status);
			bitWrite(comm_out, 7, CMD.CMD);
			HMI_Data[HMI_ID + 1] = comm_out;
	}

	//Чтение входа
		void DRV_DI::exe(int pin_DI)
		{	
			//автмоатический режим работы			
			if (CMD.Hand == 0)
			{
				//Чтение входа
				if (CMD.Use  == 0){if (pin_DI > 0) {CMD.Data = digitalRead(pin_DI);} else {CMD.Data = 0;}}				//чтение дискретного входа
				if (CMD.Use  == 1){if (pin_DI > 0){if (analogRead(pin_DI) > 500){CMD.Data = 1;} else {CMD.Data = 0;}}}	//чтение аналогового входа

				if (CMD.Data == 1)	{ if (CMD.Inv == 0){CMD.Status = 0; CMD.Start_M = 0;}else{CMD.Status = 1;CMD.Start_M = 1;}}
				if (CMD.Data == 0)	{ if (CMD.Inv == 0){CMD.Status = 1; CMD.Start_M = 1;}else{CMD.Status = 0;CMD.Start_M = 0;}}
			}
			//ручной режим работы
			else
			{
				if (CMD.Start_M == 0){CMD.Data = 0;CMD.Status = 0;} else {CMD.Data = 1;CMD.Status = 1;}
			}
		}

//Дискретный выход
	//Состав драйвера
		class DRV_DO
		{
			public:
				void ee_control(int addr, bool update, bool first_load);
				void exe(int pin_DO);							//цикличная обработка
				void HMI();
				Byte_CMD CMD;									//слово команд
				word comm_in;
				word comm_out;
				int HMI_ID = 0;
			private:
				byte x = 0;
		};

	// Работа с памятью
	void DRV_DO::ee_control(int addr, bool update, bool first_load)
	{
		if (first_load == 1)
		{
			x = EEPROM.readByte(addr);
			CMD.Inv = bitRead(x, 4);
			bitWrite(comm_out, 4, CMD.Inv);
			comm_in = comm_out;
			HMI_Data[addr] = comm_in;
			HMI_ID = addr;
		}

		if (update == 1)
		{
			bitWrite(x, 4, CMD.Inv);
			EEPROM.updateByte(addr, x);
			HMI_ID = addr;
		}
	}

	// Работа с HMI
	void DRV_DO::HMI()
	{
		//чтение команд с HMI
			comm_in = HMI_Data[HMI_ID];
			if (bitRead(comm_in, 0) == 1)	{CMD.Hand = 1;} else {CMD.Hand = 0;}
			if (CMD.Hand == 1)	{CMD.Start_M = bitRead(comm_in, 2);} else {bitWrite(HMI_Data[HMI_ID], 2, CMD.Start_M);}

		//флаг редактирования свойств
			if (CMD.Hand == 1)	{CMD.Inv = bitRead(comm_in, 4);}

		//передача данных в HMI
			bitWrite(comm_out, 0, CMD.Hand);
			bitWrite(comm_out, 1, CMD.Start_A);
			bitWrite(comm_out, 2, CMD.Start_M);
			bitWrite(comm_out, 3, CMD.Use);
			bitWrite(comm_out, 4, CMD.Inv);
			bitWrite(comm_out, 5, CMD.Data);
			bitWrite(comm_out, 6, CMD.Status);
			bitWrite(comm_out, 7, CMD.CMD);
			HMI_Data[HMI_ID + 1] = comm_out;
	}

	//Запись выхода
		void DRV_DO::exe(int pin_DO)
		{
			//управление в авторежиме
			if (CMD.Hand == 0)
			{
				CMD.Start_M = CMD.Start_A;
			}

			//управление выходом
			CMD.Status = CMD.Start_M;

			//инверсия
			if (CMD.Inv == 0)
			{
				if (CMD.Status == 1) {digitalWrite(pin_DO, HIGH);}
				if (CMD.Status == 0) {digitalWrite(pin_DO, LOW);}
			}
			else
			{
				if (CMD.Status == 1) {digitalWrite(pin_DO, LOW);}
				if (CMD.Status == 0) {digitalWrite(pin_DO, HIGH);}
			}
		}

//Аналоговый вход
	//Состав драйвера
		class DRV_AI
		{
			public:
				void ee_control(int addr, bool update, bool first_load);
				void exe(int F_Mode);						//цикличная обработка
				void HMI();
				int HMI_ID = 0;
				int pin_AI = 0;								//адрес входа
				float Result = 0.0;							//результат измерений
				float max_out = 100.0;						//диапазон входа от 0 до ...заданного значения
				Byte_CMD CMD;								//слово команд
				Byte_AI_alm cfg;							//слово аварий
				int Signal;									//входной сигнал
				float SP_LL = 5.0;
				float SP_L = 10.0;
				float SP_H = 85.0;
				float SP_HH = 95.0;
				word comm_in;
				word comm_out;
			private:
				#define SIZE_AI 	20
				#define AI_NC		180	//3.2 mA
				#define AI_SC		900	//21 mA
				float Value;
				float Summ;
				float Median;
				float DATA_AI[SIZE_AI];
				float x;
		};

	// Работа с памятью
	void DRV_AI::ee_control(int addr, bool update, bool first_load)
	{
		if (first_load == 1)
		{
			max_out = float (EEPROM.readInt(addr) * 10);
			SP_LL = float (EEPROM.readInt(addr + 2) * 10);
			SP_L = float (EEPROM.readInt(addr + 4) * 10);
			SP_H = float (EEPROM.readInt(addr + 6) * 10);
			SP_HH = float (EEPROM.readInt(addr + 8) * 10);
			HMI_ID = addr;
		}

		if (update == 1)
		{
			EEPROM.updateInt(addr, int(max_out / 10));
			EEPROM.updateInt(addr + 2, int(SP_LL / 10));
			EEPROM.updateInt(addr + 4, int(SP_L / 10));
			EEPROM.updateInt(addr + 6, int(SP_H / 10));
			EEPROM.updateInt(addr + 8, int(SP_HH / 10));
			HMI_ID = addr;
		}

		if (max_out == 0) {CMD.Use = 1;} else {CMD.Use = 0;}
		if (max_out < 0)  {CMD.Inv = 1;} else {CMD.Inv = 0;}
	}

	// Работа с HMI
	void DRV_AI::HMI()
	{
		//чтение команд с HMI
			comm_in = HMI_Data[HMI_ID];
			if (bitRead(comm_in, 0) == 1)	{CMD.Hand = 1;} else {CMD.Hand = 0;}
			if (CMD.Hand == 1)	{Result = float(HMI_Data[HMI_ID + 2]) / 10;} else {HMI_Data[HMI_ID + 2] = int (Result * 10);}

		//флаг редактирования свойств
			if (CMD.Hand == 1)
			{
				max_out = float(HMI_Data[HMI_ID + 3]) / 10;
				SP_LL = float(HMI_Data[HMI_ID + 4]) / 10;
				SP_L = float(HMI_Data[HMI_ID + 5]) / 10;
				SP_H = float(HMI_Data[HMI_ID + 6]) / 10;
				SP_HH = float(HMI_Data[HMI_ID + 7]) / 10;
			}
			else
			{
				HMI_Data[HMI_ID + 3] = int (max_out * 10);
				HMI_Data[HMI_ID + 4] = int (SP_LL * 10);
				HMI_Data[HMI_ID + 5] = int (SP_L * 10);
				HMI_Data[HMI_ID + 6] = int (SP_H * 10);
				HMI_Data[HMI_ID + 7] = int (SP_HH * 10);
			}

		//передача данных в HMI
			bitWrite(comm_out, 0, cfg.Alm);
			bitWrite(comm_out, 1, cfg.SC);
			bitWrite(comm_out, 2, cfg.NC);
			bitWrite(comm_out, 3, cfg.LL);
			bitWrite(comm_out, 4, cfg.L);
			bitWrite(comm_out, 5, cfg.H);
			bitWrite(comm_out, 6, cfg.HH);
			bitWrite(comm_out, 7, cfg.type);
			HMI_Data[HMI_ID + 1] = comm_out;
	}

	//Обработка сигнала
		void DRV_AI::exe(int F_Mode)
		{
			// Чтение сигнала
			if (pin_AI > 0) {Signal = analogRead(pin_AI);}
			
			//контроль обрыва линии и перегрузки
			if (cfg.type == 0)
			{
				//if (Signal < AI_NC)	{cfg.NC = 1; pulse.Any_Alarm = 1;}
				//if (Signal > AI_SC) {cfg.SC = 1; pulse.Any_Alarm = 1;}
				//масштабирование сигнала
				Value = (Signal - 204) / ((850 - 204) / (max_out - 0)) + 0;	//204 - значение входа при 4 мА, 850 - значение входа при 20 мА
			}
			else
			{
				//масштабирование сигнала
				max_out = fabs(max_out);
				Value = (Signal - 0) / ((1024 - 0) / (max_out - 0)) + 0;	//0 - значение входа при 0 V, 1024 - значение входа при 10 V
			}
			

			if (Value < 0)	{Value = 0;}
			if (Value > max_out)	{Value = max_out;}
			if (CMD.Inv == 1 ) {Value = max_out - Value;}

			//автоматиечкий режим
			if (CMD.Hand == 0)
			{
				//Режим без фильтрации
				if (F_Mode <= 0 or F_Mode > 2)
				{
					Result = Value;
				}

				if (F_Mode == 1)
				{
					DATA_AI[0] = Value;
					Summ = 0.0;
					for(int i=SIZE_AI; i>0; i--)
					{
						DATA_AI[i] = DATA_AI[i - 1];
						Summ += DATA_AI[i];    
					}
					Median = Summ / SIZE_AI;	
					Result = Median;
				}

				if (F_Mode == 2 and pulse.s01 == 1)
				{
					DATA_AI[0] = Value;
					Summ = 0.0;
					for(int i=SIZE_AI; i>0; i--)
					{
						DATA_AI[i] = DATA_AI[i - 1];
						Summ += DATA_AI[i];    
					}
					Median = Summ / SIZE_AI;	
					Result = Median;				
				}
			}

			//аварии и предупреждения
			if (CMD.Use == 1)
			{
				//if (Result < SP_LL)	{cfg.LL = 1; pulse.Any_Alarm = 1;}
				if (Result < SP_L)	{cfg.L = 1;} else {cfg.L = 0;}
				if (Result > SP_H) 	{cfg.H = 1;} else {cfg.H = 0;}
				//if (Result > SP_HH) {cfg.HH = 1; pulse.Any_Alarm = 1;}

				//формирование общей аварии
				if (cfg.NC == 1 or cfg.SC == 1 or cfg. LL == 1 or cfg.HH == 1)	{pulse.Any_Alarm = 1;}
			}
			
			// сброс аварий
			if (pulse.Reset_All == 1)
			{
				cfg.NC = 0;
				cfg.SC = 0;
				cfg.LL = 0;
				cfg.HH = 0;
			}
		}

//Аналоговый выход
	//Состав драйвера
		class DRV_AO
		{
			public:			
				void ee_control(int addr, bool update, bool first_load);	
				void exe();									//циклиная обработка
				void HMI();
				int pin_AO = 0;								//адрес выхода
				float SP_A = 0.0;							//уставка в аторежиме
				float SP_M = 0.0;							//уставка в ручном режиме
				float max_out = 100;						//выходной сигнал от 0 до ...заданного значения
				Byte_CMD CMD;								//слово команд
				int Signal;									//выходной сигнал
				word comm_in;
				word comm_out;
				int HMI_ID = 0;
				int Mode = 0;								//0 - 4..20, 1 - 0..10
			private:
				float x;
		};

	// Работа с памятью
	void DRV_AO::ee_control(int addr, bool update, bool first_load)
	{
		if (first_load == 1)
		{
			max_out = float (EEPROM.readInt(addr) * 10);
			HMI_ID = addr;
		}

		if (update == 1)
		{
			EEPROM.updateInt(addr, int(max_out / 10));
			HMI_ID = addr;
		}

		if (max_out == 0) {CMD.Use = 1;} else {CMD.Use = 0;}
		if (max_out < 0)  {CMD.Inv = 1;} else {CMD.Inv = 0;}
	}

	// Работа с HMI
	void DRV_AO::HMI()
	{
		//чтение команд с HMI
			comm_in = HMI_Data[HMI_ID];
			if (bitRead(comm_in, 0) == 1)	{CMD.Hand = 1;} else {CMD.Hand = 0;}
			if (CMD.Hand == 1)	{SP_M = float(HMI_Data[HMI_ID + 1]) / 10;} else {HMI_Data[HMI_ID + 1] = int (SP_M * 10);}

		//флаг редактирования свойств
			if (CMD.Hand == 1)
			{
				max_out = float(HMI_Data[HMI_ID + 2]) / 10;
			}
			else
			{
				HMI_Data[HMI_ID + 2] = int (max_out * 10);
			}
	}

	//Обработка сигнала
		void DRV_AO::exe()
		{
			if (CMD.Hand == 0)
			{
				SP_M = SP_A;
			}
			if (SP_M > max_out) {SP_M = max_out;}
			
			max_out = fabs(max_out);
			if (CMD.Inv == 1) {x = max_out - SP_M;} else {x = SP_M;}
			if (Mode == 0)
			{
				Signal = (x - 0) / ((max_out - 0) / (255 - 53)) + 53;	//53 - значение выхода при 4 мА, 255 - значение выхода при 20 мА
				if (Signal < 53)	{Signal = 53;}
			}
			if (Mode == 1)
			{
				Signal = (x - 0) / ((max_out - 0) / (255 - 0)) + 0;	//53 - значение выхода при 4 мА, 255 - значение выхода при 20 мА
				if (Signal < 0)	{Signal = 0;}
			}
			if (Signal > 255)	{Signal = 255;}

			if (pin_AO > 0) {analogWrite(pin_AO, Signal);}
		}