#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>
#include <RotaryEncoder.h>
#include <EasyButton.h>

#include "Global.h"

namespace Encoder
{
  const int32_t ENC_A = PB12;
  const int32_t ENC_B = PB13;
  const int32_t ENC_BUTTON = PB14;
  
  bool enc_click;

  RotaryEncoder encoder( ENC_A, ENC_B );
  EasyButton button( ENC_BUTTON );

  void on_press()
  {
    //Serial.println("Click");
    enc_click = true;
  }
  
  void on_turn()
  {
    //Serial.println("Turn");
    encoder.tick();
  }

	// Encoder data
	int8_t turn_direction = 0;
	
	void init()
	{
    button.begin();
    button.onPressed( &on_press );
		attachInterrupt( ENC_A, &on_turn, CHANGE );
		attachInterrupt( ENC_B, &on_turn, CHANGE );
//   attachInterrupt( button, [](){ buttoned = true; }, CHANGE );
	}
	
	void read()
	{
		button.read();

    static int32_t count = 0;
		int32_t new_count = encoder.getPosition();
    
		if( new_count < count ) turn_direction = -1;
		else if( new_count > count ) turn_direction = 1;
		else turn_direction = 0;
    
		count = new_count;
	}
	
	bool clicked()
	{
		return enc_click;
	}
	
	int8_t turned()
	{
		return turn_direction;
	}
	
	void reset()
	{
		enc_click = false;
		turn_direction = 0;
	}
};

#endif /* INPUT_H */
