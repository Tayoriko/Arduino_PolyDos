#include <Arduino.h>

// генератор импульсов
	//Состав функции
		class drv_pulse
		{
			public:
				void exe();
				void init();
				void end();
				void v2();
				void v3();
				void v4();
				bool Reset_All;
				bool Any_Alarm;
				bool s001;
				bool s01;
				bool s1;
				int DIx[25];
    			int DOx[25];
				int AIx[9];
				int AOx[5];
			private:
				unsigned long timeSec;
				unsigned long timer001s;
				unsigned long timer01s;
				unsigned long timer1s;
		};

	// конфигурация платы версии 2
		void drv_pulse::v2() 
		{
			DIx[1] = 43;
			DIx[2] = 45;
			DIx[3] = 46;
			DIx[4] = 47;
			DIx[5] = 48;
			DIx[6] = 49;
			DIx[7] = 15;
			DIx[8] = 14;
			DIx[9] = 3;
			DIx[10] = 2;
			DIx[11] = 4;
			DIx[12] = 4;
			DIx[13] = 15;
			DIx[14] = 14;
			DIx[15] = 13;
			DIx[16] = 41;
			DIx[17] = 40;
			DIx[18] = 38;
			DIx[19] = 39;
			DIx[20] = 17;

			DOx[1] = 36;
			DOx[2] = 13;
			DOx[3] = 12;
			DOx[4] = 11;
			DOx[5] = 10;
			DOx[6] = 30;
			DOx[7] = 31;
			DOx[8] = 32;
			DOx[9] = 33;
			DOx[10] = 34;
			DOx[11] = 35;
			DOx[12] = 25;
			DOx[13] = 26;
			DOx[14] = 27;
			DOx[15] = 2;
			DOx[16] = 29;
			DOx[17] = 37;
			DOx[18] = 22;
			DOx[19] = 23;
			DOx[20] = 24;

			for (int i=1; i <= 20; i++)	{	pinMode(DIx[i], INPUT);	}
			for (int i=1; i <= 20; i++)	{	pinMode(DOx[i], OUTPUT);}

			AIx[1] = 7;
			AIx[2] = 6;
			AIx[3] = 5;
			AIx[4] = 4;
			AIx[5] = 3;
			AIx[6] = 2;
			AIx[7] = 1;
			AIx[8] = 0;

			AOx[1] = 6;
			AOx[2] = 7;
			AOx[3] = 8;
			AOx[4] = 9;
		}

	// конфигурации платы версии 3
		void drv_pulse::v3() 
		{
			DIx[1] = 3;
			DIx[2] = 2;
			DIx[3] = 5;
			DIx[4] = 4;
			DIx[5] = 48;
			DIx[6] = 49;
			DIx[7] = 15;
			DIx[8] = 14;
			DIx[9] = 43;
			DIx[10] = 45;
			DIx[11] = 46;
			DIx[12] = 47;
			DIx[13] = 16;
			DIx[14] = 20;
			DIx[15] = 21;
			DIx[16] = 45;
			DIx[17] = 40;
			DIx[18] = 38;
			DIx[19] = 39;
			DIx[20] = 17;

			DOx[1] = 30;
			DOx[2] = 29;
			DOx[3] = 28;
			DOx[4] = 27;
			DOx[5] = 26;
			DOx[6] = 25;
			DOx[7] = 24;
			DOx[8] = 23;
			DOx[9] = 22;
			DOx[10] = 37;
			DOx[11] = 10;
			DOx[12] = 11;
			DOx[13] = 12;
			DOx[14] = 13;
			DOx[15] = 36;
			DOx[16] = 35;
			DOx[17] = 34;
			DOx[18] = 33;
			DOx[19] = 32;
			DOx[20] = 31;

			for (int i=1; i <= 20; i++)	{	pinMode(DIx[i], INPUT);	}
			for (int i=1; i <= 20; i++)	{	pinMode(DOx[i], OUTPUT);}

			AIx[1] = 7;
			AIx[2] = 6;
			AIx[3] = 5;
			AIx[4] = 4;
			AIx[5] = 3;
			AIx[6] = 2;
			AIx[7] = 1;
			AIx[8] = 0;

			AOx[1] = 6;
			AOx[2] = 7;
			AOx[3] = 8;
			AOx[4] = 9;
		}
	
	// конфигурация платы версии 4
		void drv_pulse::v4()
		{
			DIx[1] = 48;//3;
			DIx[2] = 49;//2;
			DIx[3] = 15;//5;
			DIx[4] = 14;//4;
			DIx[5] = 3;//48;
			DIx[6] = 2;//49;
			DIx[7] = 5;//15;
			DIx[8] = 4;//14;
			DIx[9] = 16;//43;
			DIx[10] = 12;//A//45;
			DIx[11] = 11;//A//46;
			DIx[12] = 42;//47;
			DIx[13] = 43;//16;
			DIx[14] = 45;//20;
			DIx[15] = 46;//21;
			DIx[16] = 47;//45;
			DIx[17] = 15;//A//40;
			DIx[18] = 14;//A//38;
			DIx[19] = 13;//A//39;
			DIx[20] = 41;//17;
			DIx[21] = 40;//
			DIx[22] = 38;//
			DIx[23] = 39;//
			DIx[24] = 17;//

			DOx[1] = 30;//30;
			DOx[2] = 29;//29;
			DOx[3] = 28;//28;
			DOx[4] = 27;//27;
			DOx[5] = 26;//26;
			DOx[6] = 25;//25;
			DOx[7] = 24;//24;
			DOx[8] = 23;//23;
			DOx[9] = 22;//22;
			DOx[10] = 37;//37;
			DOx[11] = 31;//10;
			DOx[12] = 32;//11;
			DOx[13] = 33;//12;
			DOx[14] = 34;//13;
			DOx[15] = 35;//36;
			DOx[16] = 36;//35;
			DOx[17] = 13;//34;
			DOx[18] = 12;//33;
			DOx[19] = 11;//32;
			DOx[20] = 10;//31;

			for (int i=1; i <= 24; i++)	{	pinMode(DIx[i], INPUT);	}
			for (int i=1; i <= 20; i++)	{	pinMode(DOx[i], OUTPUT);}
		}

	// инициализация
		void drv_pulse::init() 
		{	
			// инциализация таймеров
			unsigned long uptimeSec = millis() / 100;
			timeSec  = uptimeSec;
			timer001s  = uptimeSec;
			timer01s  = uptimeSec;
			timer1s = uptimeSec;

			// инициализация дискретных входов/выходов
			switch (version)
			{
				case 2:
					v2();
					break;
				case 3:
					v3();
					break;
				case 4:
					v4();
					break;
				default:
					break;
			}
		}

	//обработка в цикле
		void drv_pulse::exe()
		{
			timeSec = millis() / 10;
			if (timeSec < timer001s)
			{
				timer001s = timeSec;
			}
			if (timeSec - timer001s  >=  1)  
			{
				timer001s  = timeSec; s001 = 1;
			}
			if (timeSec < timer01s)
			{
				timer01s = timeSec;
			}
			if (timeSec - timer01s  >=  10)  
			{
				timer01s  = timeSec; s01 = 1;
			}
			if (timeSec < timer1s)
			{
				timer1s = timeSec;
			}
			if (timeSec - timer1s  >=  100)  
			{
				timer1s  = timeSec; s1 = 1;
			}
		}
	
	// сброс значений
		void drv_pulse::end()
		{
			s001 = 0;
			s01 = 0;
			s1 = 0;
			if (Reset_All == 1) {Any_Alarm = 0;}
			Reset_All = 0;
		}

