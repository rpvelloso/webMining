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

#ifndef TLUA_H_
#define TLUA_H_

#include <set>
#include "dom.hpp"

#define LUA_SET_GLOBAL_NUMBER(L,name,value) \
	do { \
		lua_pushnumber(L,value); \
		lua_setglobal(L,name); \
		} while (0)

#define LUA_SET_GLOBAL_STRING(L,name,value) \
	do { \
		lua_pushstring(L,value.c_str()); \
		lua_setglobal(L,name); \
		} while (0)

#define LUA_SET_GLOBAL_LUDATA(L,name,value) \
	do { \
		lua_pushlightuserdata(L,value); \
		lua_setglobal(L,name); \
		} while (0)

#define LUA_SET_GLOBAL_CFUNC(L,name,value) \
	do { \
		lua_pushcfunction(L,value); \
		lua_setglobal(L,name); \
		} while (0)

class tlua {
public:
	tlua(const char *);
	virtual ~tlua();
	void insertDOM(DOM *);
	void removeDOM(DOM *);
	bool checkDOM(DOM *);
protected:
	unordered_set<DOM *> domSet;
};

#endif /* TLUA_H_ */
