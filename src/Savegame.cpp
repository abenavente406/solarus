/*
 * Copyright (C) 2006-2013 Christopho, Solarus - http://www.solarus-games.org
 * 
 * Solarus is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Solarus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "Savegame.h"
#include "SavegameConverterV1.h"
#include "MainLoop.h"
#include "lowlevel/FileTools.h"
#include "lowlevel/InputEvent.h"
#include "lowlevel/Debug.h"
#include "lua/LuaContext.h"
#include <lua.hpp>

namespace solarus {

const int Savegame::SAVEGAME_VERSION = 2;

const std::string Savegame::KEY_SAVEGAME_VERSION = "_version";         /**< Format of this savegame file. */
const std::string Savegame::KEY_STARTING_MAP = "_starting_map";        /**< Map id where to start the savegame. */
const std::string Savegame::KEY_STARTING_POINT = "_starting_point";    /**< Destination name on the starting map. */
const std::string Savegame::KEY_KEYBOARD_ACTION = "_keyboard_action";  /**< Keyboard key mapped to the action command. */
const std::string Savegame::KEY_KEYBOARD_ATTACK = "_keyboard_attack";  /**< Keyboard key mapped to the attack command. */
const std::string Savegame::KEY_KEYBOARD_ITEM_1 = "_keyboard_item_1";  /**< Keyboard key mapped to the item 1 command. */
const std::string Savegame::KEY_KEYBOARD_ITEM_2 = "_keyboard_item_2";  /**< Keyboard key mapped to the item 2 command. */
const std::string Savegame::KEY_KEYBOARD_PAUSE = "_keyboard_pause";    /**< Keyboard key mapped to the pause command. */
const std::string Savegame::KEY_KEYBOARD_RIGHT = "_keyboard_right";    /**< Keyboard key mapped to the right command. */
const std::string Savegame::KEY_KEYBOARD_UP = "_keyboard_up";          /**< Keyboard key mapped to the up command. */
const std::string Savegame::KEY_KEYBOARD_LEFT = "_keyboard_left";      /**< Keyboard key mapped to the left command. */
const std::string Savegame::KEY_KEYBOARD_DOWN = "_keyboard_down";      /**< Keyboard key mapped to the down command. */
const std::string Savegame::KEY_JOYPAD_ACTION = "_joypad_action";      /**< Joypad string mapped to the action command. */
const std::string Savegame::KEY_JOYPAD_ATTACK = "_joypad_attack";      /**< Joypad string mapped to the attack command. */
const std::string Savegame::KEY_JOYPAD_ITEM_1 = "_joypad_item_1";      /**< Joypad string mapped to the item 1 command. */
const std::string Savegame::KEY_JOYPAD_ITEM_2 = "_joypad_item_2";      /**< Joypad string mapped to the item 2 command. */
const std::string Savegame::KEY_JOYPAD_PAUSE = "_joypad_pause";        /**< Joypad string mapped to the pause command. */
const std::string Savegame::KEY_JOYPAD_RIGHT = "_joypad_right";        /**< Joypad string mapped to the right command. */
const std::string Savegame::KEY_JOYPAD_UP = "_joypad_up_key";          /**< Joypad string mapped to the up command. */
const std::string Savegame::KEY_JOYPAD_LEFT = "_joypad_left_key";      /**< Joypad string mapped to the left command. */
const std::string Savegame::KEY_JOYPAD_DOWN = "_joypad_down_key";      /**< Joypad string mapped to the down command. */
const std::string Savegame::KEY_CURRENT_LIFE = "_current_life";        /**< Number of life points. */
const std::string Savegame::KEY_CURRENT_MONEY = "_current_money";      /**< Amount of money. */
const std::string Savegame::KEY_CURRENT_MAGIC = "_current_magic";      /**< Number of magic points. */
const std::string Savegame::KEY_MAX_LIFE = "_max_life";                /**< Maximum allowed life points. */
const std::string Savegame::KEY_MAX_MONEY = "_max_money";              /**< Maximum allowed money. */
const std::string Savegame::KEY_MAX_MAGIC = "_max_magic";              /**< Maximum allowed magic points. */
const std::string Savegame::KEY_ITEM_SLOT_1 = "_item_slot_1";          /**< Name of the equipment item in slot 1. */
const std::string Savegame::KEY_ITEM_SLOT_2 = "_item_slot_2";          /**< Name of the equipment item in slot 2. */
const std::string Savegame::KEY_ABILITY_TUNIC = "_ability_tunic";      /**< Resistance level. */
const std::string Savegame::KEY_ABILITY_SWORD = "_ability_sword";      /**< Attack level. */
const std::string Savegame::KEY_ABILITY_SWORD_KNOWLEDGE =
    "_ability_sword_knowledge";                                        /**< Super spin attack ability level. */
