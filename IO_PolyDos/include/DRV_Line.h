//специальные типы данных 
  	union Byte_Line { 
	    unsigned int value;
	    struct {
	      unsigned CMD_KC: 		1;	//0
	      unsigned CMD_Main: 	1;	//1
	      unsigned CMD_Reserv: 	1;	//2
	      unsigned Alarm_Main: 	1;	//3
	      unsigned Alarm_Reserv:1;	//4
	      unsigned Hand:	 	1;	//5
	      unsigned Start: 		1;	//6
	      unsigned Mode:		1;	//7
		  unsigned Use_RT:		1;	//8
		  unsigned Use_RM:		1;	//9
		  unsigned Ready:		1;	//10
		  unsigned FB_RT:		1;	//11
		  unsigned Use_KC:		1;	//12
		  unsigned save:		1;	//13
		  unsigned Alarm_RT:	1;	//14
		  unsigned Alarm_RM:	1;	//15
	    };
	  };


//ДРАЙВЕР ТИПОВОЙ ЛИНИИ
	//Состав драйвера
		class DRV_Line
		{
			public:
				void ee_control(int addr, bool update, bool first_load);
				void exe();
				void HMI();
				Byte_Line CMD;
				//параметры
				float K = 0.01;
				float Ti = 10.0;
				float PID_SP = 1.0;
				float D = 10.0;
				float Flow = 100.0;
				float SP_A = 0.0;
				float SP_M = 0.0;
				float SP_D = 0.0;
				float xSP = 0.0;
				int Status = 0;			//состояние
				int HMI_ID;
				int shift = 0;
				int next_addr = 0;
				word comm_in;
				word comm_out;
				TON_s T_RT_ON;
				TON_s T_RT_OFF;
				TON_m T_RM_UP;
				TON_m T_RM_DOWN;
				int ID;
			private:
				word x = 0;
				float perc_now = 0.0;
				float perc_min = 0.75;
				float perc_max = 1.25;
		};

	// Работа с памятью
	void DRV_Line::ee_control(int addr, bool update, bool first_load)
	{
		if (first_load == 1)
		{
			// чтение из EEPROM
			HMI_ID = addr * 2;
			next_addr = HMI_ID + shift * 50;
			// управление
			comm_out = EEPROM.readInt(next_addr);
			CMD.Hand = bitRead(comm_out, 5);
			CMD.Start = bitRead(comm_out, 6);
			CMD.Mode = bitRead(comm_out, 7);
			CMD.Use_RT = bitRead(comm_out, 8);
			CMD.Use_RM = bitRead(comm_out, 9);
			CMD.Use_KC = bitRead(comm_out, 12);
			CMD.save = bitRead(comm_out, 13);
			bitWrite(comm_in, 5, CMD.Hand);
			bitWrite(comm_in, 6, CMD.Start);
			bitWrite(comm_in, 7, CMD.Mode);
			bitWrite(comm_in, 8, CMD.Use_RT);
			bitWrite(comm_in, 9, CMD.Use_RM);
			bitWrite(comm_in, 12, CMD.Use_KC);
			bitWrite(comm_in, 13, CMD.save);
			comm_out = comm_in;
			HMI_Data[HMI_ID] = comm_in;
			HMI_Data[HMI_ID + 1] = comm_out;
			next_addr = next_addr + 2;
			// доза
			D = EEPROM.readFloat(next_addr);
			HMI_Data[HMI_ID + 2] = uint16_t(D * 100);	//максимальная доза 600 мг/л
			next_addr = next_addr + 4;
			// магистральный расход
			if (CMD.Mode == 1)
			{
				Flow = EEPROM.readFloat(next_addr);
				HMI_Data[HMI_ID + 3] = uint16_t(Flow * 10);	//максимальный магистральный расход 6000 м3/ч
			}
			next_addr = next_addr + 4;
			// ручное задание расхода
			SP_M = EEPROM.readFloat(next_addr);
			HMI_Data[HMI_ID + 4] = uint16_t(SP_M * 10);	//максимальный расход НД 6000 л/ч
			next_addr = next_addr + 4;
			// ПИД - коэфф пропорциональности
			K = EEPROM.readFloat(next_addr);
			HMI_Data[HMI_ID + 5] = uint16_t(K * 100);
			next_addr = next_addr + 4;
			// ПИД - время интеграции
			Ti = EEPROM.readFloat(next_addr);
			HMI_Data[HMI_ID + 6] = uint16_t(Ti);
		}

		if (update == 1)
		{
			HMI_ID = addr * 2;
			next_addr = HMI_ID + shift * 50;
			// управление
			CMD.save = CMD.Start;
			bitWrite(x, 5, CMD.Hand);
			bitWrite(x, 6, CMD.Start);
			bitWrite(x, 7, CMD.Mode);
			bitWrite(x, 8, CMD.Use_RT);
			bitWrite(x, 9, CMD.Use_RM);
			bitWrite(x, 12, CMD.Use_KC);
			bitWrite(x, 13, CMD.save);
			bitWrite(comm_in, 5, CMD.Hand);
			bitWrite(comm_in, 6, CMD.Start);
			bitWrite(comm_in, 7, CMD.Mode);
			bitWrite(comm_in, 8, CMD.Use_RT);
			bitWrite(comm_in, 9, CMD.Use_RM);
			bitWrite(comm_in, 12, CMD.Use_KC);
			bitWrite(comm_in, 13, CMD.save);
			EEPROM.updateInt(next_addr, x);
			comm_out = comm_in;
			HMI_Data[HMI_ID] = comm_in;
			HMI_Data[HMI_ID + 1] = comm_out;
			next_addr = next_addr + 2;
			// доза
			EEPROM.updateFloat(next_addr, D);
			HMI_Data[HMI_ID + 2] = uint16_t(D * 100);
			next_addr = next_addr + 4;
			// магистральный расход
			if (CMD.Mode == 1)
			{
				EEPROM.updateFloat(next_addr, Flow);
				HMI_Data[HMI_ID + 3] = uint16_t(Flow * 10);	//максимальный магистральный расход 6000 м3/ч
			}
			next_addr = next_addr + 4;
			// ручное задание расхода
			if (CMD.Hand == 1)
			{
				EEPROM.updateFloat(next_addr, SP_M);
				HMI_Data[HMI_ID + 4] = uint16_t(SP_M * 10);	//максимальный расход НД 600 л/ч
			}
			next_addr = next_addr + 4;
			// ПИД - коэфф пропорциональности
			EEPROM.updateFloat(next_addr, K);
			HMI_Data[HMI_ID + 5] = uint16_t(K * 100);
			next_addr = next_addr + 4;
			// ПИД - время интеграции
			EEPROM.updateFloat(next_addr, Ti);
			HMI_Data[HMI_ID + 6] = uint16_t(Ti);
		}
	}

	// Работа с панелью
		void DRV_Line::HMI()
		{
			// управление
			comm_in = HMI_Data[HMI_ID];
			CMD.Hand = 		bitRead(comm_in, 5);
			CMD.Start = 	bitRead(comm_in, 6);
			CMD.Mode = 		bitRead(comm_in, 7);
			CMD.Use_RT = 	bitRead(comm_in, 8);
			CMD.Use_RM =	bitRead(comm_in, 9);
			CMD.Use_KC =	bitRead(comm_in, 12);
			// индикация
			bitWrite(comm_out, 0, CMD.CMD_KC);
			bitWrite(comm_out, 1, CMD.CMD_Main);
			bitWrite(comm_out, 2, CMD.CMD_Reserv);
			bitWrite(comm_out, 3, CMD.Alarm_Main);
			bitWrite(comm_out, 4, CMD.Alarm_Reserv);
			bitWrite(comm_out, 5, CMD.Hand);
			bitWrite(comm_out, 6, CMD.Start);
			bitWrite(comm_out, 7, CMD.Mode);
			bitWrite(comm_out, 8, CMD.Use_RT);
			bitWrite(comm_out, 9, CMD.Use_RM);
			bitWrite(comm_out, 10, CMD.Ready);
			bitWrite(comm_out, 11, CMD.FB_RT);
			bitWrite(comm_out, 12, CMD.Use_KC);
			bitWrite(comm_out, 13, CMD.save);
			HMI_Data[HMI_ID + 1] = comm_out;
			// доза
			D = float(uint16_t(HMI_Data[HMI_ID + 2])) / 100;
			// магистральный расхода
			if (mode_rs_sp == 0)
			{
				// задание с панели
				Flow = float(uint16_t(HMI_Data[HMI_ID + 3])) / 10;
			}
			if (mode_rs_sp == 1)
			{
				// задание по сети
				HMI_Data[HMI_ID + 3] = int(Flow * 10);
			}
			// ручное задание расхода
			if (CMD.Hand == 0) {HMI_Data[HMI_ID + 4] = uint16_t(SP_M * 10);}
			if (CMD.Hand == 1) {SP_M = float(HMI_Data[HMI_ID + 4]) / 10;}
			// ПИД - коэфф пропорциональности
			K = float(uint16_t(HMI_Data[HMI_ID + 5])) / 100;
			// ПИД - время интеграции
			Ti = float(uint16_t(HMI_Data[HMI_ID + 6]));
			// статус
			HMI_Data[HMI_ID + 7] = Status;
		}

	// управление линией
		void DRV_Line::exe()
		{
			//Обработка статуса
				if (CMD.Start == 1 and CMD.Ready == 0)	{ Status = 2;}
				if (CMD.Start == 1 and CMD.Ready == 1)	{ Status = 1;}
				if (CMD.Start == 0)						{ Status = 0;}

			//Клапан разбавления
				if (Status == 1)
				{
					if (CMD.Use_KC == 1) {CMD.CMD_KC = 1;}
				}
				else
				{
					if (CMD.Use_KC == 1) {CMD.CMD_KC = 0;}
				}
				if (CMD.Use_KC == 0) {CMD.CMD_KC = 0;}
			
			// контроль ротаметра
				if (CMD.Use_KC == 1 and CMD.CMD_KC == 1)
				{
					if (CMD.FB_RT == 0) {T_RT_ON.CMD.Start = 1;} else {T_RT_ON.CMD.Start = 0;}
					T_RT_OFF.CMD.Start = 0;
					if (T_RT_ON.CMD.Exit == 1) {CMD.Alarm_RT = 1;}
					if (pulse.Reset_All == 1) {T_RT_ON.CMD.Reset = 1; CMD.Alarm_RT = 0;}
				}
				if (CMD.Use_KC == 1 and CMD.CMD_KC == 0)
				{
					T_RT_ON.CMD.Start = 0;
					if (CMD.FB_RT == 1) {T_RT_OFF.CMD.Start = 1;} else {T_RT_OFF.CMD.Start = 0;}
					if (T_RT_OFF.CMD.Exit == 1) {CMD.Alarm_RT = 1;}
					if (pulse.Reset_All == 1) {T_RT_OFF.CMD.Reset = 1; CMD.Alarm_RT = 0;}
				}
				if (CMD.Use_KC == 0)
				{
					T_RT_ON.CMD.Start = 0;
					T_RT_OFF.CMD.Start = 0;
				}

			// контроль расхода
				if (CMD.Use_RM == 1 and (CMD.CMD_Main == 1 or CMD.CMD_Reserv == 1))
				{
					perc_now = SP_M / IN_Analog[ID].Result;
					if (perc_now < perc_min)
					{
						T_RM_UP.CMD.Start = 0;
						T_RM_DOWN.CMD.Start = 1;
						if (T_RM_DOWN.CMD.Exit == 1) {CMD.Alarm_RM = 1;}
						if (pulse.Reset_All == 1) {T_RM_DOWN.CMD.Reset = 1; CMD.Alarm_RM = 0;}
					}
					if (perc_now > perc_max)
					{
						T_RM_UP.CMD.Start = 1;
						T_RM_DOWN.CMD.Start = 0;
						if (T_RM_UP.CMD.Exit == 1) {CMD.Alarm_RM = 1;}
						if (pulse.Reset_All == 1) {T_RM_UP.CMD.Reset = 1; CMD.Alarm_RM = 0;}
					}
					if (perc_now > perc_min and perc_now < perc_max)
					{
						T_RM_UP.CMD.Start = 0;
						T_RM_DOWN.CMD.Start = 0;
					}
				}
				else
				{
					T_RM_UP.CMD.Start = 0;
					T_RM_DOWN.CMD.Start = 0;
				}

			// Рассчёт расхода для насоса
				if (CMD.Use_RM == 0) {PID_SP = 1.0;}
				if (C >= 0.01) {SP_D = (Flow * D) / (C * 10.0);} else {SP_D =0;}
				if (CMD.Hand == 0)
				{
					if (Flow > 0 and CMD.Mode == 0)	{SP_A = SP_D;}	else { SP_A = 0;}
					if (Flow > 0 and CMD.Mode == 1) {SP_A = Flow;}
					if (Flow == 0 or D == 0 or C == 0) {SP_A = 0;}
					SP_M = SP_A;
				}
				if (CMD.Use_RM == 0) {PID_SP = 1.0;}
				xSP = SP_M * PID_SP;

			//Выбор насоса
				if (Status == 1 and xSP > 0)
				{
					if (CMD.Alarm_Main == 0) {CMD.CMD_Main = 1; CMD.CMD_Reserv = 0;}
					if (CMD.Alarm_Main == 1) {CMD.CMD_Main = 0; CMD.CMD_Reserv = 1;}
					if (CMD.Alarm_Reserv == 1){CMD.CMD_Reserv = 0;}
				}
				else
				{
					CMD.CMD_Main = 0;
					CMD.CMD_Reserv = 0;
				}

			// обнуление флагов
				CMD.Alarm_Main = 0;
				CMD.Alarm_Reserv = 0;

			// обработка таймеров
				T_RT_ON.exe(pulse.s01);
				T_RT_OFF.exe(pulse.s01);
				T_RM_UP.exe(pulse.s1);
				T_RM_DOWN.exe(pulse.s1);

			// сбор аварий аварий
				if (CMD.Alarm_RM or CMD.Alarm_RT) {pulse.Any_Alarm = 1;}
				if (pulse.Reset_All == 1) {CMD.Alarm_RT = 0; CMD.Alarm_RM = 0;}
		}