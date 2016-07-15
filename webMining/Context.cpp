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
#include "DSRE.hpp"
#include "Context.hpp"

extern "C" {

Context *context(lua_State *L) {
	lua_getglobal(L,"context");
	auto ctx = (Context *)lua_touserdata(L,-1);
	lua_pop(L,1);
	return ctx;
}

bool checkDOM(lua_State *L, DOM *d) {
	return context(L)->checkDOM(d);
}

static int lua_api_loadDOMTree(lua_State *L) {
	int nargs = lua_gettop(L);
	DOM *dom;

	if (nargs == 1) {
		if (lua_isstring(L,-1)) {
			dom = new DOM(lua_tostring(L,-1));
			if (dom->isLoaded()) {
				context(L)->insertDOM(dom);
				lua_pushlightuserdata(L,dom);
				return 1;
			} else
				delete dom;
		}
	}
	return 0;
}

static int lua_api_unloadDOMTree(lua_State *L) {
	int nargs = lua_gettop(L);

	if (nargs == 1) {
		if (lua_islightuserdata(L,-1)) {
			DOM *dom = (DOM *)lua_touserdata(L,-1);
			context(L)->removeDOM(dom);
		}
	}
	return 0;
}

static int lua_api_SRDExtract(lua_State *L) {
	int nargs = lua_gettop(L);

	if (nargs == 1) {
		if (lua_islightuserdata(L,-1)) {
			DOM *dom = (DOM *)lua_touserdata(L,-1);
			if (checkDOM(L,dom))
				context(L)->dsre.extract(dom);
		}
	}
	return 0;
}

static int lua_api_getRegionCount(lua_State *L) {
	int nargs = lua_gettop(L);

	if (nargs == 2) {
		DOM *dom = (DOM *)lua_touserdata(L,-2);
		std::string methodstr = lua_tostring(L,-1);

		lua_pushnumber(L,context(L)->dsre.regionCount());
		return 1;
	}
	return 0;
}

static int lua_api_getDataRegion(lua_State *L) {
	int nargs = lua_gettop(L);

	if (nargs == 3) {
		if (lua_islightuserdata(L,-3) && lua_isstring(L,-2) && lua_isnumber(L,-1)) {
			DOM *dom = (DOM *)lua_touserdata(L,-3);
			std::string methodstr = lua_tostring(L,-2);
			size_t dtr_no = lua_tonumber(L,-1);

			if (checkDOM(L,dom) &&
				(dtr_no < context(L)->dsre.regionCount()) &&
				((methodstr == "mdr") || (methodstr == "drde") || (methodstr == "srde"))) {

				auto tpsreg = context(L)->dsre.getDataRegion(dtr_no);

				// main table returned
				lua_createtable(L,0,0);

				// tps data of this data region
				if ((methodstr == "drde") || (methodstr == "srde")) {

					if (tpsreg.getTps().size()) {
						lua_pushstring(L,"tps");
						lua_createtable(L,tpsreg.getTps().size(),0);
						for (size_t i=0;i<tpsreg.getTps().size();i++) {
							lua_pushnumber(L,i+1);
							lua_pushnumber(L,tpsreg.getTps()[i]);
							lua_settable(L,-3);
						}
						lua_settable(L,-3);

						lua_pushstring(L,"a");
						lua_pushnumber(L,tpsreg.getLinearRegression().a);
						lua_settable(L,-3);
						lua_pushstring(L,"b");
						lua_pushnumber(L,tpsreg.getLinearRegression().b);
						lua_settable(L,-3);
						lua_pushstring(L,"e");
						lua_pushnumber(L,tpsreg.getLinearRegression().e);
						lua_settable(L,-3);
						lua_pushstring(L,"score");
						lua_pushnumber(L,tpsreg.getScore());
						lua_settable(L,-3);
						lua_pushstring(L,"content");
						lua_pushboolean(L,tpsreg.isContent());
						lua_settable(L,-3);
						lua_pushstring(L,"pos");
						lua_pushnumber(L,tpsreg.getStartPos());
						lua_settable(L,-3);
					}
				}

				// no. of cols of table "records"
				lua_pushstring(L,"cols");
				lua_pushnumber(L,tpsreg.recordSize());
				lua_settable(L,-3);

				// bidimensional table containing the records
				lua_pushstring(L,"records");
				lua_createtable(L,0,0);
				for (size_t rec_no = 0; rec_no < tpsreg.recordCount(); ++rec_no) {
					Record rec = tpsreg.getRecord(rec_no);
					lua_pushnumber(L,rec_no+1);
					lua_createtable(L,rec.size(),0);
					for (size_t i=0;i<rec.size();i++) {
						lua_pushnumber(L,i+1);
						if (rec[i])
							lua_pushstring(L,rec[i]->toString().c_str());
						else
							lua_pushstring(L,"[filler]");
						lua_settable(L,-3);
					}
					lua_settable(L,-3);
				}
				lua_settable(L,-3);

				// no. of rows of table "records"
				lua_pushstring(L,"rows");
				lua_pushnumber(L,tpsreg.recordCount());
				lua_settable(L,-3);

				return 1;
			}
		}
	}
	return 0;
}

static int lua_api_DOMTPS(lua_State *L) {
	int nargs = lua_gettop(L);

	if (nargs == 1) {
		if (lua_islightuserdata(L,-1)) {
			DOM *dom = (DOM *)lua_touserdata(L,-1);

			if (checkDOM(L,dom)) {
				std::wstring tps = context(L)->dsre.getTps();

				lua_createtable(L,tps.size(),0);
				for (size_t i=0;i<tps.size();i++) {
					lua_pushnumber(L,i+1);
					lua_pushnumber(L,tps[i]);
					lua_settable(L,-3);
				}
				return 1;
			}
		}
	}
	return 0;
}

static int lua_api_printDOM(lua_State *L) {
	int nargs = lua_gettop(L);

	if (nargs == 1) {
		if (lua_islightuserdata(L,-1)) {
			DOM *dom = (DOM *)lua_touserdata(L,-1);

			if (checkDOM(L,dom))
				dom->printHTML();
		}
	}
	return 0;
}

static int lua_api_printTPS(lua_State *L) {
	int nargs = lua_gettop(L);

	if (nargs == 1) {
		if (lua_islightuserdata(L,-1)) {
			DOM *dom = (DOM *)lua_touserdata(L,-1);

			if (checkDOM(L,dom))
				context(L)->dsre.printTps();
		}
	}
	return 0;
}

static int lua_api_nodeToString(lua_State *L) {
	int nargs = lua_gettop(L);

	if (nargs == 1) {
		if (lua_islightuserdata(L,-1)) {
			pNode node = (pNode)lua_touserdata(L,-1);

			lua_pushstring(L,node->toString().c_str());
			lua_pushstring(L,"");
			return 1;
		}
	}
	return 0;
}

LUALIB_API int (luaopen_lsqlite3)(lua_State *);

}

