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

#include <string.h>
#include <string>
#include <iostream>
#include <vector>
#include <set>
#include "tdom.h"
#include "misc.h"

#define TTL_STA 11
#define TTL_SYM 6

#define PARSE_ERROR(l,c,s) cerr << "parse error (" << state << ") at line " \
							<< line << " col " << col << endl; return -1;

#define MAX_TXT_SIZE 50

static string ltags[] = {
		"table","tbody","tfoot","thead","tr",
		"ul","ol","dl","frameset","colgroup",
		"dir","menu","select"};
set<string> listTags(ltags,ltags+13);

static string itags[] = {
		"tr","th","td","li","dt","dd","frame",
		"noframes","tbody","tfoot","thead","col",
		"colgroup","p","option"};
set<string> itemTags(itags,itags+15);

static string stags[] = {"img","br","meta","param","area","link","doctype"}; // tags without subtree
set<string> singleTags(stags,stags+7);

static string igtags[] = {"script","noscript","head","doctype"};
set<string> ignoreTags(igtags,igtags+4);

/* maps an item to it's containers */
static pair<string,string> limap[] = {
	pair<string,string>("frame","frameset"),
	pair<string,string>("noframes","frameset"),
	pair<string,string>("option","select"),
	pair<string,string>("li","ul"),
	pair<string,string>("li","ol"),
	pair<string,string>("li","dl"),
	pair<string,string>("li","dir"),
	pair<string,string>("li","menu"),
	pair<string,string>("dt","dl"),
	pair<string,string>("dd","dl"),
	pair<string,string>("tr","table"),
	pair<string,string>("tr","tbody"),
	pair<string,string>("tr","tfoot"),
	pair<string,string>("tr","thead"),
	pair<string,string>("td","tr"),
	pair<string,string>("th","tr"),
	pair<string,string>("tbody","table"),
	pair<string,string>("tfoot","table"),
	pair<string,string>("thead","table"),
	pair<string,string>("col","colgroup"),
	pair<string,string>("colgroup","table")
};
multimap<string,string> listItemMap(limap,limap+21);

tDOM::tDOM() {
	root = current = body = new tNode(-1,"");
	count = 0;
	verbose = 0;
	ignoring = "";
	formOpen = 0;
	nodeSequence.clear();
};

tDOM::~tDOM() {
	delete root;
};

int tDOM::searchTag(string tag, string text) {
	return searchString(root,tag,text,1);
}

void tDOM::searchPattern(tDOM *p, float st) {
	if (st > 0) {
		if (st > 1) st = 1;
		searchTree(root,p->getRoot()->nodes.front(),st);
	}
}

void tDOM::addNode(int tp, string tx) {
	tNode *c, *n = new tNode(tp,tx);

	if (ignoreTags.find(n->tagName) != ignoreTags.end()) {
		if ((ignoring == "") && (n->type == 0)) ignoring = n->tagName;
		else if ((ignoring == n->tagName) && (n->type == 1)) ignoring = "";
	}

	if (ignoring == "") {
		switch (n->type) {
			case 0: /* open */

				if (n->tagName=="form") {
					if (formOpen) break;
					else formOpen=1;
				}

				if (n->tagName == "body") body = n;

				/* auto close tags when nesting occurs */
				if (itemTags.find(n->tagName) != itemTags.end()) {
					c = current;
					while (1) {
						if (current == root) {
							current = c;
							break;
						}
						if (current->tagName == "form" || !multimap_pair_search(listItemMap,n->tagName,current->tagName))
							break;
						if (current->tagName == n->tagName) {
							current = current->parent;
							break;
						}
						current = current->parent;
					}
				}

				current->addNode(n);

				if (singleTags.find(n->tagName) == singleTags.end())
					if (n->text[n->text.size()-1] != '/') current = n;

				if (n->text[n->text.size()-1] == '/') n->text.erase(n->text.size()-1,1);

				count ++;
				break;

			case 1: /* close */
				if (n->tagName == "form") formOpen=0;
				c = current;
				while ((current != root) && (current->tagName != n->tagName))
					current = current->parent;
				if (current->tagName == n->tagName) {
					if (current != root) current = current->parent;
					//current->addNode(n); /* don't create close tags nodes to minimize tree size */
				} else {
					current = c; /* ignores unmatched close tag */
				}
				delete n;
				break;

			case 2: /* text */
				current->addNode(n);
				count ++;
				break;

			/*case 3: // comment
				current->addNode(n);
				count ++;
				break;*/
			default: break;
		}
	} else
		if (singleTags.find(ignoring) != singleTags.end()) ignoring = "";
}

