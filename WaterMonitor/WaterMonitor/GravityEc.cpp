﻿/*********************************************************************
* GravityEc.cpp
*
* Copyright (C)    2017   [DFRobot](http://www.dfrobot.com),
* This Library is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Description:
*
* author  :  Jason
* version :  V1.0
* date    :  2017-04-17
**********************************************************************/

#include "GravityEc.h"
#include "Arduino.h"



GravityEc::GravityEc(IWaterSensor* temp) :ECsensorPin(A1), ECcurrent(0), index(0), AnalogAverage(0),
AnalogValueTotal(0), averageVoltage(0), AnalogSampleTime(0), printTime(0),
tempSampleTime(0), AnalogSampleInterval(25),printInterval(700)
{
	this->ecTemperature = temp;
}


GravityEc::~GravityEc()
{
}


//********************************************************************************************
// 函数名称: setup()
// 函数说明：初始化传感器
//********************************************************************************************
void GravityEc::setup()
{
	pinMode(ECsensorPin, INPUT);
	for (byte thisReading = 0; thisReading < numReadings; thisReading++)
		readings[thisReading] = 0;
}


//********************************************************************************************
// 函数名称: update()
// 函数说明：更新传感器数值
//********************************************************************************************
void GravityEc::update()
{
	calculateAnalogAverage();
	calculateEc();
}


//********************************************************************************************
// 函数名称: getValue()
// 函数说明：返回传感器数据
//********************************************************************************************
double GravityEc::getValue()
{
	return ECcurrent;
}


//********************************************************************************************
// 函数名称: calculateAnalogAverage()
// 函数说明：计算平均电压
//********************************************************************************************
void GravityEc::calculateAnalogAverage()
{
	if (millis() - AnalogSampleTime >= AnalogSampleInterval)
	{
		AnalogSampleTime = millis();
		AnalogValueTotal = AnalogValueTotal - readings[index];
		readings[index] = analogRead(ECsensorPin);
		AnalogValueTotal = AnalogValueTotal + readings[index];
		index = index + 1;
		if (index >= numReadings)
			index = 0;
		AnalogAverage = AnalogValueTotal / numReadings;
		/*Serial.print("AnalogAverage = ");
		Serial.println(AnalogAverage);*/
	}
}


//********************************************************************************************
// 函数名称: calculateAnalogAverage()
// 函数说明：计算电导率
//********************************************************************************************
void GravityEc::calculateEc()
{
	if (millis() - printTime >= printInterval)
	{
		printTime = millis();
		averageVoltage = AnalogAverage*5000.0 / 1024.0;
		double TempCoefficient = 1.0 + 0.0185*(this->ecTemperature->getValue() - 25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.0185*(fTP-25.0));
	
		double CoefficientVolatge = (double)averageVoltage / TempCoefficient;		

		if (CoefficientVolatge < 150) {
			ECcurrent = 0;
			return;
		}			
		else if (CoefficientVolatge > 3300)
		{
			ECcurrent = 20;
			return;
		}		
		else
		{
			if (CoefficientVolatge <= 448)
				ECcurrent = 6.84*CoefficientVolatge - 64.32;   //1ms/cm<EC<=3ms/cm
			else if (CoefficientVolatge <= 1457)
				ECcurrent = 6.98*CoefficientVolatge - 127;   //3ms/cm<EC<=10ms/cm
			else 
				ECcurrent = 5.3*CoefficientVolatge + 2278;                           //10ms/cm<EC<20ms/cm
			ECcurrent /= 1000;    //convert us/cm to ms/cm
		}
	}
}
