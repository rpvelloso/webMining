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
#include <algorithm>
#include <sstream>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#ifndef WIN32
	#include <sys/wait.h>
#else
#include <winerror.h>
	#include <process.h>
	#include <io.h>
#endif
#include <libsockets/libsockets.h>
#include "thttpclientsocket.h"
#include "thttpthread.h"
#include "httpreply.h"

#define ERR_STR_LEN 100
#define PHP_BIN "/usr/bin/php-cgi"
#define CMD_BIN "C:\\Windows\\System32\\cmd.exe /c C:"

#ifdef WIN32 // under windows, this functions are thread-safe
	#define gmtime_r(i,j) memcpy(j,gmtime(i),sizeof(struct tm))
	#define strerror_r(i,j,k) strerror(i)
	#define pipe(p) _pipe(p,4096,0)
	#define fork() 0
	#define waitpid(p,r,n) _cwait(r,p,n)
#endif

static string weekDays[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static string months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

string time2str(time_t t) {
	struct tm tt;
	stringstream dtStr;

	gmtime_r(&t,&tt);
	dtStr << weekDays[tt.tm_wday] << ", " << tt.tm_mday << " " << months[tt.tm_mon] << " " << (1900+tt.tm_year) << " "
			<< tt.tm_hour << ":" << tt.tm_min << ":" << tt.tm_sec << " GMT";
	// data & time format: Mon, 31 Dec 1900 23:59:59 GMT
	return dtStr.str();
}

// TODO: add more MIME types
static string mime[][2] = {
		{".txt","text/plain"},
		{".htm.html","text/html"},
		{".gfi.jpg.jpeg.bmp.png.xpm.xbm","image/"},
		{".mp3.wav","audio/"},
		{".mpeg.avi.mp4.mkv.mpg.asf.flv","video/"},
		{".php.php3.phps","application/php"},
		{".css","text/css"},
#ifdef WIN32
		{".bat.cmd","application/batch"},
#endif
		{"", ""}
		};

string mimeType(string f) {
	size_t p,p2;
	string ret,ext="";

	p = f.find_last_of(".",f.length());
	if (p != string::npos) {
		p2 = f.find_last_of("/",f.length());
		if ((p2 != string::npos) && (p>p2)) {
			ext = f.substr(p+1,f.length()-p);
			lowerCase(ext);
		}
	}
	if (ext == "") return "application/octet-stream";
	else {
		for (int i=0;mime[i][0]!="";i++) {
			if (mime[i][0].find(ext,0)!=string::npos) {
				ret = mime[i][1];
				if (ret[ret.length()-1] == '/') ret = ret + ext;
				return ret;
			}
		}
	}
	return ("application/"+ext);
}

string unescapeUri(string uri) {
	size_t p;
	string hex;
	char c,*s;

	while ((p=uri.find('%',0)) != string::npos) {
		hex = "0x" + uri.substr(p+1,2);
		c = strtol(hex.c_str(),&s,16);
		uri.replace(p,3,1,c);
	}
	return uri;
}

tHTTPClientSocket::tHTTPClientSocket(int fd, sockaddr_in *sin) : tClientSocket(fd, sin) {
	log = NULL;
	msgOverflow = 0;
	hdrPos = 0;
	lineLength = 0;
	reqState = tHTTPReceiveHeader;
	tmpPostData = NULL;
}

tHTTPClientSocket::~tHTTPClientSocket() {
	if (tmpPostData) fclose(tmpPostData);
	Close();
}

void tHTTPClientSocket::setLog(tHTTPLog *l) {
	log = l;
}

void tHTTPClientSocket::onSend(void *buf, size_t *size) {
	if (*size == -1) {
		string *s = (string *)buf;

		log->log("sent to %s: %s",getHostname(),s->c_str());
	}
}

void tHTTPClientSocket::onReceive(void *buf, size_t size) {
	size_t i=0;

	for (;(i<size) && (reqState == tHTTPReceiveHeader);i++) {
		if (((char *)buf)[i] == ENDL) {
			if (!lineLength) {
				httpHeader[hdrPos] = 0x00;
				hdrPos = 0;
				if (msgOverflow) {
					msgOverflow = 0;
					LOG("HTTP header buffer overflow.\n");
				} else if (strlen(httpHeader) > 0) processHttpHeader();
			}
			lineLength = 0;
		} else {
			if (hdrPos >= HTTP_HDR_LEN) {
				hdrPos = 0;
				msgOverflow = 1;
			}
			if (((char *)buf)[i] != '\r') {
				httpHeader[hdrPos++] = ((char *)buf)[i];
				lineLength++;
			} else httpHeader[hdrPos++] = '\n';
		}
	}
	if (reqState == tHTTPReceiveBody) {
		size_t cpyLen = (size-i)>contentLength?contentLength:(size-i);
		fwrite(&(((char *)buf)[i]),cpyLen,1,tmpPostData);
		bodyPos += cpyLen;
		if (bodyPos == contentLength) processHttpBody();
	}
	if (reqState == tHTTPProcessRequest) processHttpRequest();
}

void tHTTPClientSocket::onConnect() {
}

void tHTTPClientSocket::onDisconnect() {
	log->log("connection to %s closed.\n",getHostname());
}

tHTTPServer *tHTTPClientSocket::getOwner() {
	return owner;
}

void tHTTPClientSocket::setOwner(tHTTPServer *o) {
	owner = o;
}

void tHTTPClientSocket::processHttpHeader() {
	string line, lineAux, parm, msg = httpHeader;
	int i=0;
	stringstream cl;
	string request;

	// reset header
	host = "";
	method = "";
	uri = "";
	query = "";
	httpVersion = "";
	userAgent = "";
	contentType = "";
	boundary = "";
	contentLength = 0;

	do { // TODO: parse header params better
		line = stringTok(&msg,"\n\r"); i++;
		lineAux = line;
		if (i == 1) { // request line
			method = stringTok(&line," ");
			request = stringTok(&line," ");
			if (request[0]=='/') request.erase(0,1);
			request = unescapeUri(owner->getDocumentRoot() + request);
			uri = stringTok(&request,"?");
			query = request;
			httpVersion = line;
		} else { // header lines
			parm = stringTok(&line,": ");
			upperCase(parm);
			if (parm == "HOST") {
				host = line;
			} else if (parm == "USER-AGENT") {
				userAgent = line;
			} else if (parm == "CONTENT-TYPE") {
				contentType = stringTok(&line,"; ");
				stringTok(&line,"=");
				boundary = line;
			} else if (parm == "CONTENT-LENGTH") {
				cl << line;
				cl >> contentLength;
			}
		}
	} while (lineAux != "");

	cout << "Method..: \'" << method << "\'" << endl
	<< "URI.....: \'" << uri << "\'" << endl
	<< "Query...: \'" << query << "\'" << endl
	<< "Version.: \'" << httpVersion << "\'" << endl
	<< "Host....: \'" << host << "\'" << endl
	<< "Agent...: \'" << userAgent << "\'" << endl
	<< "Length..: \'" << contentLength << "\'" << endl
	<< "Type....: \'" << contentType << "\'" << endl
	<< "Boundary: \'" << boundary << "\'" << endl;

	if (contentLength > 0) {
		if (tmpPostData) fclose(tmpPostData);
		tmpPostData = tmpfile();
		bodyPos = 0;
		reqState = tHTTPReceiveBody;
	} else reqState = tHTTPProcessRequest;
}

void tHTTPClientSocket::processHttpBody() {
	reqState = tHTTPProcessRequest;
}

void tHTTPClientSocket::processHttpRequest() {
	     if (method == "GET")     GET();
 	else if (method == "HEAD")    HEAD();
	else if (method == "POST")    POST();
	else if (method == "OPTIONS") OPTIONS();

	     /* unimplemented methods */
	else if (method == "PUT")     PUT();
	else if (method == "DELETE")  DEL();
	else if (method == "TRACE")   TRACE();
	else if (method == "CONNECT") CONNECT();
	else reply501NotImplemented();

	reqState = tHTTPReceiveHeader; // end of request: goes back to initial state
}

void tHTTPClientSocket::GET()
{
	off_t offset = 0;
	char **envp=NULL,*argv[3] = { NULL, NULL, NULL };
	size_t i=0,j=1;
	pid_t f;
	int cgiRet;
	string q,mt;
#ifndef WIN32
	char strerr[ERR_STR_LEN];
#else
	PROCESS_INFORMATION processInfo;
	STARTUPINFO startUpInfo;
#endif
	stringstream sstr;

	if ((query != "") && !access(uri.c_str(),X_OK)) {
		if (!(f=fork())) {
			while (i<query.length()) if (query[i++] == '&') j++;
			j = j + 17;
			envp = (char **)malloc(sizeof(void *) * j);
			i = 0; q = query;
			while (q != "") envp[i++] = strdup(stringTok(&q,"&").c_str());
			sstr << "CONTENT_LENGTH=" << contentLength;
			envp[i] = strdup(sstr.str().c_str());
			if (boundary != "")
				envp[i+ 1] = strdup(("CONTENT_TYPE=" + contentType + "; boundary=" + boundary).c_str());
			else
				envp[i+ 1] = strdup(("CONTENT_TYPE=" + contentType).c_str());
			sstr.str(""); sstr << "REMOTE_PORT=" << this->getPort();
			envp[i+ 2] = strdup(sstr.str().c_str());
			sstr.str(""); sstr << "SERVER_PORT=" << owner->getServerSocket()->getPort();
			envp[i+ 3] = strdup(sstr.str().c_str());
			envp[i+ 4] = strdup(("REMOTE_ADDR=" + this->getIP()).c_str());
			envp[i+ 5] = strdup(("SERVER_ADDR=" + owner->getServerSocket()->getIP()).c_str());
			envp[i+ 6] = strdup(("REQUEST_METHOD=" + method).c_str());
			envp[i+ 7] = strdup(("HTTP_HOST=" + host).c_str());
			envp[i+ 8] = strdup(("SERVER_NAME=" + host).c_str());
			envp[i+ 9] = strdup(("HTTP_USER_AGENT=" + userAgent).c_str());
			envp[i+10] = strdup("GATEWAY_INTERFACE=CGI/1.1");
			envp[i+11] = strdup(("QUERY_STRING=" + query).c_str());
			envp[i+12] = strdup(("REQUEST_URI=" + uri).c_str());
			envp[i+13] = strdup(("SERVER_PROTOCOL=" + httpVersion).c_str());
			envp[i+14] = strdup(("SCRIPT_FILENAME=" + uri).c_str());
			envp[i+15] = strdup(("DOCUMENT_ROOT=" + owner->getDocumentRoot()).c_str());
			envp[i+16] = NULL;
			mt = mimeType(uri);

			if (mt == "application/php") {
				argv[0] = strdup(PHP_BIN);
				argv[1] = strdup(uri.c_str());
#ifdef WIN32
			} else if (mt == "application/batch") {

				i=0;
				while (i<uri.length()) {
					if (uri[i]=='/') uri[i]='\\';
					i++;
				}

				argv[0] = strdup((CMD_BIN + uri).c_str());
#endif
			} else argv[0] = strdup(uri.c_str());

			if ((tmpPostData) && (contentLength > 0) && (method == "POST")) {
				fseek(tmpPostData,0,SEEK_SET);
#ifndef WIN32
				dup2(fileno(tmpPostData),fileno(stdin));
#endif
			}
#ifndef WIN32
			else dup2(this->socketFd,fileno(stdin));

			/* TODO: in the future change this redirection of stdout
			 * to the client socket. Instead of redirecting, use a
			 * tmp file to intermediate CGI output and detect if the
			 * CGI hasn't sent HTTP response header, so the server
			 * can send/complete it */
			dup2(this->socketFd,fileno(stdout));
			dup2(this->socketFd,fileno(stderr));
			execve(argv[0],argv,envp);
			LOG(strerror_r(errno,strerr,ERR_STR_LEN)); // execve() error
			exit(-1);
#endif
#ifdef WIN32
			ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));
			ZeroMemory(&startUpInfo, sizeof(STARTUPINFO));
			startUpInfo.cb = sizeof(STARTUPINFO);
			startUpInfo.hStdError = (HANDLE)(this->socketFd);
			startUpInfo.hStdOutput = (HANDLE)(this->socketFd);
			startUpInfo.hStdInput = tmpPostData?(HANDLE)_get_osfhandle(fileno(tmpPostData)):(HANDLE)(this->socketFd);
			startUpInfo.dwFlags |= STARTF_USESTDHANDLES;

			if (CreateProcess(NULL, argv[0], NULL, NULL, TRUE, 0, NULL, NULL, &startUpInfo, &processInfo)) {
				WaitForSingleObject(processInfo.hProcess,INFINITE);
				CloseHandle(processInfo.hProcess);
				CloseHandle(processInfo.hThread);
			} else reply500InternalError();
			for (i=0;i<j;i++) free(envp[i]);
			free(envp);
			free(argv[0]);
			if (argv[1]) free(argv[1]);
#endif
		} else if (f == -1) {
			LOG(strerror_r(errno,strerr,ERR_STR_LEN)); // fork() error
			reply500InternalError();
		} else {
			waitpid(f,&cgiRet,0);
			if (cgiRet) reply500InternalError();	// CGI has executed, but ended abnormally
		}
		Close();
	} else {
		if (!HEAD()) {
			if (sendFile(uri.c_str(),&offset,contentLength) <= 0) reply500InternalError();
		}
	}
	if (tmpPostData) fclose(tmpPostData);
	tmpPostData = NULL;
}

