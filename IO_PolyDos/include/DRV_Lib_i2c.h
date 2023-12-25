#include <Arduino.h>
#include <Wire.h>

// мастер коммуникации по i2c
    class i2c_master
    {
        public:
            int i2c_AI[5];
            int i2c_AO[5];
            int i2c_SP[2];
            byte read_sys;
            byte write_sys;
            void setup();
            void data_send();
            void data_recieve();
            void exe();
        private:
            #define POLY_TYPE 0         //0 - дозация, 1 - приготовление
            byte slave = 1;
            byte length_read = 10;      //запрашиваемая длина данных
            int cnt;
            int parity;
    };

//инициализация
    void i2c_master::setup()
    {
        Wire.begin();
    }

// данные для отправки
    void i2c_master::data_send()
    {
        // определение типа установки
        if (POLY_TYPE == 0) {bitSet(write_sys, 0);}
        if (POLY_TYPE == 1) {bitSet(write_sys, 1);}

        // уставки для аналоговых выходов
        i2c_AO[0] = write_sys;
        for (size_t i = 0; i <= 4; i++)
        {
            i2c_AO[i] = Q_Analog[i].Signal;
        }
    }

// данные для получения
    void i2c_master::data_recieve()
    {
        for (size_t i = 0; i <= 4; i++)
        {
            IN_Analog[i].Signal = i2c_AI[i];
        }
        read_sys = i2c_AI[0];
    }

//обмен данными
    void i2c_master::exe()
    {
        // только для плат версии 4 и выше
        if (version >= 4)
        {
            // подготовка данных к отправке
            data_send();

            // начало коммуникации
            Wire.beginTransmission(slave);

            // передача уставок аналоговых выходов
            for (size_t i = 0; i < 5; i++)
            {
                i2c_AO[i] = 0;
                if (i >= 1 && i <=4) {i2c_AO[i] = Q_Analog[i].Signal;}
                Wire.write(i2c_AO[i]);
            }
            // передача уставок для шагового привода
            Wire.write(i2c_SP[0]);     //дистация, об * 100
            Wire.write(i2c_SP[1]);     //скорость, об/м * 10

            // завершение коммуникации
            Wire.endTransmission();

            // чтение данных
            cnt = 0;
            parity = 0;
            Wire.requestFrom(slave, length_read);
            while (Wire.available() > 0)
            {
                byte data = Wire.read();
                switch (parity)
                {
                case 0:
                    i2c_AI[cnt] = data * 256;
                    parity = 1;
                    break;
                case 1:
                    i2c_AI[cnt] = i2c_AI[cnt] + data;
                    parity = 0;
                    cnt++;
                    break;
                default:
                    break;
                }
            }

            // обработка полученных данных
            data_recieve();
        }
    }
