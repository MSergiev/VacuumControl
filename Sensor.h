#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include "Sensors/BMP180.h"
#include "Sensors/BMP280.h"

#include "Global.h"

class Sensor
{    
  // Relay pin
  int8_t relay_pin = 0;
  
protected:

  // Data
  double Temperature = 0.0;
  double Pressure = 0.0;  
  
  // Reference pressure value
  double reference = -1.00;
  
public:

  // Target pressure value
  double target = -1.00;

  virtual bool init() = 0;

  Sensor( int32_t relay ) 
  : relay_pin( relay ) 
  {    
    reference = -1.0;
    pinMode( relay_pin, OUTPUT_OPEN_DRAIN );
  }
  
  virtual void read()
  {         
    if( reference < 0 ) 
    {
       reference = Pressure;
    }
    Pressure /= reference;
    const bool pin_state = ( Global::is_active && (pressure() < target) );
    digitalWrite( relay_pin, (pin_state ? LOW : HIGH) );
  }
  
  double pressure()
  {
    return Pressure;
  }
  
  double temperature()
  {
    return Temperature;
  }
};

// BMP180 Sensor
class BMP180 : public Sensor
{
  //Sensor library
  SFE_BMP180 sensor;

  // Sensor timer
  uint64_t timer = 0;

  // Sensor state
  enum { SensorIdle, SensorTempRetrieval, SensorPressureRetrieval } state;

  // Sensor timer delay
  uint8_t wait = 0;

public:
  
  BMP180( int8_t scl, int8_t sda, int8_t relay ) : sensor( scl, sda ), Sensor( relay ) {}
  
  bool init() override
  {
    return sensor.begin();
  }
  
  void read() override
  {
  	switch( state ) 
  	{
  	  case SensorIdle: 
  	  {
    		wait = sensor.startTemperature();
    		if( wait == 0 ) 
    		{
    		  Serial.println( "Error starting temperature measurement" );
    		  Error::set( "Sensor temp. start" );
    		  return;
    		}
    
    		timer = millis();
    		state = SensorTempRetrieval;
  	  } // case SensorIdle
  	  break;
  
  	  case SensorTempRetrieval: 
  	  {
    		if( millis() - timer > wait ) 
    		{
    		  wait = sensor.getTemperature( Temperature );
    		  if( wait == 0 ) 
    		  {
      			Serial.println( "Error retrieving temperature measurement" );
      			Error::set( "Sensor temp. measure" );
      			state = SensorIdle;
      			return;
    		  }
    
    		  wait = sensor.startPressure(3);
    		  if( wait == 0 ) 
    		  {
      		  Serial.println( "Error starting pressure measurement" );
      			Error::set( "Sensor pres. start" );
      			state = SensorIdle;
      			return;
    		  }
    
    		  timer = millis();
    		  state = SensorPressureRetrieval;
    		}
  	  } // case SensorTempRetrieval
  	  break;
  
  	  case SensorPressureRetrieval: 
  	  {
    		wait = sensor.getPressure( Pressure, Temperature );
    		if( wait == 0 ) 
    		{
    		  Serial.println( "Error retrieving pressure measurement" );
    		  Error::set( "Sensor pres. measure" );
    		}
    		state = SensorIdle;
  	  } // case SensorPressureRetrieval
  	  break;
  	}

    Sensor::read();
  }
};

// BMP280 Sensor
class BMP280 : public Sensor
{
  //Sensor library
  Adafruit_BMP280 sensor;

public:
  
  BMP280( int8_t scl, int8_t sda, int8_t relay ) : sensor( scl, sda ), Sensor( relay ) {}
  
  bool init() override
  {
    return sensor.begin();
  }
  
  void read() override
  {
    Temperature = sensor.readTemperature();
    Pressure = sensor.readPressure();
    Sensor::read();
  }
};

#endif /* SENSOR_H */