const std::string Savegame::KEY_ABILITY_SHIELD = "_ability_shield";    /**< Protection level. */
const std::string Savegame::KEY_ABILITY_LIFT = "_ability_lift";        /**< Lift level. */
const std::string Savegame::KEY_ABILITY_SWIM = "_ability_swim";        /**< Swim level. */
const std::string Savegame::KEY_ABILITY_RUN = "_ability_run";          /**< Run level. */
const std::string Savegame::KEY_ABILITY_DETECT_WEAK_WALLS =
    "_ability_detect_weak_walls";                                      /**< Weak walls detection level. */
const std::string Savegame::KEY_ABILITY_GET_BACK_FROM_DEATH =
    "_ability_get_back_from_death";                                    /**< Resurrection ability level. */

/**
 * \brief Creates a savegame with a specified file name, existing or not.
 * \param main_loop The Solarus root object.
 * \param file_name Name of the savegame file (can be a new file),
 * relative to the quest write directory, with its extension.
 */
Savegame::Savegame(MainLoop& main_loop, const std::string& file_name):
  ExportableToLua(),
  file_name(file_name),
  main_loop(main_loop),
  equipment(*this),
  game(NULL) {

  const std::string& quest_write_dir = FileTools::get_quest_write_dir();
  Debug::check_assertion(!quest_write_dir.empty(),
      "The quest write directory for savegames was not set in quest.dat");

  if (!FileTools::data_file_exists(file_name)) {
    // This save does not exist yet.
    empty = true;
    set_initial_values();
  }
  else {
    // A save already exists, let's load it.
    empty = false;
    load();
  }
}

/**
 * \brief Destructor.
 */
Savegame::~Savegame() {
}

/**
 * \brief Returns whether this is a new save.
 * \return true if there is no savegame file with this name yet
 */
bool Savegame::is_empty() const {
  return empty;
}

/**
 * \brief Loads the initial values.
 */
void Savegame::set_initial_values() {

  // Set the savegame format version.
  set_integer(KEY_SAVEGAME_VERSION, SAVEGAME_VERSION);

  // Set the initial controls.
  set_default_keyboard_controls();
  set_default_joypad_controls();

  // Set the initial equipment.
  equipment.set_max_life(1);
  equipment.set_life(1);
  equipment.set_ability("tunic", 1);  // Mandatory to have a valid hero sprite.
}

/**
 * \brief Sets default values for the keyboard game controls.
 */
void Savegame::set_default_keyboard_controls() {

#if PANDORA
  set_string(KEY_KEYBOARD_ACTION, InputEvent::get_keyboard_key_name(InputEvent::KEY_PAGE_DOWN));
  set_string(KEY_KEYBOARD_ATTACK, InputEvent::get_keyboard_key_name(InputEvent::KEY_HOME));
  set_string(KEY_KEYBOARD_ITEM_1, InputEvent::get_keyboard_key_name(InputEvent::KEY_PAGE_UP));
  set_string(KEY_KEYBOARD_ITEM_2, InputEvent::get_keyboard_key_name(InputEvent::KEY_END));
  set_string(KEY_KEYBOARD_PAUSE, InputEvent::get_keyboard_key_name(InputEvent::KEY_LEFT_ALT));
#elif GCWZERO
  set_string(KEY_KEYBOARD_ACTION, InputEvent::get_keyboard_key_name(InputEvent::KEY_LEFT_ALT));//B
  set_string(KEY_KEYBOARD_ATTACK, InputEvent::get_keyboard_key_name(InputEvent::KEY_SPACE));//Y
  set_string(KEY_KEYBOARD_ITEM_1, InputEvent::get_keyboard_key_name(InputEvent::KEY_LEFT_SHIFT));//X
  set_string(KEY_KEYBOARD_ITEM_2, InputEvent::get_keyboard_key_name(InputEvent::KEY_LEFT_CONTROL));//A
  set_string(KEY_KEYBOARD_PAUSE, InputEvent::get_keyboard_key_name(InputEvent::KEY_RETURN));//START
#else
  set_string(KEY_KEYBOARD_ACTION, InputEvent::get_keyboard_key_name(InputEvent::KEY_SPACE));
  set_string(KEY_KEYBOARD_ATTACK, InputEvent::get_keyboard_key_name(InputEvent::KEY_c));
  set_string(KEY_KEYBOARD_ITEM_1, InputEvent::get_keyboard_key_name(InputEvent::KEY_x));
  set_string(KEY_KEYBOARD_ITEM_2, InputEvent::get_keyboard_key_name(InputEvent::KEY_v));
  set_string(KEY_KEYBOARD_PAUSE, InputEvent::get_keyboard_key_name(InputEvent::KEY_d));
#endif
  set_string(KEY_KEYBOARD_RIGHT, InputEvent::get_keyboard_key_name(InputEvent::KEY_RIGHT));
  set_string(KEY_KEYBOARD_UP, InputEvent::get_keyboard_key_name(InputEvent::KEY_UP));
  set_string(KEY_KEYBOARD_LEFT, InputEvent::get_keyboard_key_name(InputEvent::KEY_LEFT));
  set_string(KEY_KEYBOARD_DOWN, InputEvent::get_keyboard_key_name(InputEvent::KEY_DOWN));
}

