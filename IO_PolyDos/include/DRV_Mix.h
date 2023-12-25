//специальные типы данных 
  	union Byte_Mix { 
	    unsigned int value;
	    struct {
	      unsigned Mix_Use: 	1;	//0
	      unsigned Mix_Start:	1;	//1
	      unsigned Mix_Sel:  	1;	//2
	      unsigned Mix_LLS:		1;	//3
	      unsigned Mix_HLS:		1;	//4
		  unsigned Mix_CMD:		1;	//5
		  unsigned Mix_Lock_LS:	1;	//6
		  unsigned Mix_Lock_HS:	1;	//7
	    };
	  };

// МЕШАЛКИ ДЛЯ ПРИГОТОВЛЕНИЯ
	class DRV_Mix
	{
		public:
			void ee_control(int addr, bool update, bool first_load);
			bool exe(bool LLS, bool HLS);
			void HMI();
			Byte_Mix CMD;
			int HMI_ID;
			int ID;
			int shift;
			word comm_in;
			word comm_out;
			int SP_ON = 5;
			int SP_OFF = 10;
		private:
			TON_m T_ON;
			TON_m T_OFF;
			TON_s T_Delay;
			int next_addr = 0;
			bool Lock;
			word tmp;
	};
	
// Работа с памятью
	void DRV_Mix::ee_control(int addr, bool update, bool first_load)
	{
		if (first_load == 1)
		{
			HMI_ID = addr;
			next_addr = addr + 1100 + shift * 10;
			// управление
			comm_out = EEPROM.readInt(next_addr);
			CMD.Mix_Use = bitRead(comm_out, 0);
			CMD.Mix_Start = bitRead(comm_out, 1);
			CMD.Mix_Sel = bitRead(comm_out, 2);
			CMD.Mix_LLS = bitRead(comm_out,3);
			CMD.Mix_HLS = bitRead(comm_out, 4);
			bitWrite(comm_in, 0, CMD.Mix_Use);
			bitWrite(comm_in, 1, CMD.Mix_Start);
			bitWrite(comm_in, 2, CMD.Mix_Sel);
			bitWrite(comm_in, 3, CMD.Mix_LLS);
			bitWrite(comm_in, 4, CMD.Mix_HLS);
			comm_out = comm_in;
			next_addr = next_addr + 2;
			SP_ON = EEPROM.readInt(next_addr);
			next_addr = next_addr + 2;
			SP_OFF = EEPROM.readInt(next_addr);
			comm_in = comm_out;
			HMI_Data[HMI_ID] = comm_in;
			HMI_Data[HMI_ID + 1] = comm_out;
			HMI_Data[HMI_ID + 2] = SP_ON;
			HMI_Data[HMI_ID + 3] = SP_OFF;
			T_Delay.tSP = 30;
			Lock = 0;
		}

		if (update == 1)
		{
			HMI_ID = addr;
			next_addr = addr + 1100 + shift * 10;
			// управление
			bitWrite(tmp, 0, CMD.Mix_Use);
			bitWrite(tmp, 1, CMD.Mix_Start);
			bitWrite(tmp, 2, CMD.Mix_Sel);
			bitWrite(tmp, 3, CMD.Mix_LLS);
			bitWrite(tmp, 4, CMD.Mix_HLS);
			bitWrite(comm_in, 0, CMD.Mix_Use);
			bitWrite(comm_in, 1, CMD.Mix_Start);
			bitWrite(comm_in, 2, CMD.Mix_Sel);
			bitWrite(comm_in, 3, CMD.Mix_LLS);
			bitWrite(comm_in, 4, CMD.Mix_HLS);
			EEPROM.updateInt(next_addr, tmp);
			comm_out = comm_in;
			HMI_Data[HMI_ID] = comm_in;
			HMI_Data[HMI_ID + 1] = comm_out;
			next_addr = next_addr + 2;
			EEPROM.updateInt(next_addr, SP_ON);
			next_addr = next_addr + 2;
			EEPROM.updateInt(next_addr, SP_OFF);
			HMI_Data[HMI_ID + 2] = SP_ON;
			HMI_Data[HMI_ID + 3] = SP_OFF;
			T_Delay.tSP = 30;
			Lock = 0;
		}
	}