void Context::insertDOM(DOM *d) {
	if (d)
		domSet.insert(d);
}

void Context::removeDOM(DOM *d) {
	if (checkDOM(d)) {
		domSet.erase(d);
		delete d;
	}
}

bool Context::checkDOM(DOM *d) const {
	return domSet.count(d) > 0;
}

Context::Context(const std::string &inp, int argc, char **argv) {
	sol::state lua;

	lua.open_libraries();

	auto script = lua.load_file(inp);

	if (script.status() == sol::load_status::ok) {
		Node::luaBinding(lua);
		DOM::luaBinding(lua);
		DSREDataRegion::luaBinding(lua);
		DSRE::luaBinding(lua);

		luaopen_lsqlite3(lua.lua_state());
		lua_setglobal(lua.lua_state(),"sqlite3");

		lua_createtable(lua.lua_state(),argc,0);
		for (int i=0;i<argc;i++) {
			lua_pushnumber(lua.lua_state(),i+1);
			lua_pushstring(lua.lua_state(),argv[i]);
			lua_settable(lua.lua_state(),-3);
		}
		lua_setglobal(lua.lua_state(),"arg");

		LUA_SET_GLOBAL_LUDATA(lua.lua_state(),"context",this);
		LUA_SET_GLOBAL_CFUNC(lua.lua_state(),"loadDOMTree",lua_api_loadDOMTree);
		LUA_SET_GLOBAL_CFUNC(lua.lua_state(),"unloadDOMTree",lua_api_unloadDOMTree);
		LUA_SET_GLOBAL_CFUNC(lua.lua_state(),"SRDExtract",lua_api_SRDExtract);
		LUA_SET_GLOBAL_CFUNC(lua.lua_state(),"getDataRegion",lua_api_getDataRegion);
		LUA_SET_GLOBAL_CFUNC(lua.lua_state(),"getRegionCount",lua_api_getRegionCount);
		LUA_SET_GLOBAL_CFUNC(lua.lua_state(),"DOMTPS",lua_api_DOMTPS);
		LUA_SET_GLOBAL_CFUNC(lua.lua_state(),"printDOM",lua_api_printDOM);
		LUA_SET_GLOBAL_CFUNC(lua.lua_state(),"printTPS",lua_api_printTPS);
		LUA_SET_GLOBAL_CFUNC(lua.lua_state(),"nodeToString",lua_api_nodeToString);
		script();
	} else {
		std::cout << "Lua error: " << lua_tostring(lua.lua_state(), -1) << std::endl;
		lua_pop(lua.lua_state(), 1);
	}
}

Context::~Context() {
	for (auto i=domSet.begin();i!=domSet.end();i++) {
		delete (*i);
	}
	domSet.clear();
}