//специальные типы данных 
  union Byte_TON {
    unsigned char value;
    struct {
      unsigned Start: 	1;
      unsigned Exit: 	1;
      unsigned Pause: 	1;
      unsigned Reset: 	1;
      unsigned res2: 	1;
      unsigned res3: 	1;
      unsigned res4: 	1;
      unsigned res5: 	1;
    };
  };

//Секундный таймер
	//Состав таймера
		class TON_s
		{
			public:
				void exe(int clock);
				Byte_TON CMD;
				float tSP = 2;
				float percent = 0;
			private:
				float NOW = 0;
		};

	//Выполнение таймера
		void TON_s::exe(int clock)
		{
			if (CMD.Start == 0)	{NOW = 0; CMD.Exit = 0; percent = 0.0;}
			if (CMD.Start == 1)
			{
				if (NOW < tSP)
					{
						if (clock == 1)	{NOW = NOW + 0.1;}
						CMD.Exit = 0;
						percent = NOW / tSP;
					}
				if (NOW >= tSP) {CMD.Exit = 1; percent = 1.0;}
			}
			if (CMD.Reset == 1) {NOW = 0.0; CMD.Reset = 0; percent = 0.0;}
		}

//Минутный таймер
	//Состав таймера
		class TON_m
		{
			public:
				void exe(int clock);
				Byte_TON CMD;
				float tSP = 1;
			private:
				float NOW = 0;
				float SP_m = 0;
		};

	//Выполнение таймера
		void TON_m::exe(int clock)
		{
			SP_m = tSP * 60.0;
			if (CMD.Start == 0)	{NOW = 0; CMD.Exit = 0;}
			if (CMD.Start == 1 and CMD.Pause == 0)
			{
				if (NOW < SP_m)
					{
						if (clock == 1)	{NOW = NOW + 1;}
						CMD.Exit = 0;
					}
				if (NOW >= SP_m) {CMD.Exit = 1;}
			}
			if (CMD.Reset == 1) {NOW = 0.0; CMD.Reset = 0;}
		}

// Контроль переднего фронта сигнала
	class front_pos
	{
		public:
			bool exe(bool signal);
		private:
			bool x = 0;
	};

	bool front_pos::exe(bool signal)
	{
		if (signal == 0) {x = 0; return false;}
		if (signal == 1 and x == 1) {return false;}
		if (signal == 1 and x == 0) {x = 1; return true;}
	}

// Контроль заднего фронта сигнала
	class front_neg
	{
		public:
			bool exe(bool signal);
		private:
			bool x = 0;
	};

	bool front_neg::exe(bool signal)
	{
		if (signal == 1) {x = 0; return false;}
		if (signal == 0 and x == 1) {return false;}
		if (signal == 0 and x == 0) {x = 1; return true;}
	}

// склейка 2 word >> 1 float
	class word_to_float
	{
		public:
			float exe(int word_1, int word_2);
		private:
			union
			{
				float asFloat;
				int asInt[2];
			}
			value;
	};
	
	float word_to_float::exe(int word_1, int word_2)
	{
		value.asInt[0] = word_1;
		value.asInt[1] = word_2;
		return value.asFloat;
	}