void tDOM::printDOM() {
	printNode(root,0);
	cout << "Node count: " << count << endl;
}

int tDOM::scan(istream &htmlInput) {
	char c;
	int oldsta = 0, state = 0, line = 0, col = 0, close = 0, oc = 0; // oc = open comment
	string tagName = "", text = "", comment = "";
	int ct; // char type

	int transitionTable[TTL_SYM][TTL_STA] = {
		{ 0,10, 2, 2, 0, 2, 9, 9, 2, 9, 2},
		{-1, 5,-1,-1, 0,-1,-1,-1,-1,-1,-1},
		{-1, 3,-1,-1, 0,-1,-1,-1,-1,-1,-1},
		{-1,-1, 4,-1, 0,-1,-1, 4,-1,-1, 4},
		{ 1,-1, 1,-1, 0,-1,-1,-1,-1,-1, 1},
		{-1,-1,-1,-1, 0, 8, 7,-1, 9, 6,-1},
	};

	if (root) {
		delete root;
		root = current = new tNode(-1,"");
		count = 0;
	}

	c = htmlInput.get();
	while ((!htmlInput.eof()) && (state != -1)) {

		if (c == '\n') {
			line++;
			col = 0;
		} else col++;

		switch (state) {
		case 0:
			if (c == '<') ct = 4;
			else ct = 0;
			break;
		case 1:
			if (c == '!') ct = 1;
			else if (c == '/') ct = 2;
			else ct = 0;
			break;
		case 2:
		case 10:
			if (c == '>') ct = 3;
			else if (c == '<') ct = 4;
			else ct = 0;
			break;
		case 3:
		case 4:
			ct = 0;
			break;
		case 5:
		case 6:
		case 8:
		case 9:
			if (c == '-') ct = 5;
			else ct = 0;
			break;
		case 7:
			if (c == '>') ct = 3;
			else ct = 0;
			break;
		default:
			PARSE_ERROR(line,col,state);
			break;
		}

		if (state == 3) close = 1;
		if (state == 10) close = 0;

		oldsta = state;
		state = transitionTable[ct][state];

		if ((state == 2) || (state == 10)) tagName += c;
		if ((state == 9) && (state == oldsta)) oc = 1;
		if ((oldsta == 9) && (state != 9)) oc = 0;
		if (!state && !oldsta) text += c;
		if (oc) comment += c;
		if (state == 1) tagName = "";

		if (state == 4) {
			trim(tagName);
			trim(text);
			trim(comment);
			if (text != "") addNode(2, text);
			if (comment != "") addNode(3, comment);
			if (tagName != "") addNode(close == 1, tagName);

			tagName = "";
			text = "";
			comment = "";
		} else c = htmlInput.get();
	}

	trim(tagName);
	trim(text);
	trim(comment);
	if (text != "") addNode(2, text);
	if (comment != "") addNode(3, comment);
	if (tagName != "") addNode(close == 1, tagName);

	if (!((state >= 0) && (state < TTL_STA))) {
			PARSE_ERROR(line,col,state);
	}
	treeSize(root);
	treeDepth(root);
	current = root;
	return 0;
}

