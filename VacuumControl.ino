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

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306_STM32.h>
#include "SFE_BMP180.h"
#include <RotaryEncoder.h>
#include <EasyButton.h>
#include <SoftWire.h>

typedef const char* string;

//Display
#define OLED_ADDR 0x3C // OLED display TWI address
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

//Rotary Encoder
const int32_t encOutputA = PB12;
const int32_t encOutputB = PB13;
const int32_t encClick = PB14;

// Encoder data
RotaryEncoder encoder(encOutputA, encOutputB);
EasyButton encoderClick(encClick);
bool encoderClicked = false;
int32_t encCount = 0;

// Width of text characters on screen with size 1
const uint8_t SYMBOL_WIDTH = 6;

// Vacuum sensor setting values
uint8_t currentSensorToSet = 0;
double sensorValueModifier = 1.0;

// Error string
string currentError;

// System is active flag
bool isActive = false;

// System states
enum SystemState {
  InfoScreen,
  MenuScreen,
  SetVacuum,
  ErrorScreen,
  SystemStateCount
} currentState;

// Menu types
enum MenuCategories {
  MainMenu,
  VacuumMenu
} currentMenuCategory;

// Main menu items
enum MenuItem {
  MenuBack,
  MenuStart,
  MenuSetVacuum,
  MenuItemCount
} currentMenuItem;

// Main menu item strings
string MenuStrings[MenuItemCount] = {
  "Back",
  "Start",
  "Set Vacuum",
};

// "Set vacuum" menu items
enum VacuumMenuItem {
  VacuumBack,
  Vacuum1Set,
  Vacuum2Set,
  VacuumMenuItemCount
} currentVacuumMenuItem;

// "Set vacuum" menu item strings
string VacuumMenuStrings[VacuumMenuItemCount] = {
  "Back",
  "Set Top Chamber",
  "Set Bottom Chamber",
};

// Pressure sensor states
enum SensorState {
  SensorIdle,
  SensorTempRetrieval,
  SensorPressureRetrieval,
} sensorState;

// Sensor data structure
struct SensorData {

  //Sensor library
  SFE_BMP180 pressure;

  // Data
  double Temperature;
  double Pressure;

  // Sensor timer
  uint64_t timer = 0;

  // Reference pressure value
  double reference = -1.00;

  // Target pressure value
  double target = 0.00;

  // Sensor state
  SensorState state;

  // Sensor timer delay
  uint8_t wait = 0;

  // Relay pin
  int32_t relay_pin = 0;

  // Constructor
  SensorData( int32_t scl, int32_t sda, int32_t relay_pin )
  : pressure( scl, sda ), relay_pin(relay_pin)
  {}

};

// Number of sensors
const uint8_t SENSORS_NUM = 1;

// Sensors data
SensorData sensors[] = {
  SensorData( PA11, PA12, PB0 ),
  // SensorData( PB8, PB9, PB1 )
};

// Relays
// const uint8_t RELAY_COUNT = 4;
// const int32_t RELAYS[] = { PB0, PB1, PB10, PB11 };

//////////////////////////////////////////////////////////////////////////////////////////////

bool Init();
void EncoderRead();
void SensorRead();
void Control();
void Draw();
void DisplayTest();
void DrawInfo();
void DrawMenu();
void DrawError();
void DrawSetVacuum();

void SetError( string );
uint8_t ctr( uint8_t, uint8_t );
uint8_t dst( uint8_t, uint8_t, uint8_t, uint8_t );

//////////////////////////////////////////////////////////////////////////////////////////////

void setup() 
{
  Serial.begin(9600);

//Rotary Encoder Setup
   Serial.println( "BEGIN" );
   pinMode(encOutputA,INPUT); //encoder L
   pinMode(encOutputB,INPUT); //encoder R
   pinMode(encClick, INPUT); //encoder Switch
   attachInterrupt( encOutputA, [](){ encoder.tick(); }, CHANGE );
   attachInterrupt( encOutputB, [](){ encoder.tick(); }, CHANGE );
//   attachInterrupt( encClick, [](){ encoderClicked = true; }, CHANGE );

  Init();

  DisplayTest();
}

