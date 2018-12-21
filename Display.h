#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306_STM32.h>

#include "Global.h"

// Width of text characters on screen with size 1
#define SYMBOL_WIDTH 6
//Display
#define OLED_ADDR 0x3C // OLED display TWI address
#define OLED_RESET 4

// Sensor data structure
namespace Display 
{
	using coord = uint8_t;
	
	Adafruit_SSD1306 display(OLED_RESET);
	  
  void clear()
  {
    display.clearDisplay();
  }
  
	void init()
	{
		display.begin( SSD1306_SWITCHCAPVCC, OLED_ADDR );
		clear();
	}
	
	void render()
	{
		display.display();
	};
	
	void line( coord x_from, coord y_from, coord x_to, coord y_to, uint32_t color )
	{
		display.drawLine( x_from, y_from, x_to, y_to, color );
	}
	
	void rect( coord x, coord y, coord w, coord h, uint32_t color )
	{
		display.drawRect( x, y, w, h, color);
	}
	
	void rounded_rect( coord x, coord y, coord w, coord h, coord rounding, uint32_t color )
	{
		display.drawRoundRect( x, y, w, h, rounding, color);
	}
 
  void set_cursor( coord x, coord y )
  {
    display.setCursor( x, y );
  }
  
  void set_text_color( uint32_t color )
  {
    display.setTextColor( color );
  } 
  
  void set_text_color( uint32_t color, uint32_t bg )
  {
    display.setTextColor( color, bg );
  }
	
	void set_text_size( uint8_t size )
	{
		display.setTextSize( size );
	}
  
  coord width()
  {
    return display.width();
  }
  
  coord height()
  {
    return display.height();
  }
  
  void print( const String& str ) { display.print( str ); }
	void print( uint8_t c )  { display.print( c ); }
	void print( int8_t c )	 { display.print( c ); }
	void print( double c ) 	 { display.print( c ); }
  
  void print_centered( const String& str, uint8_t size, coord y ) 
  {
    set_text_size( size );
    set_cursor( Global::ctr(size*SYMBOL_WIDTH * str.length(), width()), y );
    print( str ); 
  }
};

#endif /* DISPLAY_H */