int tDOM::treeMatch(tNode *t, tNode *p) {
	if (t->compare(p)) {
		if (t->nodes.size() >= p->nodes.size()) {
			list<tNode *>::iterator i,j;
			size_t m=0;

			i = t->nodes.begin();
			j = p->nodes.begin();
			for (; j!=p->nodes.end(); i++, j++) {
				if ((*i)->compare(*j)) m += treeMatch(*i,*j);
			}
			return (m == p->nodes.size());
		}
	}
	return 0;
}

size_t tDOM::STM(tNode *a, tNode *b, tNode *rec)
{
	if (!a->compare(b)) return 0;
	else {
		int k=a->nodes.size();
		int n=b->nodes.size();
		int *m[k+1],i,j,r;
		list<tNode *>::iterator ii,jj;

		for (i=0;i<=k;i++) m[i] = new int[n+1];

		for (i=0;i<=k;i++) m[i][0]=0;
		for (j=0;j<=n;j++) m[0][j]=0;

		ii = a->nodes.begin();
		for (i=1;i<=k;i++,ii++) {
			jj = b->nodes.begin();
			for (j=1;j<=n;j++,jj++) {
				int z = m[i-1][j-1]+STM(*ii,*jj,rec);

				m[i][j] = max(max(m[i][j-1],m[i-1][j]),z);
			}
		}

		if (rec) treeAlign(a,b,m,rec);

		r = m[k][n]+1;

		for (i=0;i<=k;i++) delete m[i];

		return r;
	}
}

void tDOM::treeAlign(tNode* a, tNode* b, int **m, tNode *rec) {
	int pi,i,k=a->nodes.size();
	int pj,j,n=b->nodes.size();
	list<tNode *>::iterator ii,jj;
	int insert=1;

	ii = a->nodes.begin();
	jj = b->nodes.begin();

	cerr << k << "x" << n << endl << "\t\t";
	for (j=1;j<=n;j++,jj++) cerr << (*jj)->tagName << "\t";
	cerr << endl;

	ii--;
	for (i=0;i<=k;i++,ii++) {
		if (!i) cerr << "\t";
		else cerr << (*ii)->tagName << "\t";
		for (j=0;j<=n;j++) {
			cerr << m[i][j] << "\t";
		}
		cerr << endl;
	}
	cerr << "---" << endl;

	pi = i = k;
	pj = j = n;
	ii = a->nodes.end();
	jj = b->nodes.end();
	while (m[i][j]) {
		cerr << i << ", " << j << " ";
		if (m[i-1][j-1] >= m[i][j-1]) {
			if (m[i-1][j-1] >= m[i-1][j]) {
				pi = i-1; ii--;
				pj = j-1; jj--;
			} else {
				pi = i-1; ii--;
			}
		} else {
			if (m[i][j-1] >= m[i-1][j]) {
				pj = j-1; jj--;
			} else {
				pi = i-1; ii--;
			}
		}

		if (m[pi][pj] == m[i][j]) {
			if (j!=pj && i==pi) {
				cerr << "insert" << endl;
				if (insert) {
					ii = a->addNode(ii,*jj);
					(*ii)->align(*jj,rec);
				}
			} else {
				cerr << "mismatch" << endl;
				insert = 0;
			}
		} else {
			cerr << "match" << endl;
			if (ii==a->nodes.end()) {
				ii--;
				pi--;
			}
			if (jj==b->nodes.end()) {
				jj--;
				pj--;
			}

			(*ii)->align(*jj,rec);
			insert = 1;
		}

		i = pi;
		j = pj;
	}
}

list <tNode *> tDOM::getRecord(tNode * seed, tNode *rec) {
	list<tNode *> ret;
	list<tNode *>::iterator i;
	size_t recsize=0;

	getAlignment(seed,rec,ret);
	for (i=ret.begin();i!=ret.end();i++) recsize += ((*i)!=NULL);
	if (!recsize) ret.clear();
	return ret;
}

