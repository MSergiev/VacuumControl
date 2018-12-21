#ifndef UI_H
#define UI_H

#include "Global.h"
#include "Display.h"

namespace UI
{	  
  // Main menu item Strings
  const String menu_Strings[] = 
  {
    "Back",
    "Start",
    "Set Vacuum"
  };

  // "Set vacuum" menu item Strings
  const String vacuum_menu_Strings[] =
  {
    "Back",
    "Set Top",
    "Set Bottom"
  };
 
 // Draw intro screen
  void display_test()
  {
    Display::rounded_rect(0, 0, 127, 63, 7, WHITE);
    
    const auto frame = [&](const String& str, uint8_t size, uint8_t y, uint16_t ms_delay, bool transparent = true)
    {
      Display::set_text_color( WHITE, (transparent ? WHITE : BLACK) );
      Display::print_centered( str, size, y );
      Display::render();
      delay( ms_delay );
    };

    frame("SolidFill", 2, 10, 250);
    frame("VACUUM CASTING", 1, 33, 350);
    frame("Initializing", 1, 49, 350);
    frame("<              >", 1, 49, 100);
    frame("<               >", 1, 49, 100);
    frame("<                >", 1, 49, 100);
    frame("                  ", 1, 49, 100, false);
    frame("SYSTEM READY", 1, 49, 750);
  }

	// Draw error screen
	void draw_error() 
	{
	  Display::set_text_color( WHITE );
    Display::print_centered( "ERROR", 1, 5 );
    Display::print_centered( Error::get(), 1, 20 );
    Display::print_centered( "Push to reinit", 1, 50 );
	}

	// Draw info screen
	void draw_info() 
	{
	  Display::set_text_color(WHITE);

	  // Header
    Display::print_centered( (Global::is_active ? "STATUS:ACTIVE" : "STATUS:IDLE"), 1, 5 );

	  // Sensors
	  {
  		const uint8_t LEN = SYMBOL_WIDTH*strlen("S0");
  		const uint8_t OFFSET = 10;
  
  		for( uint8_t i = 0; i < Sensors::count(); ++i ) 
  		{
  		  Display::set_cursor( Global::dst( i, Sensors::count(), LEN, Display::width()-OFFSET ) + OFFSET, 20 );
  		  Display::print( "S" );
  		  Display::print( i );
  		}
	  }

	  // Sensor data
	  {
  		const uint8_t ROW_1 = 35;
  		const uint8_t ROW_2 = 48;
  
  		Display::set_cursor( 10, ROW_1 );
  		Display::print( "P:" );
  
  		Display::set_cursor( 10, ROW_2 );
  		Display::print( "T:" );
  
  		const uint8_t LEN = SYMBOL_WIDTH*strlen("000.00");
  		const uint8_t OFFSET = 12;
  
  		for( uint8_t i = 0; i < Sensors::count(); ++i ) 
  		{
  		  const uint8_t X_VAL = Global::dst( i, Sensors::count(), LEN, Display::width()-OFFSET ) + OFFSET;
  
  		  Display::set_cursor( X_VAL, ROW_1 );
  		  Display::print( Sensors::at(i).pressure() );
  		  Display::set_cursor( X_VAL, ROW_2 );
  		  Display::print( Sensors::at(i).temperature() );
  		}
	  }

	}

	// Draw menu screen
	void draw_menu() 
	{
	  Display::set_text_color(WHITE);
	  Display::set_text_size(1);

	  // Header
    Display::print_centered( "SETTINGS", 1, 5 );

	  // Menu Strings
	  {
  		const uint8_t X_START = 12;
  		const uint8_t Y_START = 20;
  		const uint8_t Y_OFFSET = 10;
  
  		switch( States::menu_category ) 
  		{
  		  case States::MenuCategory::MainMenu: 
  		  {
    			for( uint8_t i = 0; i < (uint8_t)States::MenuItem::Count; ++i ) 
    			{
    			  Display::set_cursor( X_START, Y_START + i*Y_OFFSET );
    
    			  // Print cursor position
    			  if( i == (uint8_t)States::menu_item ) 
    			  {
    				  Display::print( "> " );
    			  } 
    			  else 
    			  {
    				  Display::print( "  " );
    			  }
    
    			  if( i == (uint8_t)States::MenuItem::MenuStart && Global::is_active ) 
    			  {
    				  Display::print( "Stop" );
    			  } 
    			  else 
    			  {
    				  Display::print( menu_Strings[i] );
    			  }
    			}
  		  }
  		  break;
  		  
  		  case States::MenuCategory::VacuumMenu: 
  		  {
    			for( uint8_t i = 0; i < (uint8_t)States::VacuumMenuItem::Count; ++i ) 
    			{
    			  Display::set_cursor(X_START, Y_START + i*Y_OFFSET);
    
    			  // Print cursor position
    			  if( i == (uint8_t)States::vacuum_menu_item ) 
    			  {
    				  Display::print( "> " );
    			  } 
    			  else 
    			  {
    				  Display::print( "  " );
    			  }
    
    			  Display::print( vacuum_menu_Strings[i] );
    			}
  		  }
  		  break;
  		}
	  }
	}

	// Draw vacuum set screen
	void draw_set_vacuum() 
	{
	  Display::set_text_color(WHITE);
    
	  // Header
  	Display::print_centered( ( Sensors::current_sensor == 0 ? "SET TOP PRESSURE" : "SET BOTTOM PRESSURE" ), 1, 5 );

	  // Digit marker
	  {
  		uint8_t COORD_X;
  		if( Sensors::sensor_value_modifier == 1.0 ) 
  		{
  		  COORD_X = 26;
  		} 
  		else if( Sensors::sensor_value_modifier == 0.1 ) 
  		{
  		  COORD_X = 62;
  		} 
  		else 
  		{
  		  COORD_X = 80;
  		}
  
  		Display::rect( COORD_X, 25, 3*SYMBOL_WIDTH + 1, 27, WHITE );
	  }

	  // Set number
    Display::print_centered( String(Sensors::current().target), 3, 28 );
	}

   
  // Draw interface
  void draw() 
  {
    Display::clear();
    Display::rounded_rect(0, 0, 127, 63, 7, WHITE);
    Display::line(9, 16, 117, 16, WHITE);

    switch( States::system_state ) 
    {
      case States::SystemState::ErrorScreen: draw_error();      break;
      case States::SystemState::InfoScreen:  draw_info();       break;
      case States::SystemState::MenuScreen:  draw_menu();       break;
      case States::SystemState::SetVacuum:   draw_set_vacuum(); break;
      default: break;
    }

    Display::render();
  }
};

#endif /* UI_H */
