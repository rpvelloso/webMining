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

#ifndef CONTEXT_HPP_
#define CONTEXT_HPP_

#include <string>

#include "../3rdparty/sol.hpp"

class Context {
 public:
  Context(const std::string &, int argc, char **argv);
  virtual ~Context();
 protected:
  void bind(int argc, char **argv);
  Context() = delete;
  sol::state lua;
};

#endif /* CONTEXT_HPP_ */