/**
 * \brief Sets default values for the joypad game controls.
 */
void Savegame::set_default_joypad_controls() {

  set_string(KEY_JOYPAD_ACTION, "button 0");
  set_string(KEY_JOYPAD_ATTACK, "button 1");
  set_string(KEY_JOYPAD_ITEM_1, "button 2");
  set_string(KEY_JOYPAD_ITEM_2, "button 3");
  set_string(KEY_JOYPAD_PAUSE, "button 4");
  set_string(KEY_JOYPAD_RIGHT, "axis 0 +");
  set_string(KEY_JOYPAD_UP, "axis 1 -");
  set_string(KEY_JOYPAD_LEFT, "axis 0 -");
  set_string(KEY_JOYPAD_DOWN, "axis 1 +");
}

/**
 * \brief Reads the data from the savegame file.
 */
void Savegame::load() {

  // Try to parse as Lua first.
  lua_State* l = luaL_newstate();
  size_t size;
  char* buffer;
  FileTools::data_file_open_buffer(file_name, &buffer, &size);
  int load_result = luaL_loadbuffer(l, buffer, size, file_name.c_str());
  FileTools::data_file_close_buffer(buffer);

  // Call the Lua savegame file.
  if (load_result == 0) {
    // The buffer was successfully loaded.

    // Make the Lua world aware of this savegame object.
    lua_pushlightuserdata(l, this);
    lua_setfield(l, LUA_REGISTRYINDEX, "savegame");

    // Set a special environment to catch every variable declaration.
    lua_newtable(l);
                                    // fun env
    lua_newtable(l);
                                    // fun env env_mt
    lua_pushcfunction(l, l_newindex);
                                    // fun env env_mt __newindex
    lua_setfield(l, -2, "__newindex");
                                    // fun env env_mt
    lua_setmetatable(l, -2);
                                    // fun env
    lua_setfenv(l, -2);
                                    // fun

    if (lua_pcall(l, 0, 0, 0) != 0) {
      Debug::die(std::string("Failed to load savegame file '")
          + file_name + "': " + lua_tostring(l, -1));
      lua_pop(l, 1);
    }
  }
  else if (load_result == LUA_ERRSYNTAX) {
     // Apparently it was not a Lua file.
     // Let's try the obsolete format of Solarus 0.9.
     SavegameConverterV1 converter(file_name);
     converter.convert_to_v2(*this);
   }

  lua_close(l);
}

/**
 * \brief __newindex function of the environment of the savegame file.
 *
 * This special __newindex function catches declaration of global variables
 * to store them into the savegame.
 *
 * \param l The Lua context that is calling this function.
 * \return Number of values to return to Lua.
 */