void loop() 
{
  SensorRead();
  EncoderRead();
  Control();
  Draw();
}

//////////////////////////////////////////////////////////////////////////////////////////////

// Initialize devices
bool Init() {

  Serial.println( "Encoder init" );
  encoderClick.begin();
  encoderClick.onPressed( [](){ encoderClicked = true; } );

//Display Setup
  Serial.println( "Display init" );
  display.begin( SSD1306_SWITCHCAPVCC, OLED_ADDR );
  display.clearDisplay();

//Pressure Setup
  for( uint8_t i = 0; i < SENSORS_NUM; ++i ) {
    Serial.print( "Init sensor " );
    Serial.println( +i );
    sensors[i].reference = -1.0;
    pinMode( sensors[i].relay_pin, OUTPUT_OPEN_DRAIN );
    if( sensors[i].pressure.begin() ){
      Serial.println( "Pressure Sensor Initialized" );
    } else {
      Serial.println( "Pressure Sensor Init Error" );
      SetError( "Sensor Init" );
      return false;
    }
  }
  Serial.println( "Sensor init successful" );

  return true;
}

// Read encoder input data
void EncoderRead() {

  encoderClick.read();

  bool encoderTurned = false;
  int8_t encoderTurnDirection = 0;

  // Set encoder direction
  {
    int newEncCount = encoder.getPosition();
    if( newEncCount != encCount ) {
      encoderTurned = true;
      if( newEncCount <= encCount ) {
        encoderTurnDirection = -1;
      }
      else {
        encoderTurnDirection = 1;
      }
    }
    encCount = newEncCount;
  }


// Behaviours

  if( encoderTurned || encoderClicked ) {

    switch( currentState ) {

// Info screen
      case InfoScreen: {
        if( encoderClicked ) {
          currentState = MenuScreen;
        }
        break;
      }

// Menu
      case MenuScreen: {
        switch( currentMenuCategory ) {

// Menu -> Main menu
          case MainMenu: {

            if( encoderTurned ) {
              int8_t selection = currentMenuItem + encoderTurnDirection;
              if( selection >= MenuItemCount ) {
                selection = 0;
              } else if ( selection < 0 ) {
                selection = MenuItemCount - 1;
              }
              currentMenuItem = (MenuItem)(selection);
            }

            if( encoderClicked ) {
              switch( currentMenuItem ) {

  // Menu -> Main menu -> Back
                case MenuBack: {
                  currentState = InfoScreen;
                  break;
                } // case MenuBack

  // Menu -> Main menu -> Start
                case MenuStart: {
                  isActive = !isActive;
                  currentState = InfoScreen;
                  break;
                } // case MenuStart

  // Menu -> Main menu -> Set Vacuum
                case MenuSetVacuum: {
                  currentMenuCategory = VacuumMenu;
                  break;
                } // MenuSetVacuum

                default: break;
              } // switch( currentMenuItem )
            }

            break;
          } // case MainMenu

// Menu -> Vacuum menu
          case VacuumMenu: {

            if( encoderTurned ) {
              int8_t selection = currentVacuumMenuItem + encoderTurnDirection;
              if( selection >= VacuumMenuItemCount ) {
                selection = 0;
              } else if ( selection < 0 ) {
                selection = VacuumMenuItemCount - 1;
              }
              currentVacuumMenuItem = (VacuumMenuItem)(selection);
            }

            if( encoderClicked ) {
              switch( currentVacuumMenuItem ) {

  // Menu -> Vacuum menu -> Vacuum Top Set
                case Vacuum1Set: {
                  currentSensorToSet = 0;
                  currentState = SetVacuum;
                  break;
                } // case Vacuum1Set

  // Menu -> Vacuum menu -> Vacuum Bottom Set
                case Vacuum2Set: {
                  currentSensorToSet = 1;
                  currentState = SetVacuum;
                  break;
                } // case Vacuum2Set

  // Menu -> Vacuum menu -> Back
                case VacuumBack: {
                  currentMenuCategory = MainMenu;
                  break;
                } // case VacuumBack

                default: break;
              } // switch( currentVacuumMenuItem )
            }

            break;
          } // case VacuumMenu

          default: break;
        } // switch( currentMenuCategory )

        break;
      } // case MenuScreen

// Vaccum Set Screen
      case SetVacuum: {
        if( encoderTurned ) {
          sensors[currentSensorToSet].target += sensorValueModifier * encoderTurnDirection;
          if( sensors[currentSensorToSet].target < 0.0 ) {
            sensors[currentSensorToSet].target = 0.0;
          } else if( sensors[currentSensorToSet].target > 9.99 ) {
            sensors[currentSensorToSet].target = 9.99;
          }
        }

        if( encoderClicked ) {
          sensorValueModifier /= 10;
          if( sensorValueModifier < 0.01 ) {
            currentState = MenuScreen;
            sensorValueModifier = 1;
          }
        }
        break;
      } // case SetVacuum

// Error screen
      case ErrorScreen: {
        if( encoderClicked ) {
          currentState = InfoScreen;
          Init();
        }
        break;
      }

      default: break;
    } // switch( currentState )

  } // if( encoderTurned || encoderClicked )

  encoderClicked = false;
}

