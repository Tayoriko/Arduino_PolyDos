#include <Arduino.h>

// управление насосами линии
class DRV_Pump
{
    public:
        void ee_control(int addr, bool update, bool first_load);
        void exe();
        void HMI();
        int xMode = 0;
        int xLine = 0;
        int xMD = 0;
        int Type = 0;   //0 - насос с управление по АО, 1 - насос с прямым пуском, 2 - насос с работой по времени
        TON_s T_Main;
        TON_s T_Reserv;
        TON_s T_Clock_ON;
        TON_s T_Clock_OFF;
        int HMI_ID = 0;
    private:
        float percent = 0;
        int next_addr = 0;
        int HMI_addr = 0;
};

// Работа с памятью
	void DRV_Pump::ee_control(int addr, bool update, bool first_load)
	{
		if (first_load == 1)
		{
            HMI_ID = addr * 2;
            xLine = EEPROM.readInt(HMI_ID);
            xMode = EEPROM.readInt(HMI_ID + 2);
            T_Main.tSP = EEPROM.readInt(HMI_ID + 4);
			HMI_Data[HMI_ID] = xLine;
            HMI_Data[HMI_ID + 1] = xMode;
            HMI_Data[HMI_ID + 2] = T_Main.tSP;
		}

		if (update == 1)
		{
            HMI_ID = addr * 2;
			EEPROM.updateInt(HMI_ID, xLine);
            EEPROM.updateInt(HMI_ID + 2, xMode);
            EEPROM.updateInt(HMI_ID + 4, T_Main.tSP);
            HMI_Data[HMI_ID] = xLine;
            HMI_Data[HMI_ID + 1] = xMode;
            HMI_Data[HMI_ID + 2] = T_Main.tSP;
            if (T_Main.tSP == 1) {Type = 1;}
            if (T_Main.tSP == 0) {Type = 0;}
            if (T_Main.tSP >= 2) {Type = 2;}
		}
	}

// Работа с панелью
void DRV_Pump::HMI()
{
    xLine = HMI_Data[HMI_ID];
    xMode = HMI_Data[HMI_ID + 1];
    T_Main.tSP = HMI_Data[HMI_ID + 2];
}

// управление насосом
void DRV_Pump::exe()
{
    //выбор типа насоса
        if (T_Main.tSP == 0)    {Type = 0;}
        if (T_Main.tSP == 1)    {Type = 1;}
        if (T_Main.tSP > 1)     {Type = 2;}
    // авария насоса
        if (MD[xMD].Status > 3 and xMode == 1) {Line[xLine].CMD.Alarm_Main = 1;}
        if (MD[xMD].Status > 3 and xMode == 2) {Line[xLine].CMD.Alarm_Reserv = 1;}

    // нормальные насосы
    if (Type == 0)
    {
        // насос не активен
            if (xMode == 0) {MD[xMD].CMD.Start_A = 0; Q_Analog[xMD].SP_A = 0;}
        // насос - основной
            if (xMode == 1)
            {
                MD[xMD].CMD.Start_A = Line[xLine].CMD.CMD_Main;
            }
        // насос - резервный
            if (xMode == 2)
            {
                MD[xMD].CMD.Start_A = Line[xLine].CMD.CMD_Reserv;
            }
        //задание расхода
        if (MD[xMD].CMD.Start_A == 1) {Q_Analog[xMD].SP_A = Line[xLine].xSP;}
        if (MD[xMD].CMD.Start_A == 0) {Q_Analog[xMD].SP_A = 0.0;}
    }
    
    // импульсные насосы
    if (Type == 1)
    {   
        T_Clock_ON.exe(pulse.s001);
        T_Clock_OFF.exe(pulse.s01);
        T_Clock_ON.tSP = 0.5;
        percent = (Line[xLine].xSP / Q_Analog[xMD].max_out) * 100.0;
        if (percent > 0) {T_Clock_OFF.tSP = 20.0 / percent;} else {T_Clock_OFF.tSP = 20.0;}
        // насос не активен
            if (xMode == 0) {MD[xMD].CMD.Start_A = 0; Q_Analog[xMD].SP_A = 0;}
        // насос - основной
            if (xMode == 1)
            {
                MD[xMD].CMD.Start_A = Line[xLine].CMD.CMD_Main;
            }
        // насос - резервный
            if (xMode == 2)
            {
                MD[xMD].CMD.Start_A = Line[xLine].CMD.CMD_Reserv;
            }
        //задание расхода
            if (MD[xMD].CMD.Start_A == 1) 
            {
                if (T_Clock_ON.CMD.Exit == 0 and T_Clock_OFF.CMD.Exit == 0) {T_Clock_ON.CMD.Start = 1; T_Clock_OFF.CMD.Start = 1; Q_Analog[xMD].SP_A = Q_Analog[xMD].max_out;}
                if (T_Clock_ON.CMD.Exit == 1 and T_Clock_OFF.CMD.Exit == 0) {Q_Analog[xMD].SP_A = 0;}
                if (T_Clock_ON.CMD.Exit == 1 and T_Clock_OFF.CMD.Exit == 1) {T_Clock_ON.CMD.Start = 0; T_Clock_OFF.CMD.Start = 0;}
            }
        if (MD[xMD].CMD.Start_A == 0) {T_Clock_ON.CMD.Start = 0; T_Clock_OFF.CMD.Start = 0; Q_Analog[xMD].SP_A = 0.0;}
    }

    // охуевшие насосы для бомжей
    if (Type == 2)
    {  
        T_Main.exe(pulse.s01);
        T_Reserv.exe(pulse.s01);
        T_Reserv.tSP = T_Main.tSP;
        percent = Line[xLine].xSP / Q_Analog[xMD].max_out;
        // насос не активен
            if (xMode == 0) {MD[xMD].CMD.Start_A = 0; T_Main.CMD.Start = 0; T_Reserv.CMD.Start = 0;}
        // насос - основной
            if (xMode == 1)
            {
                if (Line[xLine].CMD.CMD_Main == 1)
                {
                    if (T_Main.CMD.Exit == 0)   {T_Main.CMD.Start = 1;}
                    if (T_Main.CMD.Exit == 1)   {T_Main.CMD.Start = 0;}
                    if (percent <= T_Main.percent) {MD[xMD].CMD.Start_A = 1;} else {MD[xMD].CMD.Start_A = 0;}
                }
                else
                {
                    T_Main.CMD.Start = 0;
                    MD[xMD].CMD.Start_A = 0;
                }
            }
        // насос - резервный
            if (xMode == 2)
            {
                if (Line[xLine].CMD.CMD_Reserv == 1)
                {
                    if (T_Reserv.CMD.Exit == 0)   {T_Reserv.CMD.Start = 1;}
                    if (T_Reserv.CMD.Exit == 1)   {T_Reserv.CMD.Start = 0;}
                    if (percent <= T_Reserv.percent) {MD[xMD].CMD.Start_A = 1;} else {MD[xMD].CMD.Start_A = 0;}
                }
                else
                {
                    T_Reserv.CMD.Start = 0;
                    MD[xMD].CMD.Start_A = 0;
                }
            }
    }
}

