/*********************************************************************
 * Display PIN connection:
 * pin connection see: https://www.arduino.cc/en/Reference/Wire
 * for UNO: SDA to A4, SCL to A5
 * for Mega2560: SDA to 20, SCL to 21
 * for Leonardo: SDA to 2, SCL to 3
 * for Due: SDA to 20, SCL to 21
 * VCC to 5V
 * GND to GND :-)
*/

/*
 * Pressure Sensor Hardware connections:
- (GND) to GND
+ (VDD) to 3.3V !!!
(WARNING: do not connect + to 5V or the sensor will be damaged!)

You will also need to connect the I2C pins (SCL and SDA) to your
Arduino. The pins are different on different Arduinos:

Any Arduino pins labeled:  SDA  SCL
Uno, Redboard, Pro:        A4   A5
Mega2560, Due:             20   21
Leonardo:                   2    3

Leave the IO (VDDIO) pin unconnected. This pin is for connecting
the BMP180 to systems with lower logic levels such as 1.8V
 */

#include "Global.h"

#include "Display.h"
#include "Sensors.h"
#include "Input.h"
#include "States.h"
#include "UI.h"

// const int32_t RELAYS[] = { PB0, PB1, PB10, PB11 };

//////////////////////////////////////////////////////////////////////////////////////////////

bool initialize();
void process_input();

//////////////////////////////////////////////////////////////////////////////////////////////

void setup() 
{
  Serial.begin(9600);
  Serial.println( "BEGIN" );
  if( !initialize() )
  {
    UI::draw();
  }
  else
  {
    UI::display_test();
  }
}

void loop() 
{
  Encoder::read();
  process_input();
  Sensors::read();
  UI::draw();
  Encoder::reset();
}

//////////////////////////////////////////////////////////////////////////////////////////////

// Initialize devices
bool initialize() 
{
  //Display Setup
  Serial.println( "Display init" );
  Display::init();

  Serial.println( "Input init" );
  Encoder::init();

  //Sensor Setup
  Serial.println( "Sensors init" ); 
  if( !Sensors::init(2) )
  {
      Serial.println( "Sensor Init Error" );
  }

  return true;
}

// Read encoder input data
void process_input() 
{
  if( !Encoder::turned() && !Encoder::clicked() ) 
  {
    return;
  }
  
  switch( States::system_state ) 
  {
    case States::SystemState::InfoScreen: 
    {
      if( Encoder::clicked() ) 
      {
        States::system_state = States::SystemState::MenuScreen;
      }
    }
    break;
  
    case States::SystemState::MenuScreen: 
    {
      switch( States::menu_category ) 
      {
        case States::MenuCategory::MainMenu: 
        {
          if( Encoder::turned() ) 
          {
            int8_t selection = (uint8_t)States::menu_item + Encoder::turned();
            if( selection >= (uint8_t)States::MenuItem::Count ) 
            {
              selection = 0;
            } 
            else if ( selection < 0 ) 
            {
              selection = (int8_t)States::MenuItem::Count - 1;
            }
            States::menu_item = (States::MenuItem)selection;
          }
  
          if( Encoder::clicked() ) 
          {
            switch( States::menu_item ) 
            {
              case States::MenuItem::MenuBack: 
              {
                States::system_state = States::SystemState::InfoScreen;
              }
              break;
  
              case States::MenuItem::MenuStart: 
              {
                Global::is_active = !Global::is_active;
                States::system_state = States::SystemState::InfoScreen;
              }
              break;
  
              case States::MenuItem::MenuSetVacuum: 
              {
                States::menu_category = States::MenuCategory::VacuumMenu;
              }
              break;
  
              default: break;
            }
          }
        }
        break;
  
        case States::MenuCategory::VacuumMenu: 
        {
          if( Encoder::turned() ) 
          {
            int8_t selection = (int8_t)States::vacuum_menu_item + Encoder::turned();
            if( selection >= (uint8_t)States::VacuumMenuItem::Count ) 
            {
              selection = 0;
            } 
            else if ( selection < 0 ) 
            {
              selection = (int8_t)States::VacuumMenuItem::Count - 1;
            }
            States::vacuum_menu_item = (States::VacuumMenuItem)selection;
          }
  
          if( Encoder::clicked() ) 
          {
            switch( States::vacuum_menu_item ) 
            {
              case States::VacuumMenuItem::Vacuum1Set: 
              {
                Sensors::current_sensor = 0;
                States::system_state = States::SystemState::SetVacuum;
              }
              break;
  
              case States::VacuumMenuItem::Vacuum2Set: 
              {
                Sensors::current_sensor = 1;
                States::system_state = States::SystemState::SetVacuum;
              }
              break;
  
              case States::VacuumMenuItem::VacuumBack: 
              {
                States::menu_category = States::MenuCategory::MainMenu;
              }
              break;
  
              default: break;
            }
          }
        }
        break;
  
        default: break;
      }
    }
    break;
  
    case States::SystemState::SetVacuum: 
    {
      if( Encoder::turned() ) 
      {
        Sensors::current().target += Sensors::sensor_value_modifier * Encoder::turned();
        if( Sensors::current().target < 0.0 ) 
        {
          Sensors::current().target = 0.0;
        } 
        else if( Sensors::current().target > 9.99 ) 
        {
          Sensors::current().target = 9.99;
        }
      }
  
      if( Encoder::clicked() ) 
      {
        Sensors::sensor_value_modifier /= 10;
        if( Sensors::sensor_value_modifier < 0.01 ) 
        {
          States::system_state = States::SystemState::MenuScreen;
          Sensors::sensor_value_modifier = 1;
        }
      }
    }
    break;
  
    case States::SystemState::ErrorScreen: 
    {
      if( Encoder::clicked() ) 
      {
        States::system_state = States::SystemState::InfoScreen;
        initialize();
      }
    }
    break;
  
    default: break;
  }
}
