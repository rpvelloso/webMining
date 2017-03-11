/*
 Copyright 2011 Roberto Panerai Velloso.
 This file is part of webMining.
 webMining is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 webMining is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with webMining.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Context.hpp"

#include <lua.h>
#include <luaconf.h>
#include <iostream>
#include <string>

#include "../DSRE/DSRE.hpp"
#include "../WebDriver/WebDriver.hpp"
#include "../WebDriver/FireFoxWebDriver.hpp"
#include "../WebDriver/ChromeWebDriver.hpp"
#include "DOM.hpp"

extern "C" {

LUALIB_API int (luaopen_lsqlite3)(lua_State *);

}

Context::Context(const std::string &inp, int argc, char **argv) {
  lua.open_libraries();

  auto script = lua.load_file(inp);

  if (script.status() == sol::load_status::ok) {
    bind(argc, argv);
    script();
  }

  if (script.status() != sol::load_status::ok) {
    std::cout << "Lua error: " << lua_tostring(lua.lua_state(), -1)
        << std::endl;
    lua_pop(lua.lua_state(), 1);
  }
}

Context::~Context() {
}

void Context::bind(int argc, char **argv) {
  DOM::luaBinding (lua);
  DSRE::luaBinding(lua);
  WebDriver::luaBinding(lua);

  luaopen_lsqlite3(lua.lua_state());
  lua_setglobal(lua.lua_state(), "sqlite3");

  lua.create_named_table("args");
  for (int i = 0; i < argc; ++i) {
    lua["args"][i + 1] = std::string(argv[i]);
  }
}