// Read sensor data
void SensorRead() {

  for( uint8_t i = 0; i < SENSORS_NUM; ++i ) {
    switch( sensors[i].state ) {

      case SensorIdle: {
        sensors[i].wait = sensors[i].pressure.startTemperature();
        if( sensors[i].wait == 0 ) {
          Serial.println( "Error starting temperature measurement" );
          SetError( "Sensor temp. start" );
          return;
        }

        sensors[i].timer = millis();
        sensors[i].state = SensorTempRetrieval;
        break;
      }

      case SensorTempRetrieval: {
        if( millis() - sensors[i].timer > sensors[i].wait ) {

          sensors[i].wait = sensors[i].pressure.getTemperature( sensors[i].Temperature );
          if( sensors[i].wait == 0 ) {
            Serial.println( "Error retrieving temperature measurement" );
            SetError( "Sensor temp. measure" );
            sensors[i].state = SensorIdle;
            return;
          }

          sensors[i].wait = sensors[i].pressure.startPressure(3);
          if( sensors[i].wait == 0 ) {
            Serial.println( "Error starting pressure measurement" );
            SetError( "Sensor pres. start" );
            sensors[i].state = SensorIdle;
            return;
          }

          sensors[i].timer = millis();
          sensors[i].state = SensorPressureRetrieval;
        }
        break;
      }

      case SensorPressureRetrieval: {
        sensors[i].wait = sensors[i].pressure.getPressure( sensors[i].Pressure, sensors[i].Temperature );
        if( sensors[i].wait == 0 ) {
          Serial.println( "Error retrieving pressure measurement" );
          SetError( "Sensor pres. measure" );
        }
        else {
          if( sensors[i].reference < 0 ) {
            sensors[i].reference = sensors[i].Pressure;
          }
          sensors[i].Pressure /= sensors[i].reference;
        }
        sensors[i].state = SensorIdle;
        break;
      }

    }
  }

}

// Control block
void Control()
{

  bool pinStates[SENSORS_NUM] = { false };

  for( uint8_t i = 0; i < SENSORS_NUM; ++i )
  {
    pinStates[i] = isActive && ( sensors[i].Pressure > sensors[i].target );
    digitalWrite( sensors[0].relay_pin, (pinStates[i] ? LOW : HIGH) );
  }

}


//////////////////////////////////////////////////////////////////////////////////////////////


