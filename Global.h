#ifndef GLOBAL_H
#define GLOBAL_H

#include "States.h"

namespace Global
{
	bool is_active;

  // Center X coordinate of an object on screen
  inline uint8_t ctr( uint8_t width, uint8_t total_width ) 
  {
    return ( total_width - width ) / 2;
  }
  
  // Distribute on screen
  inline uint8_t dst( uint8_t num, uint8_t total, uint8_t width, uint8_t total_width ) 
  {
    uint8_t seg = total_width / total;
    return abs( seg - width ) / 2 + seg*num;
  }
}

namespace Error
{
  String error = "";
  
	inline void set( const String& str ) 
	{
	  Serial.println( str );
	  error = str;
    Serial.println( "Setting error state" );
	  States::system_state = States::SystemState::ErrorScreen;
	  Global::is_active = false;
	}
	
	inline bool is_set()
	{
		return error != "";
	}
	
	inline const String& get()
	{
		return error;
	}
	
  inline void clear()
  {
    error = "";
  }
  
  inline uint8_t length()
  {
    return error.length();
  }
};

#endif /* GLOBAL_H */
