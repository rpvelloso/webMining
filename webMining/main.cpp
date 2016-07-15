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
#include <unistd.h>
#include "DOM.hpp"
#include "DSRE.hpp"
//#include "Context.hpp"
#include "sol.hpp"


using namespace std;

void printUsage(char *p)
{
	cout << "usage: "<<p<<" [-i input_file] [-debug]"<<endl;
	cout << "-i      input file" << endl;
	cout << "-debug  output debug info to stderr" << endl;
	cout << "-h      shows help screen" << endl;
	exit(-1);
}

int main(int argc, char *argv[])
{
	int opt,dbg=0;
	string script;

	while ((opt = getopt(argc, argv, "i:d:h")) != -1) {
		switch (opt) {
		case 'i':
			if (!optarg)
				printUsage(argv[0]);
			script = optarg;
			break;
		case 'd':
			if (optarg && string(optarg) == "ebug") dbg=1;
			else  printUsage(argv[0]);
			break;
		case 'h':
		default:
	    	printUsage(argv[0]);
	    break;
		}
	}

	if (!dbg) {
		fclose(stderr);
		cerr.rdbuf(NULL);
	}

	if (script != "") {
		sol::state lua;

		lua.open_libraries(sol::lib::base);

		Node::luaBinding(lua);
		DOM::luaBinding(lua);
		DSREDataRegion::luaBinding(lua);
		DSRE::luaBinding(lua);

		std::string sc =
				"dom = DOM.new(\"../../datasets/tpsf/americanas.html\")\n"
				"if dom:isLoaded() then\n"
				"  print(\"carregou DOM\\n\")\n"
				"  dsre = DSRE.new()\n"
				"  dsre:extract(dom)\n"
				"  print(\"Total de regioes: \",dsre:regionCount(),\"\\n\")\n"
				"  for i=1,dsre:regionCount() do\n"
				"    dr = dsre:getDataRegion(i-1)\n"
				"    print(\"Regiao \",i,\": \",dr:recordCount(),\", \",dr:recordSize(),\"\\n\")\n"
				"  end\n"
				"  dr = dsre:getDataRegion(0)\n"
				"  rec = dr:getRecord(0)\n"
				"  for i=1,dr:recordSize() do\n"
				"    node = rec[i]\n"
				"    print(node:toString(),\"\\n\")\n"
				"  end\n"
				"--  dom:printHTML()\n"
				"end\n";

		std::cout << sc << std::endl;
		lua.script(sc);

		/*try {
			Context context(script, argc, argv);
		} catch (std::exception &e) {
			std::cout << e.what() << std::endl;
		}*/
	} else
		printUsage(argv[0]);

	cout.flush();
	return 0;
}