void tHTTPClientSocket::OPTIONS()
{
	Send(httpVersion + " 200 OK" + CRLF);
	replyServer();
	replyDate();
	Send("Content-length: 0" CRLF);
	Send("Allow: HEAD, GET, POST, OPTIONS" CRLF);
}

int tHTTPClientSocket::HEAD()
{
	struct stat st;
	stringstream len;
	string index;

	if (!stat(uri.c_str(),&st)) {
		if (S_ISDIR(st.st_mode)) { // TODO: create option to list files in dir with no index.html
			index = uri + "/index.htm";
			if (stat(index.c_str(),&st)) {
				index = uri + "/index.html";
				if (stat(index.c_str(),&st)) {
					reply404NotFound();
					return -1;
				}
			}
			uri = index;
		}
		if (access(uri.c_str(),R_OK)) {
			reply403Forbidden();
			return -1;
		}
		len << st.st_size;
		contentLength = st.st_size;
		Send(httpVersion + " 200 OK" + CRLF);
		replyServer();
		replyDate();
		Send("Content-length: " + len.str() + CRLF);
		Send("Content-type: " + mimeType(uri) + CRLF);
		if (index == uri) Send("Content-Location: " + index + CRLF);
		Send("Last-modified: " + time2str(st.st_mtime) + CRLF);
		Send("Accept-ranges: bytes" CRLF);
		Send(CRLF);
		return 0;
	} else {
		reply404NotFound();
		return -1;
	}
}

