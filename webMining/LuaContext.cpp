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

#include "LuaContext.h"

#include <iostream>
#include <fstream>
#include "dom.hpp"
#include "node.hpp"
#include "SRDEFilter.h"

using namespace std;

extern "C" {

bool checkDOM(lua_State *L, DOM *d) {
	LuaContext *ctx;

	lua_getglobal(L,"context");
	ctx = (LuaContext *)lua_touserdata(L,-1);
	lua_pop(L,1);
	return ctx->checkDOM(d);
}

static int lua_api_loadDOMTree(lua_State *L) {
	int nargs = lua_gettop(L);
	LuaContext *ctx;
	DOM *dom;

	if (nargs == 1) {
		lua_getglobal(L,"context");
		ctx = (LuaContext *)lua_touserdata(L,-1);
		lua_pop(L,1);

		if (lua_isstring(L,-1)) {
			dom = new DOM(lua_tostring(L,-1));
			if (dom->isLoaded()) {
				ctx->insertDOM(dom);
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
	LuaContext *ctx;

	if (nargs == 1) {
		lua_getglobal(L,"context");
		ctx = (LuaContext *)lua_touserdata(L,-1);
		lua_pop(L,1);

		if (lua_islightuserdata(L,-1)) {
			DOM *dom = (DOM *)lua_touserdata(L,-1);
			if (checkDOM(L,dom)) {
				ctx->removeDOM(dom);
				delete dom;
			}
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
				dom->setExtractor(new SRDEFilter(dom));
		}
	}
	return 0;
}

static int lua_api_getRegionCount(lua_State *L) {
	int nargs = lua_gettop(L);

	if (nargs == 2) {
		DOM *dom = (DOM *)lua_touserdata(L,-2);
		string methodstr = lua_tostring(L,-1);
		ExtractorInterface *method;

		method = dom->getExtractor();
		lua_pushnumber(L,method->getRegionCount());
		return 1;
	}
	return 0;
}

static int lua_api_getDataRegion(lua_State *L) {
	int nargs = lua_gettop(L);

	if (nargs == 3) {
		if (lua_islightuserdata(L,-3) && lua_isstring(L,-2) && lua_isnumber(L,-1)) {
			DOM *dom = (DOM *)lua_touserdata(L,-3);
			string methodstr = lua_tostring(L,-2);

			if (checkDOM(L,dom) && ((methodstr == "mdr") || (methodstr == "drde") || (methodstr == "srde"))) {
				int dtr_no = lua_tonumber(L,-1);
				int rec_no=0;
				vector<pNode> rec;
				ExtractorInterface *method;
				tTPSRegion *tpsreg;

				method = dom->getExtractor();

				// main table returned
				lua_createtable(L,0,0);

				// tps data of this data region
				if ((methodstr == "drde") || (methodstr == "srde")) {
					tpsreg = method->getRegion(dtr_no);
					if (tpsreg && tpsreg->tps.size()) {
						lua_pushstring(L,"tps");
						lua_createtable(L,tpsreg->tps.size(),0);
						for (size_t i=0;i<tpsreg->tps.size();i++) {
							lua_pushnumber(L,i+1);
							lua_pushnumber(L,tpsreg->tps[i]);
							lua_settable(L,-3);
						}
						lua_settable(L,-3);

						lua_pushstring(L,"a");
						lua_pushnumber(L,tpsreg->lc.a);
						lua_settable(L,-3);
						lua_pushstring(L,"b");
						lua_pushnumber(L,tpsreg->lc.b);
						lua_settable(L,-3);
						lua_pushstring(L,"e");
						lua_pushnumber(L,tpsreg->lc.e);
						lua_settable(L,-3);
						lua_pushstring(L,"score");
						lua_pushnumber(L,tpsreg->score);
						lua_settable(L,-3);
						lua_pushstring(L,"content");
						lua_pushboolean(L,tpsreg->content);
						lua_settable(L,-3);
						lua_pushstring(L,"pos");
						lua_pushnumber(L,tpsreg->pos);
						lua_settable(L,-3);
					}
				}

				// no. of cols of table "records"
				lua_pushstring(L,"cols");
				lua_pushnumber(L,method->getRecord(dtr_no,0).size());
				lua_settable(L,-3);

				// bidimensional table containing the records
				lua_pushstring(L,"records");
				lua_createtable(L,0,0);
				while ((rec = method->getRecord(dtr_no,rec_no++)).size()) {
					lua_pushnumber(L,rec_no);
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
				lua_pushnumber(L,rec_no-1);
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
				wstring tps = dom->getExtractor()->getTagPathSequence(-1);

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

	if (nargs == 2) {
		if (lua_islightuserdata(L,-2) && lua_isboolean(L,-1)) {
			DOM *dom = (DOM *)lua_touserdata(L,-2);
			//bool verbose = lua_toboolean(L,-1);

			if (checkDOM(L,dom)) {
				//dom->setVerbose(verbose);
				dom->printHTML();
			}
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

}

void LuaContext::insertDOM(DOM *d) {
	if (d)
		domSet.insert(d);
}

void LuaContext::removeDOM(DOM *d) {
	if (checkDOM(d)) {
		domSet.erase(d);
		delete d;
	}
}

bool LuaContext::checkDOM(DOM *d) {
	return domSet.count(d) > 0;
}

LuaContext::LuaContext(const char *inp) {
	state = luaL_newstate();

	luaL_openlibs(state);

	int s = luaL_loadfile(state, inp);

	if (!s) {
		LUA_SET_GLOBAL_LUDATA(state,"context",this);
		LUA_SET_GLOBAL_CFUNC(state,"loadDOMTree",lua_api_loadDOMTree);
		LUA_SET_GLOBAL_CFUNC(state,"unloadDOMTree",lua_api_unloadDOMTree);
		LUA_SET_GLOBAL_CFUNC(state,"SRDExtract",lua_api_SRDExtract);
		LUA_SET_GLOBAL_CFUNC(state,"getDataRegion",lua_api_getDataRegion);
		LUA_SET_GLOBAL_CFUNC(state,"getRegionCount",lua_api_getRegionCount);
		LUA_SET_GLOBAL_CFUNC(state,"DOMTPS",lua_api_DOMTPS);
		LUA_SET_GLOBAL_CFUNC(state,"printDOM",lua_api_printDOM);
		LUA_SET_GLOBAL_CFUNC(state,"nodeToString",lua_api_nodeToString);
		s = lua_pcall(state, 0, LUA_MULTRET, 0);
	} else {
		cout << "Lua error: " << lua_tostring(state, -1) << endl;
		lua_pop(state, 1);
	}
	lua_close(state);
}

LuaContext::~LuaContext() {
	for (auto i=domSet.begin();i!=domSet.end();i++) {
		delete (*i);
	}
	domSet.clear();
}