void tDOM::getAlignment(tNode *seed, tNode *rec, list<tNode *> &attrs) {
	list <tNode *>::iterator i;

	for (i=seed->nodes.begin();i!=seed->nodes.end();i++) {
		if ((*i)->nodes.size() == 0) {
			if ((*i)->alignments.find(rec) != (*i)->alignments.end()) {
				if ((*i)->alignments[rec]->parent->tagName == "a")
					attrs.push_back((*i)->alignments[rec]->parent);
				else
					attrs.push_back((*i)->alignments[rec]);
			} else attrs.push_back(NULL);
		} else getAlignment(*i,rec,attrs);
	}
}

void tDOM::searchTree(tNode *n, tNode *t, float st) {
	list<tNode *>::iterator i;

	if (n) {
		if (st == 1) { // threshold = 100%, exact match
			if (treeMatch(n,t)) onPatternFound(n,t,1);
		} else {
			float sim=(float)STM(n,t,NULL) / (float)t->size;
			if (sim >= st) onPatternFound(n,t,sim);
		}
		for (i = n->nodes.begin();i!=n->nodes.end();i++)
			searchTree(*i,t,st);
	}
}

int tDOM::searchString(tNode *n, string t, string tt, int callEvent) {
	list<tNode *>::iterator i;
	int r=0;

	if (n) {
		for (i = n->nodes.begin();i!=n->nodes.end();i++) {
			if ((*i)->compare(t) && (tt=="" || (*i)->text.find(tt)!=string::npos)) {
				if (callEvent) onTagFound(*i);
				r++;
			}
			r += searchString(*i,t,tt,callEvent);
		}
	}
	return r;
}

tNode *tDOM::getRoot() {
	return root;
}

void tDOM::printNode(tNode *n, int lvl) {
	list<tNode *>::iterator i;

	if (n) {
		for (int j=1;j<lvl;j++) cout << " ";
		switch (n->type) {
			case 0:
				cout << "<"; break;
			case 1:
				cout << "</"; break;
			case 2:
				cout << ""; break;
			case 3:
				cout << "<!--"; break;
			default: break;
		}

		if (n->type != -1) {
			if (n->type < 2) cout << n->tagName;
			if (n->text.size() > 0) {
				cout << " " << (verbose?n->text:n->text.substr(0,MAX_TXT_SIZE));
				if (!verbose && (n->text.size() > MAX_TXT_SIZE)) cout << " ...";
			}
			if (n->type == 3) cout << "--";
			if (n->type != 2) {
				if (!verbose) cout << " (" << n->depth << ", " << n->size << ")";
				cout << ">";
			}
			cout << endl;
		}

		i = n->nodes.begin();
		for (;i!=n->nodes.end();i++) printNode(*i,lvl+1);

		// close current tag
		if ((n->type != -1) && (singleTags.find(n->tagName) == singleTags.end())) {
			if (n->type != 2) for (int j=1;j<lvl;j++) cout << " ";
			if (n->type == 3) cout << "-->" << endl;
			else if (n->type != 2) {
				cout << "</" << n->tagName << ">" << endl;
			}
		}
	}
}

void tDOM::setVerbose(int v)
{
    verbose = v;
}