void tHTTPClientSocket::POST()
{
	GET();
}

void tHTTPClientSocket::PUT()
{
	reply501NotImplemented();
}

void tHTTPClientSocket::DEL()
{
	reply501NotImplemented();
}

void tHTTPClientSocket::TRACE()
{
	reply501NotImplemented();
}

void tHTTPClientSocket::CONNECT()
{
	reply501NotImplemented();
}

void tHTTPClientSocket::replyServer()
{
	Send("Server: httpd-libsockets-devel." CRLF);
}

void tHTTPClientSocket::replyDate()
{
	Send("Date: " + time2str(time(NULL)) + CRLF);
}

void tHTTPClientSocket::reply403Forbidden()
{
	string html = REPLY_403;
	stringstream len;

	len << html.length();
	Send(httpVersion + " 403 Forbidden." + CRLF);
	replyServer();
	replyDate();
	Send("Content-length: " + len.str() + CRLF);
	Send("Content-type: text/html" CRLF);
	Send("Connection: close" CRLF);
	Send(CRLF);
	Send(html);
}

void tHTTPClientSocket::reply404NotFound()
{
	string html = REPLY_404;
	stringstream len;

	len << html.length();
	Send(httpVersion + " 404 Not Found." + CRLF);
	replyServer();
	replyDate();
	Send("Content-length: " + len.str() + CRLF);
	Send("Content-type: text/html" CRLF);
	Send("Connection: close" CRLF);
	Send(CRLF);
	Send(html);
}

void tHTTPClientSocket::reply500InternalError()
{
	string html = REPLY_500;
	stringstream len;

	len << html.length();
	Send(httpVersion + " 500 Internal Server Error." + CRLF);
	replyServer();
	replyDate();
	Send("Content-length: " + len.str() + CRLF);
	Send("Content-type: text/html" CRLF);
	Send("Connection: close" CRLF);
	Send(CRLF);
	Send(html);
}

void tHTTPClientSocket::reply501NotImplemented()
{
	string html = REPLY_501;
	stringstream len;

	len << html.length();
	Send(httpVersion + " 501 Not Implemented." + CRLF);
	replyServer();
	replyDate();
	Send("Content-length: " + len.str() + CRLF);
	Send("Content-type: text/html" CRLF);
	Send("Connection: close" CRLF);
	Send(CRLF);
	Send(html);
}