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
#include "LuaContext.h"


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
	string inp="",outp="";
	fstream inputFile;
	filebuf outputFile;

	while ((opt = getopt(argc, argv, "i:d:h")) != -1) {
		switch (opt) {
		case 'i':
			if (!optarg)
				printUsage(argv[0]);
			inp = optarg;
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

	if (inp != "")
		delete (new LuaContext(inp.c_str()));
	else
		printUsage(argv[0]);

	cout.flush();
	return 0;
}
