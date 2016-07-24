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

#include <getopt.h>
#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <exception>
#include <iostream>
#include <string>

#include "Context.hpp"

//#include "sol.hpp"

using namespace std;

void printUsage(char *p) {
  cout << "usage: " << p << " [-i input_file] [-debug]" << endl;
  cout << "-i      input file" << endl;
  cout << "-debug  output debug info to stderr" << endl;
  cout << "-h      shows help screen" << endl;
  exit(-1);
}

int main(int argc, char *argv[]) {
  int opt, dbg = 0;
  string script;

  while ((opt = getopt(argc, argv, "i:d:h")) != -1) {
    switch (opt) {
      case 'i':
        if (!optarg)
          printUsage(argv[0]);
        script = optarg;
        break;
      case 'd':
        if (optarg && string(optarg) == "ebug")
          dbg = 1;
        else
          printUsage(argv[0]);
        break;
      case 'h':
      default:
        printUsage(argv[0]);
        break;
    }
  }

  if (!dbg) {
    fclose (stderr);
    cerr.rdbuf(NULL);
  }

  if (script != "") {
    try {
      Context context(script, argc, argv);
    } catch (std::exception &e) {
      std::cout << "*** exception: " << e.what() << std::endl;
    }
  } else
    printUsage(argv[0]);

  cout.flush();
  return 0;
}
