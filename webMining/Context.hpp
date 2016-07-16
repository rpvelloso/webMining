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

#ifndef CONTEXT_HPP_
#define CONTEXT_HPP_

#include <set>
#include <lua.hpp>
#include "DSRE.hpp"
#include "DOM.hpp"
#include "sol.hpp"

class Context {
public:
	Context(const std::string &, int argc, char **argv);
	virtual ~Context();
protected:
	Context() = delete;
	sol::state lua;
};

#endif /* CONTEXT_HPP_ */