// Draw interface
void Draw() {
  display.clearDisplay();
  display.drawRoundRect(0, 0, 127, 63, 7, WHITE);
  display.drawLine(9, 16, 117, 16, WHITE);

  switch( currentState ) {
    case ErrorScreen: DrawError(); break;
    case InfoScreen: DrawInfo(); break;
    case MenuScreen: DrawMenu(); break;
    case SetVacuum: DrawSetVacuum(); break;
    default: break;
  }

  display.display();
}

// Draw error screen
void DrawError() {

  display.setTextColor(WHITE);
  display.setTextSize(1);

  // Header
  {
    string headerString = "ERROR";
    display.setCursor( ctr( SYMBOL_WIDTH*strlen(headerString),display.width() ), 5 );
    display.print( headerString );
  }

  // Error string
  {
    display.setCursor( ctr( SYMBOL_WIDTH*strlen(currentError),display.width() ), 20 );
    display.print( currentError );
  }

  // Footer
  {
    string resetString = "Push to reinit";
    display.setCursor( ctr( SYMBOL_WIDTH*strlen(resetString),display.width() ), 50 );
    display.print( resetString );
  }

}

// Draw info screen
void DrawInfo() {

  display.setTextColor(WHITE);
  display.setTextSize(1);

  // Header
  {
    string headerString;
    if( isActive ) {
      headerString = "STATUS:ACTIVE";
    }
    else {
      headerString = "STATUS:IDLE";
    }
    display.setCursor( ctr( SYMBOL_WIDTH*strlen(headerString), display.width() ), 5 );
    display.print( headerString );
  }

  // Sensors
  {
    const uint8_t LEN = SYMBOL_WIDTH*strlen("S0");
    const uint8_t OFFSET = 10;

    for( uint8_t i = 0; i < SENSORS_NUM; ++i ) {
      display.setCursor( dst( i, SENSORS_NUM, LEN, display.width()-OFFSET ) + OFFSET, 20 );
      display.print( "S" );
      display.print( i );
    }
  }

  // Sensor data
  {

    const uint8_t ROW_1 = 35;
    const uint8_t ROW_2 = 48;

    display.setCursor( 10, ROW_1 );
    display.print( "P:" );

    display.setCursor( 10, ROW_2 );
    display.print( "T:" );

    const uint8_t LEN = SYMBOL_WIDTH*strlen("000.00");
    const uint8_t OFFSET = 12;

    for( uint8_t i = 0; i < SENSORS_NUM; ++i ) {
      const uint8_t X_VAL = dst( i, SENSORS_NUM, LEN, display.width()-OFFSET ) + OFFSET;

      display.setCursor( X_VAL, ROW_1 );
      display.print( sensors[i].Pressure );
      display.setCursor( X_VAL, ROW_2 );
      display.print( sensors[i].Temperature );
    }
  }

}

// Draw menu screen
void DrawMenu() {

  display.setTextColor(WHITE);
  display.setTextSize(1);

  // Header
  {
    string headerString = "SETTINGS";
    display.setCursor( ctr( SYMBOL_WIDTH*strlen(headerString),display.width() ), 5 );
    display.print( headerString );
  }

  // Menu strings
  {
    const uint8_t X_START = 12;
    const uint8_t Y_START = 20;
    const uint8_t Y_OFFSET = 10;

    switch( currentMenuCategory ) {
      case MainMenu: {
        for( uint8_t i = 0; i < MenuItemCount; ++i ) {
          display.setCursor( X_START, Y_START + i*Y_OFFSET );

          // Print cursor position
          if( i == currentMenuItem ) {
              display.print( "> " );
          } else {
              display.print( "  " );
          }

          if( i == MenuStart && isActive ) {
            display.print( "Stop" );
          } else {
            display.print( MenuStrings[i] );
          }
        }
        break;
      }
      case VacuumMenu: {
        for( uint8_t i = 0; i < VacuumMenuItemCount; ++i ) {
          display.setCursor(X_START, Y_START + i*Y_OFFSET);

          // Print cursor position
          if( i == currentVacuumMenuItem ) {
              display.print( "> " );
          } else {
              display.print( "  " );
          }

          display.print( VacuumMenuStrings[i] );
        }
        break;
      }
    }
  }

}