list<tDataRegion> tDOM::MDR(tNode *p, int k, float st, int mineRegions) {
	list<tDataRegion> ret;

	if (p->depth >= 3 && p->size > 1) {
		tNode *a,*b;
		int n=0,DRFound;
		size_t r=0;
		list<tNode *>::iterator j,f;
		float simTable[k+1][p->nodes.size()];
		tDataRegion bestDR, currentDR;
		vector<tNode *>v(p->nodes.begin(),p->nodes.end()); // Remaining children, not covered by any DR, to call MDR recursively

		// *** Initialize similarity table
		for (int x=1;x<=k;x++)
			for (size_t y=0;y<p->nodes.size();y++) simTable[x][y]=0;

		// *** Create fake parent nodes to group children nodes of 'p' for STM()
		a = new tNode(0,"");
		b = new tNode(0,"");

		// *** Combine & Compare
		f = p->nodes.begin();
		for (int ff=0;f!=p->nodes.end();f++, ff++) { // iterate the start child of 'p'
			for (int i=ff+1;i<=k;i++) { // iterate group size
				a->clear();
				b->clear();
				j = f; n = 0;
				for (int jj=0;j!=p->nodes.end();j++,jj++) { // iterate from 'f' child to the end

					if (!(jj%i)) n = !n; // Combine nodes under fake parents (a and b) in groups of size 'i'
					if (n) a->addNode(*j);
					else b->addNode(*j);

					if (a->nodes.size() == b->nodes.size()) {
						int score = STM(a,b,NULL)-1; // subtract 1 match (a=b)

						simTable[i][ff+jj-(2*i)+1] = ((float)score / ((float)max(a->size,b->size)-1)); // subtract 1 node from size (the fake parent)

						if (!n) a->clear();
						else b->clear();
					}
				}
			}
		}
		a->clear();
		b->clear();
		delete a;
		delete b;

		if (p->nodes.size()>1) {
			cerr << "* --- Sim Table --- *" << endl;
			for (int x=1;x<=k;x++) {
				for (size_t y=0;y<p->nodes.size()-1;y++) {
					cerr << (simTable[x][y])*100 << ";";
				}
				cerr << endl;
			}
			cerr << endl;
		}

		// *** Identify Data Regions
		for (size_t x=0;x<p->nodes.size()-1;x++) {
			bestDR.clear();
			bestDR.start = p->nodes.size();
			for (int y=1;y<=k;y++) {
				currentDR.clear();
				DRFound = 0;
				for (size_t z=x;z<p->nodes.size();z++/*=y*/) {
					if (!DRFound && (z > bestDR.start)) break; // interrupts if nothing was found before 'best' DR
					if (simTable[y][z] >= st) {
						if (!DRFound) {
							currentDR.groupSize=y; // number of combined nodes to form the data region
							currentDR.start=z; // start node
							DRFound = 1;
						}
						z+=y-1;
					} else {
						if (DRFound) {
							currentDR.end=z+y-1; // end node
							currentDR.DRLength = currentDR.end - currentDR.start + 1; // length of DR

							cerr << "L(" << currentDR.groupSize << ") - [" << currentDR.start << "," << currentDR.end << "]" << endl;

							if ((currentDR.DRLength > bestDR.DRLength) &&
								((currentDR.start <= bestDR.start) || (bestDR.DRLength == 0)))
								bestDR=currentDR;

							break;
						}
					}
				}
			}
			if (bestDR.DRLength) {
				cerr << "best: L(" << bestDR.groupSize << ") [" << bestDR.start << "," << bestDR.end << "]" << endl;
				bestDR.s = std::find(p->nodes.begin(),p->nodes.end(),v[bestDR.start-r]);
				bestDR.e = ++std::find(p->nodes.begin(),p->nodes.end(),v[bestDR.end-r]);
				bestDR.p = p;
				v.erase(v.begin()+bestDR.start-r,v.begin()+bestDR.end-r+1);
				r += bestDR.end - bestDR.start + 1;
				ret.push_back(bestDR);
				if (mineRegions) onDataRegionFound(bestDR,k,st);
				x=bestDR.end;
			}
		}

		if (p->nodes.size()>1) cerr << "* --- end --- *" << endl << endl;

		for (r=0;r<v.size();r++) {
			if (v[r]->nodes.size()) {
				list<tDataRegion> dr = MDR(v[r],k,st,mineRegions);
				ret.insert(ret.end(),dr.begin(),dr.end());
			}
		}
	}
	return ret;
};

