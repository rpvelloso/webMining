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

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cmath>
#include <cerrno>
#include "misc.h"
#include "tlua.h"


using namespace std;

void printUsage(char *p)
{
	cout << "usage: "<<p<<" [-i input_file] [-o output_file] [-p pattern file] [-s search_string] [-v] [-f str] [-b] [-g]"<<endl;
	cout << "-i    input file (default stdin)"<<endl;
	cout << "-o    output file (default stdout)"<<endl;
	cout << "-p    pattern file to search for"<<endl;
	cout << "-s    search string: a list of tags to search for (tag1,tag2,...)"<<endl;
	cout << "-v    Verbose (do not abbreviate tags/text content." << endl;
	cout << "-f    text filter string" << endl;
	cout << "-g    mine forms and fields" << endl;
	cout << "-css  Use style definitions when creating tag path sequences." << endl;
	cout << "-z    LZ extraction." << endl << endl;
	exit(-1);
}

#define K 10

int main(int argc, char *argv[])
{
	int opt,dbg=0;
	string inp="",outp="",search="",pattern="",filterStr="";
	fstream inputFile;
	filebuf outputFile;

	while ((opt = getopt(argc, argv, "i:o:p:s:f:d:g")) != -1) {
		switch (opt) {
		case 'i':
			inp = optarg;
			break;
		case 'o':
			outp = optarg;
			break;
		case 'p':
			pattern = optarg;
			break;
		case 's':
			search = optarg;
			break;
		case 'f':
			filterStr = optarg;
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

	if (outp != "") {
		outputFile.open(outp.c_str(),ios_base::out|ios_base::binary);

		if (!errno) cout.rdbuf(&outputFile);
	}

	if (inp != "") {

		delete (new tlua(inp.c_str()));
		return 0;

	}

	cout.flush();
	return 0;
}