int Savegame::l_newindex(lua_State* l) {

  lua_getfield(l, LUA_REGISTRYINDEX, "savegame");
  Savegame* savegame = static_cast<Savegame*>(lua_touserdata(l, -1));
  lua_pop(l, 1);

  const std::string& key = luaL_checkstring(l, 2);

  switch (lua_type(l, 3)) {

    case LUA_TBOOLEAN:
      savegame->set_boolean(key, lua_toboolean(l, 3));
      break;

    case LUA_TNUMBER:
      savegame->set_integer(key, int(lua_tointeger(l, 3)));
      break;

    case LUA_TSTRING:
      savegame->set_string(key, lua_tostring(l, 3));
      break;

   default:
      luaL_typerror(l, 3, "string, number or boolean");
  }

  return 0;
}

/**
 * \brief Saves the data into a file.
 */
void Savegame::save() {

  std::ostringstream oss;
  std::map<std::string, SavedValue>::iterator it;
  for (it = saved_values.begin(); it != saved_values.end(); it++) {
    const std::string& key = it->first;
    oss << key << " = ";
    const SavedValue& value = it->second;
    if (value.type == SavedValue::VALUE_BOOLEAN) {
      oss << (value.int_data ? "true" : "false");
    }
    else if (value.type == SavedValue::VALUE_INTEGER) {
      oss << value.int_data;
    }
    else {  // String.
      oss << "\"" << value.string_data << "\"";
    }
    oss << "\n";
  }

  const std::string& text = oss.str();
  FileTools::data_file_save_buffer(file_name, text.c_str(), text.size());
  empty = false;
}

/**
 * \brief Returns the name of the file where the data is saved.
 * \return the file name of this savegame
 */
const std::string& Savegame::get_file_name() const {
  return file_name;
}

/**
 * \brief Returns the Solarus main loop.
 * \return The main loop.
 */
MainLoop& Savegame::get_main_loop() {
  return main_loop;
}

/**
 * \brief Returns the Lua context where this savegame lives.
 * \return The Lua context.
 */
LuaContext& Savegame::get_lua_context() {
  return main_loop.get_lua_context();
}

/**
 * \brief Returns the player's equipment corresponding to this savegame.
 * \return The equipment.
 */
Equipment& Savegame::get_equipment() {
  return equipment;
}

/**
 * \brief Returns the player's equipment corresponding to this savegame.
 * \return The equipment.
 */
const Equipment& Savegame::get_equipment() const {
  return equipment;
}

/**
 * \brief If this savegame is currently running in a game, return that game.
 * \return A game or NULL.
 */
Game* Savegame::get_game() {
  return game;
}

/**
 * \brief Sets the game that is running this savegame.
 * \param game A game or NULL.
 */
void Savegame::set_game(Game* game) {
  this->game = game;
}

/**
 * \brief Notifies this savegame that its game starts.
 */
void Savegame::notify_game_started() {

  equipment.notify_game_started();
}

/**
 * \brief Notifies this savegame that its game is finished.
 */
void Savegame::notify_game_finished() {

  equipment.notify_game_finished();
}

/**
 * \brief Returns whether a saved value is a string.
 * \param key Name of the value to get.
 * \return true if this value exists and is a string.
 */
bool Savegame::is_string(const std::string& key) const {

  SOLARUS_ASSERT(LuaContext::is_valid_lua_identifier(key),
      std::string("Savegame variable '") + key + "' is not a valid key");

  bool result = false;
  std::map<std::string, SavedValue>::const_iterator it = saved_values.find(key);
  if (it != saved_values.end()) {
    const SavedValue& value = it->second;
    result = (value.type == SavedValue::VALUE_STRING);
  }
  return result;
}

/**
 * \brief Returns a string value saved.
 * \param key Name of the value to get.
 * \return The string value associated with this key or an empty string.
 */
const std::string& Savegame::get_string(const std::string& key) const {

  SOLARUS_ASSERT(LuaContext::is_valid_lua_identifier(key),
      std::string("Savegame variable '") + key + "' is not a valid key");

  std::map<std::string, SavedValue>::const_iterator it = saved_values.find(key);
  if (it != saved_values.end()) {
    const SavedValue& value = it->second;
    SOLARUS_ASSERT(value.type == SavedValue::VALUE_STRING,
        std::string("Value '") + key + "' is not a string");
    return value.string_data;
  }

  static const std::string empty_string = "";
  return empty_string;
}

/**
 * \brief Sets a string value saved.
 * \param key Name of the value to set.
 * \param value The string value to associate with this key.
 */