tNode *tDOM::findNext(tNode *n, string t) {
	list<tNode *>::iterator i;
	tNode * r=NULL;

	if (!n) n = current;
	if (n) {
		for (i = n->nodes.begin();i!=n->nodes.end();i++) {
			if ((*i)->compare(t)) return (*i);
			if ((r=findNext(*i,t))) return r;
		}
	}
	return r;
}

void tDOM::setRoot(tNode *r)
{
	root = r;
}

bool compareNodesSize(tNode *a, tNode *b) {
	cerr << a << " " << b << endl;
	return (a->getSize()>=b->getSize());
}

vector<tNode *> tDOM::partialTreeAlignment(tDataRegion dr) {
	list<tNode *>::iterator i;
	vector<tNode *> trees;
	tNode *rec = NULL;
	int seedSize=-1;

	if (dr.groupSize > 1) {
		for (i=dr.s;i!=dr.e;) {
			rec = new tNode(0,"");
			rec->depth = 3;
			for (size_t j=0;j<dr.groupSize;j++,i++) {
				rec->addNode(*i);
			}
			if (rec->size > 2) trees.push_back(rec);
		}
	} else {
		if ((*(dr.s))->size > 1) {
			trees.insert(trees.begin(),dr.s,dr.e);
			for (;dr.s!=dr.e;dr.s++) cerr << *dr.s << endl;
		}
	}

	if (trees.size() > 1) {
		std::stable_sort(trees.begin(),trees.end(),compareNodesSize); // seed == trees[0]

		STM(trees[0],trees[0],trees[0]);
		while (seedSize != trees[0]->size) {
			seedSize = trees[0]->size;
			for (size_t j=1;j<trees.size();j++)
				STM(trees[0],trees[j],trees[j]);
		}
	}
	return trees;
}

string tDOM::getRegEx(tNode* seed, int reccount, int lvl) {
	string ret="",s;

	for (list<tNode *>::iterator i=seed->nodes.begin();i!=seed->nodes.end();i++) {
		ret = ret + "\n";
		for (int j=0;j<lvl;j++) ret = ret + "  ";
		ret = ret + ((*i)->type==0?"<":"") + (*i)->tagName + ((*i)->type==0?">":"") + (((*i)->matches == reccount)?"":"?");
		s = getRegEx((*i),(*i)->matches,lvl+1);
		if (s != "") {
			for (int j=0;j<=lvl;j++) s = "  " + s;
			ret = ret + "(\n" + s + ") ";
		} else ret = ret + " ";
	}
	return trim(ret);
}

void tDOM::onDataRegionFound(tDataRegion region, int K, float st) {
	list<tNode *>::iterator i=region.s;
	tNode *n = new tNode(0,"");
	list<tDataRegion> records;

	n->depth = 3;

	if (region.groupSize > 1) {
		size_t j=0,k;

		while (j<region.DRLength) {
			n->clear();
			for (k=0;k<region.groupSize;k++,j++,i++)
				n->addNode(*i);
			records = MDR(n,K,st,0);
			if (records.size() == 1 && records.front().groupSize < region.groupSize) {
				region.groupSize = (*(records.begin())).groupSize;
				region.DRLength = (*(records.begin())).DRLength;
				break;
			}
		}
	} else {
		for (i=region.s;i!=region.e;i++) {
			list<tNode *>::iterator l=i;

			records = MDR((*i),K,st,0);
			if (records.size()==1 && records.front().DRLength == n->nodes.size()) {
				n->clear();
				for (i=region.s;i!=region.e;i++) {
					for (l=(*i)->nodes.begin();l!=(*i)->nodes.end();l++) {
						n->addNode(*l);
					}
				}
				region.clear();
				region.p = n;
				region.s = n->nodes.begin();
				region.e = n->nodes.end();
				region.groupSize = records.front().groupSize;
				region.DRLength = n->nodes.size();
				break;
			}
		}
	}

	onDataRecordFound(region);
	n->clear();
	delete n;
}

