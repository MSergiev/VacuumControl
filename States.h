#ifndef STATES_H
#define STATES_H

namespace States
{
  // System states
  enum class SystemState : uint8_t
  {
    InfoScreen,
    MenuScreen,
    SetVacuum,
    ErrorScreen,
    Count
  } system_state;
  
  // Menu types
  enum class MenuCategory : uint8_t
  {
    MainMenu,
    VacuumMenu,
    Count
  } menu_category;
  
  // Main menu items
  enum class MenuItem : uint8_t
  {
    MenuBack,
    MenuStart,
    MenuSetVacuum,
    Count
  } menu_item;
  
  // "Set vacuum" menu items
  enum class VacuumMenuItem : uint8_t
  {
    VacuumBack,
    Vacuum1Set,
    Vacuum2Set,
    Count
  } vacuum_menu_item;
}

#endif /* STATES_H */