// Draw vacuum set screen
void DrawSetVacuum() {

  // Header
  {
    string headerString;
    if( currentSensorToSet == 0 ) {
      headerString = "SET TOP PRESSURE";
    }
    else {
      headerString = "SET BOTTOM PRESSURE";
    }

    display.setTextColor(WHITE);
    display.setTextSize(1);

    display.setCursor( ctr( SYMBOL_WIDTH*strlen(headerString),display.width() ), 5 );
    display.print( headerString );
  }

  // Digit marker
  {
    uint8_t COORD_X;
    if( sensorValueModifier == 1.0 ) {
      COORD_X = 26;
    } else if( sensorValueModifier == 0.1 ) {
      COORD_X = 62;
    } else {
      COORD_X = 80;
    }

    display.drawRect( COORD_X, 25, 3*SYMBOL_WIDTH + 1, 27, WHITE );
  }

  // Set number
  {
    display.setTextSize(3);

    display.setCursor( ctr( 3*SYMBOL_WIDTH*strlen("0.00"),display.width() ), 28 );
    display.print( sensors[currentSensorToSet].target );
  }

}

// Draw intro screen
void DisplayTest(){

  display.drawRoundRect(0, 0, 127, 63, 7, WHITE);
  display.display();
  delay(500);
  display.setTextColor(WHITE);

  {
    string str = "SolidFill";
    display.setTextSize(2);
    display.setCursor(ctr(2*SYMBOL_WIDTH*strlen(str),display.width()),10);
    display.print(str);
    display.display();
    delay(250);
  }
  {
    string str = "VACUUM CASTING";
    display.setCursor(ctr(SYMBOL_WIDTH*strlen(str),display.width()),33);
    display.setTextSize(1);
    display.print(str);
    display.display();
    delay(350);
  }
  {
    string str = "Initializing";
    display.setCursor(ctr(SYMBOL_WIDTH*strlen(str),display.width()),49);
    display.print(str);
    display.display();
    delay(350);
  }
  {
    string str = "<              >";
    display.setCursor(ctr(SYMBOL_WIDTH*strlen(str),display.width()),49);
    display.print(str);
    display.display();
    delay(100);
  }
  {
    string str = "<               >";
    display.setCursor(ctr(SYMBOL_WIDTH*strlen(str),display.width()),49);
    display.print(str);
    display.display();
    delay(100);
  }
  {
    string str = "<                >";
    display.setCursor(ctr(SYMBOL_WIDTH*strlen(str),display.width()),49);
    display.print(str);
    display.display();
    delay(100);
  }
  {
    display.setTextColor(WHITE, BLACK);
    string str = "                  ";
    display.setCursor(ctr(SYMBOL_WIDTH*strlen(str),display.width()),49);
    display.print(str);
    display.display();
    delay(100);
  }
  {
    display.setTextColor(WHITE);
    string str = "SYSTEM READY";
    display.setCursor(ctr(SYMBOL_WIDTH*strlen(str),display.width()),49);
    display.print(str);
    display.display();
    delay(750);
  }
}


//////////////////////////////////////////////////////////////////////////////////////////////


// Set error screen
inline void SetError( string errorMessage ) {
  currentError = errorMessage;
  currentState = ErrorScreen;
  isActive = false;
}

// Center X coordinate of an object on screen
inline uint8_t ctr( uint8_t width, uint8_t totalWidth ) {
  return ( totalWidth - width ) / 2;
}

// Distribute on screen
inline uint8_t dst( uint8_t num, uint8_t total, uint8_t width, uint8_t totalWidth ) {
  uint8_t seg = totalWidth / total;
  return abs( seg - width ) / 2 + seg*num;
}