int tDOM::treeSize(tNode* n) {
	list<tNode *>::iterator i;
	int s=0;

	for (i=n->nodes.begin();i!=n->nodes.end();i++) {
		s += treeSize(*i);
	}
	n->size = s + 1;
	return n->size;
}

int tDOM::treeDepth(tNode* n) {
	list<tNode *>::iterator i;
	int d=0,d1;

	for (i=n->nodes.begin();i!=n->nodes.end();i++) {
		d1 = treeDepth(*i);
		if (d1>d) d = d1;
	}
	n->depth = d+1;
	return n->depth;
}

// naive lz decomposition
template<class T>
void lz_decomp(T &inp) {
	size_t len = inp.size(),blk=0;

	cout << endl << "LZ decomposition: " << endl;
	for (size_t i=0;i<len;i++) {
		T p,s;
		size_t k,l;

		s=inp.substr(i,len);
		p=inp.substr(0,i);

		k=l=0;
		for (size_t j=0;j<=s.size();j++) {
			size_t pos;

			pos=p.find(s.substr(0,j));
			if (pos != T::npos) {
				k=pos;
				l=j;
			}
		}

		++blk;
		if (l>=0) {
			cout << blk << "\t";
			if (k) {
				cout << k << "\t" << l << "\t";
				for (size_t m=0;m<l;m++) cout << s[m] << ((m!=l-1)?",":"");
				i+=l-1;
			} else cout << i << "\t" << 1 << "\t" << inp[i];
			cout << endl;
		}
	}

	/* TODO:
	 * 1. armazenar blocos da decomposicao;
	 * 2. para cada bloco,
	 * 3.	se houver repeticao; e
	 * 4.	esta tiver tamanho razoavel; e
	 * 5.	ela nao ocorrer em nenhum outro bloco seguinte da decomposicao;
	 * 6.	entao esta eh uma repeticao primitiva
	 *
	 * investigar - deteccao de diferentes regioes da pagina, atraves da mudanca do alfabeto,
	 * quando a intersecacao dos alfabetos for vazia. clustering?
	 * 1. iniciar com um conjunto contendo todo o alfabeto;
	 * 2. decrementar numero de ocorrencias a cada uma encontrada;
	 * 3. remover do conjunto inicial e inserir no proximo conjunto quando numero de ocorrencias zerar;
	 * 4. quando a interseccao for vazia, uma nova regiao inicia;
	 * 5. excluir do conjunto incial, simbolos do alfabeto com poucas repeticoes (ruido)
	 */
}

int tDOM::nodeSequenceSize(vector<tNode *> &ns, size_t b, size_t e) {
	int depth = ns[0]->depth;
	int size = 0;

	for (size_t i=b;i<=e && i < ns.size();i++) {
		if (ns[i]->depth <= depth) {
			size += ns[i]->size;
			depth = ns[i]->depth;
		}
	}

	cout << "[" << b << "," << e << "] = " << size << endl;
	return e-b;//size;
}