void Savegame::set_string(const std::string& key, const std::string& value) {

  Debug::check_assertion(LuaContext::is_valid_lua_identifier(key),
      std::string("Savegame variable '") + key + "' is not a valid key");

  saved_values[key].type = SavedValue::VALUE_STRING;
  saved_values[key].string_data = value;
}

/**
 * \brief Returns whether a saved value is an integer.
 * \param key Name of the value to get.
 * \return true if this value exists and is an integer.
 */
bool Savegame::is_integer(const std::string& key) const {

  SOLARUS_ASSERT(LuaContext::is_valid_lua_identifier(key),
      std::string("Savegame variable '") + key + "' is not a valid key");

  bool result = false;
  std::map<std::string, SavedValue>::const_iterator it = saved_values.find(key);
  if (it != saved_values.end()) {
    const SavedValue& value = it->second;
    result = (value.type == SavedValue::VALUE_INTEGER);
  }
  return result;
}

/**
 * \brief Returns a integer value saved.
 * \param key Name of the value to get.
 * \return The integer value associated with this key or 0.
 */
int Savegame::get_integer(const std::string& key) const {

  SOLARUS_ASSERT(LuaContext::is_valid_lua_identifier(key),
      std::string("Savegame variable '") + key + "' is not a valid key");

  int result = 0;
  std::map<std::string, SavedValue>::const_iterator it = saved_values.find(key);
  if (it != saved_values.end()) {
    const SavedValue& value = it->second;
    SOLARUS_ASSERT(value.type == SavedValue::VALUE_INTEGER,
        std::string("Value '") + key + "' is not an integer");
    result = value.int_data;
  }
  return result;
}

/**
 * \brief Sets an integer value saved.
 * \param key Name of the value to set.
 * \param value The integer value to associate with this key.
 */
void Savegame::set_integer(const std::string& key, int value) {

  Debug::check_assertion(LuaContext::is_valid_lua_identifier(key),
      std::string("Savegame variable '") + key + "' is not a valid key");

  saved_values[key].type = SavedValue::VALUE_INTEGER;
  saved_values[key].int_data = value;
}

/**
 * \brief Returns whether a saved value is a boolean.
 * \param key Name of the value to get.
 * \return true if this value exists and is a boolean.
 */
bool Savegame::is_boolean(const std::string& key) const {

  SOLARUS_ASSERT(LuaContext::is_valid_lua_identifier(key),
      std::string("Savegame variable '") + key + "' is not a valid key");

  bool result = false;
  std::map<std::string, SavedValue>::const_iterator it = saved_values.find(key);
  if (it != saved_values.end()) {
    const SavedValue& value = it->second;
    result = (value.type == SavedValue::VALUE_BOOLEAN);
  }
  return result;
}

/**
 * \brief Returns a boolean value saved.
 * \param key Name of the value to get.
 * \return The boolean value associated with this key or false.
 */
bool Savegame::get_boolean(const std::string& key) const {

  SOLARUS_ASSERT(LuaContext::is_valid_lua_identifier(key),
      std::string("Savegame variable '") + key + "' is not a valid key");

  bool result = false;
  std::map<std::string, SavedValue>::const_iterator it = saved_values.find(key);
  if (it != saved_values.end()) {
    const SavedValue& value = it->second;
    SOLARUS_ASSERT(value.type == SavedValue::VALUE_BOOLEAN,
        std::string("Value '") + key + "' is not a boolean");
    result = value.int_data != 0;
  }
  return result;
}

/**
 * \brief Sets a boolean value saved.
 * \param key Name of the value to set.
 * \param value The boolean value to associate with this key.
 */
void Savegame::set_boolean(const std::string& key, bool value) {

  Debug::check_assertion(LuaContext::is_valid_lua_identifier(key),
      std::string("Savegame variable '") + key + "' is not a valid key");

  saved_values[key].type = SavedValue::VALUE_BOOLEAN;
  saved_values[key].int_data = value;
}

/**
 * \brief Unsets a value saved.
 * \param key Name of the value to unset.
 */
void Savegame::unset(const std::string& key) {

  Debug::check_assertion(LuaContext::is_valid_lua_identifier(key),
      std::string("Savegame variable '") + key + "' is not a valid key");

  saved_values.erase(key);
}

/**
 * \brief Returns the name identifying this type in Lua.
 * \return The name identifying this type in Lua.
 */
const std::string& Savegame::get_lua_type_name() const {
  return LuaContext::game_module_name;
}

}