// Работа с панелью
	void DRV_Mix::HMI()
	{
		// чтение команд с HMI
		comm_in = HMI_Data[HMI_ID];			// входные команды с панели
		CMD.Mix_Use = bitRead(comm_in, 0);
		CMD.Mix_Start = bitRead(comm_in, 1);
		CMD.Mix_Sel = bitRead(comm_in, 2);
		CMD.Mix_LLS = bitRead(comm_in, 3);
		CMD.Mix_HLS = bitRead(comm_in, 4);
		
		bitWrite(comm_out, 0, CMD.Mix_Use);
		bitWrite(comm_out, 1, CMD.Mix_Start);
		bitWrite(comm_out, 2, CMD.Mix_Sel);
		bitWrite(comm_out, 3, CMD.Mix_LLS);
		bitWrite(comm_out, 4, CMD.Mix_HLS);
		bitWrite(comm_out, 5, CMD.Mix_CMD);
		bitWrite(comm_out, 6, CMD.Mix_Lock_LS);
		bitWrite(comm_out, 7, CMD.Mix_Lock_HS);
		HMI_Data[HMI_ID + 1] = comm_out;	// состояние программы
		SP_ON = HMI_Data[HMI_ID + 2];
		SP_OFF = HMI_Data[HMI_ID + 3];
		T_ON.tSP = SP_ON;
		T_OFF.tSP = SP_OFF;
	}
	
// Управление мешалкой
	bool DRV_Mix::exe(bool LLS, bool HLS)
	{
		// контроль уровней - низкий уровень
		if (CMD.Mix_LLS == 1)
		{
			if (LLS == 1) {CMD.Mix_Lock_LS = 1;}
			if (LLS == 0) {CMD.Mix_Lock_LS = 0;}
		}
		else
		{
			if (pulse.Reset_All == 1) {CMD.Mix_Lock_LS = 0;}
		}
		// контроль уровней - высокий уровень
		if (CMD.Mix_HLS == 1)
		{
			if (HLS == 1) {CMD.Mix_Lock_HS = 1;}
			if (HLS == 0) {CMD.Mix_Lock_HS = 0;}
		}
		else
		{
			if (pulse.Reset_All == 1) {CMD.Mix_Lock_HS = 0;}
		}

		// мешалка не используется
		if (CMD.Mix_Use == 0)
		{
			T_ON.CMD.Start = 0;
			T_OFF.CMD.Start = 0;
			T_Delay.CMD.Start = 0;
			CMD.Mix_CMD = 0;
			Lock = 0;
		}
		// мешалка используется
		if (CMD.Mix_Use == 1)
		{
			if (CMD.Mix_Start == 1)
			{
				// включить мешалку - работа по циклу
				if (LLS == 0 and Lock == 0)
				{
					T_Delay.CMD.Start = 0;
					if (T_ON.CMD.Exit == 0) 	{T_ON.CMD.Start = 1; T_OFF.CMD.Start = 0; CMD.Mix_CMD = 1;}
					if (T_ON.CMD.Exit == 1) 	{T_ON.CMD.Start = 1; T_OFF.CMD.Start = 1; CMD.Mix_CMD = 0;}
					if (T_OFF.CMD.Exit == 1) 	{T_ON.CMD.Start = 0; T_OFF.CMD.Start = 0; CMD.Mix_CMD = 0;}
				}
				// отключить мешалку - защита по низкому уровню
				if (LLS == 1)
				{
					T_ON.CMD.Start = 0;
					T_OFF.CMD.Start = 0;
					T_Delay.CMD.Start = 1;
					CMD.Mix_CMD = 0;
					Lock = 1;
				}
				// разблокировка по низкому уровню
				if (T_Delay.CMD.Exit == 1) {Lock = 0;}
			}
			else
			{
				T_ON.CMD.Start = 0;
				T_OFF.CMD.Start = 0;
				T_Delay.CMD.Start = 0;
				CMD.Mix_CMD = 0;
				Lock = 0;
			}
		}

		// управление мешалкой
		return CMD.Mix_CMD;
	}