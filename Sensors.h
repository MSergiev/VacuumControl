#ifndef SENSORS_H
#define SENSORS_H

#include "Global.h"
#include "Sensor.h"

// Sensor data structure
namespace Sensors
{
	// Number of sensors
	uint8_t sensors_count = 0;
	
	// Sensor instances
  Sensor** sensors = nullptr;
	//Sensor sensors[2] = 
	//{ 
  //  BMP180(PA11, PA12, PB0), 
  //  BMP180(PB8, PB9, PB1) 
  //};
 
  uint8_t current_sensor = 0;
  double sensor_value_modifier = 1.0;
  
  Sensor& at( uint8_t idx )
  {
    return *sensors[idx];
  } 
  
  Sensor& current()
  {
    return *sensors[current_sensor];
  }
  
  uint8_t count()
  {
    return sensors_count;
  }
  
	bool init( uint8_t count )
	{      

    if( sensors )
    {
      delete[] sensors;
    }
    
	  sensors = new Sensor*[4];
    //sensors[0] = new BMP180(PA11, PA12, PB0); 
    //sensors[1] = new BMP180(PB8, PB9, PB1);
    sensors[0] = new BMP280(PA11, PA12, PB10);
    sensors[1] = new BMP280(PB8, PB9, PB11);
    
    sensors_count = count;
    current_sensor = 0;
    sensor_value_modifier = 1.0;
    
    for( uint8_t i = 0; i < count; ++i )
    {
      Serial.println( "Sensor init" );
      if( !at(i).init() )
      {
        Error::set( "Sensor Init" );
        Serial.println( "Sensor init error" );
        return false;
      }
    }
    
	  return true;
	}

 void read()
 {
    for( uint8_t i = 0; i < count(); ++i )
    {
      at(i).read();
    }
 }
};

#endif /* SENSORS_H */