void tDOM::noiseFilter(wstring s) {
	set<int> alphabet,filteredAlphabet,subAlphabet,intersect;
	map<int,int> symCount,initialSymCount;
	int regionCount=0,threshold=3;
	size_t div;

	for (size_t i=0;i<s.size();i++) {
		if (alphabet.find(s[i]) == alphabet.end()) {
			initialSymCount[s[i]]=0;
			alphabet.insert(s[i]);
		}
		initialSymCount[s[i]]++;
	}
	filteredAlphabet = alphabet;

	while (regionCount != 2) {
		filteredAlphabet.clear();
		for (map<int,int>::iterator j=initialSymCount.begin();j!=initialSymCount.end();j++) {
			if ((*j).second > threshold) filteredAlphabet.insert((*j).first);
		}
		if (filteredAlphabet.size() < 2) break;
		threshold++;

		for (set<int>::iterator j=filteredAlphabet.begin();j!=filteredAlphabet.end();j++) {
			cout << *j << "\t" << initialSymCount[*j] << endl;
		}

		subAlphabet.clear();
		symCount = initialSymCount;
		regionCount = 0;
		for (size_t i=0;i<s.size();i++) {
			subAlphabet.insert(s[i]);
			if (filteredAlphabet.find(s[i]) != filteredAlphabet.end()) {
				symCount[s[i]]--;
				if (symCount[s[i]]==0) {
					filteredAlphabet.erase(s[i]);
					set_intersection(filteredAlphabet.begin(),filteredAlphabet.end(),subAlphabet.begin(),subAlphabet.end(),inserter(intersect,intersect.begin()));

					cout << "s[i]=" << s[i] << endl;
					for (set<int>::iterator j=filteredAlphabet.begin();j!=filteredAlphabet.end();j++) cout << *j << " ";
					cout << endl;
					for (set<int>::iterator j=subAlphabet.begin();j!=subAlphabet.end();j++) cout << *j << " ";
					cout << endl;
					for (set<int>::iterator j=intersect.begin();j!=intersect.end();j++) cout << *j << " ";
					cout << endl << endl;

					if (intersect.empty()) {
						cout << "region detected (" << subAlphabet.size() << "): ";
						for (set<int>::iterator j=subAlphabet.begin();j!=subAlphabet.end();j++) cout << (*j) << " ";
						cout << endl;
						subAlphabet.clear();
						regionCount++;
						if (regionCount == 1) div=i;
					}
					intersect.clear();
				}
			}
		}
	}

	if (regionCount == 2) {
		if (nodeSequenceSize(nodeSequence,0,div) < nodeSequenceSize(nodeSequence,div+1,nodeSequence.size())) {
			tNode *n = nodeSequence[div+1];
			int d = n->depth;

			s = s.substr(div+1,s.size());
			body->nodes.clear();
			while (++div < nodeSequence.size()) {
				if (nodeSequence[div]->depth <= d) {
					nodeSequence[div]->parent->nodes.remove(nodeSequence[div]);
					nodeSequence[div]->parent = NULL;
					body->addNode(nodeSequence[div]);
					//d = nodeSequence[div]->depth;
				}
			}
		} else {
			tNode *n = nodeSequence[div+1];
			int d = n->depth;

			s = s.substr(0,div);
			while (++div < nodeSequence.size()) {
				if (nodeSequence[div]->depth <= d) {
					nodeSequence[div]->parent->nodes.remove(nodeSequence[div]);
					//d = nodeSequence[div]->depth;
				}
			}
		}
		noiseFilter(s);
	}
	cout << "Threshold: " << threshold << endl;
}

void tDOM::printTagPath(string s, tNode *n) {
	list<tNode *>::iterator i = n->nodes.begin();
	int p;

	if (s == "") {
		pathCount = 0;
		tagPathMap.clear();
		tagPathSequence.clear();
		nodeSequence.clear();
	}

	p = pathCount;

	s = s + "/" + n->getTagName();

	if (tagPathMap.find(s) == tagPathMap.end()) {
		pathCount++;
		tagPathMap[s] = pathCount;
	}
	tagPathSequence = tagPathSequence + wchar_t(tagPathMap[s]);
	nodeSequence.push_back(n);

	cout << tagPathSequence.size()-1 << ":" << tagPathMap[s] << ":\t" << s << endl;

	if (!(n->nodes.size())) return;

	for (;i!=n->nodes.end();i++)
		printTagPath(s,*i);

	if (!p) {
		/*for (size_t k=0;k<tagPathSequence.size();k++)
			cout << tagPathSequence[k];
		cout << endl;*/
		//lz_decomp(tagPathSequence);
		noiseFilter(tagPathSequence);
		printNode(body,0);
	}
}