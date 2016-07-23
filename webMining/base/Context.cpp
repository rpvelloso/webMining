/*
    Copyright 2011 Roberto Panerai Velloso.
    This file is part of libsockets.
    libsockets is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    libsockets is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with libsockets.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <fstream>
#include "DOM.hpp"
#include "Node.hpp"
#include "Context.hpp"
#include "../DSRE/DSRE.hpp"

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
		std::cout << "Lua error: " << lua_tostring(lua.lua_state(), -1) << std::endl;
		lua_pop(lua.lua_state(), 1);
	}
}

Context::~Context() {
}

void Context::bind(int argc, char **argv) {
	Node::luaBinding(lua);
	DOM::luaBinding(lua);
	DSREDataRegion::luaBinding(lua);
	DSRE::luaBinding(lua);

	luaopen_lsqlite3(lua.lua_state());
	lua_setglobal(lua.lua_state(),"sqlite3");

	lua.create_named_table("args");
	for (int i = 0; i < argc; ++i) {
		lua["args"][i+1] = std::string(argv[i]);
	}
}
