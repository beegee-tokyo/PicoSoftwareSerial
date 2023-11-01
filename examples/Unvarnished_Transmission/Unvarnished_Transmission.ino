/**
   @file Unvarnished_Transmission.ino
   @author rakwireless.com
   @brief unvarnished transmission via USB
   @version 0.1
   @date 2021-6-28
   @copyright Copyright (c) 2020
**/

#include <Wire.h>
#include <PicoSoftwareSerial.h>

#define POWER_KEY WB_IO5

#define TX_PIN PIN_SERIAL1_TX
#define RX_PIN PIN_SERIAL1_RX

SoftwareSerial SoftSerial1 = SoftwareSerial(TX_PIN, RX_PIN);

void setup()
{
	time_t serial_timeout = millis();
	Serial.begin(115200);

	while (!Serial)
	{
		if ((millis() - serial_timeout) < 5000)
		{
			delay(100);
		}
		else
		{
			break;
		}
	}

	Serial.println("AT CMD TEST!");
	// Check if the modem is already awake
	time_t timeout = millis();
	bool moduleSleeps = true;
	SoftSerial1.begin(9600);
	delay(1000);
	SoftSerial1.println("ATI");
	// MC20 init
	while ((millis() - timeout) < 6000)
	{
		if (SoftSerial1.available())
		{
			String result = SoftSerial1.readString();
			Serial.println("Modem response after start:");
			Serial.println(result);
			moduleSleeps = false;
		}
	}
	if (moduleSleeps)
	{
		// Module slept, wake it up
		pinMode(POWER_KEY, OUTPUT);
		digitalWrite(POWER_KEY, 0);
		delay(200);
		digitalWrite(POWER_KEY, 1);
		delay(2000);
		digitalWrite(POWER_KEY, 0);
		delay(1000);
	}
	Serial.println("MC20 power up!");
}

void loop()
{
	while (SoftSerial1.available())
	{
		Serial.print((char)SoftSerial1.read());
	}
	while (Serial.available())
	{
		SoftSerial1.print((char)Serial.read());
	}
	// int timeout = 100;
	// String resp = "";
	// String snd = "";
	// char cArr[128] = {0};
	// while (timeout--)
	// {
	// 	if (SoftSerial1.available() > 0)
	// 	{
	// 		resp += char(SoftSerial1.read());
	// 	}
	// 	if (Serial.available() > 0)
	// 	{
	// 		snd += char(Serial.read());
	// 	}
	// 	delay(1);
	// }
	// if (resp.length() > 0)
	// {
	// 	Serial.print(resp);
	// }
	// if (snd.length() > 0)
	// {
	// 	memset(cArr, 0, 128);
	// 	snd.toCharArray(cArr, snd.length() + 1);
	// 	SoftSerial1.write(cArr);
	// 	delay(10);
	// }
	// resp = "";
	// snd = "";
}
