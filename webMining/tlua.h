/*
 * tlua.h
 *
 *  Created on: 25/07/2014
 *      Author: rvelloso
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
	int checkDOM(DOM *);
protected:
	set<DOM *> dom_set;
};

#endif /* TLUA_H_ */
