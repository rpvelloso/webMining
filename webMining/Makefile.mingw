CC =gcc
CFLAGS = -Wall -g3 -O0 -I/local/include -fPIC -DWIN32 -std=c11
CXXFLAGS = -g3 -O0 -I/local/include -I3rdparty/libtidy -I/c/WinPython/python-3.6.5.amd64/include \
	-fPIC -std=c++1y -D_USE_MATH_DEFINES -DWIN32
LD = g++
LDFLAGS = -L/local/lib -L/c/WinPython/python-3.6.5.amd64/libs -lcurl -llua -lgdi32 -lws2_32 -lpython36

OBJS = 3rdparty/Ckmeans.1d.dp.o  3rdparty/lsqlite3.o \
3rdparty/hsfft.o            3rdparty/sqlite3.o \
WebDriver/ChromeWebDriver.o   WebDriver/HTTPClient.o \
WebDriver/FireFoxWebDriver.o \
base/Context.o     base/Node.o                  base/util.o \
base/DOM.o         base/StructuredDataRegion.o \
base/DataRegion.o  base/main.o \
DSRE/DSRE.o         DSRE/DSRECenterStar.o         DSRE/DSREDataRegion.o \
DSRE/DSREAligner.o  DSRE/DSREContentClassifier.o

all	:	webMining.exe
webMining.exe	:	$(OBJS) libtidy
	$(LD) $(OBJS) 3rdparty/libtidy/libtidy.o -o $@ $(LDFLAGS)
libtidy	:
	$(MAKE) -C 3rdparty/libtidy
clean:
	rm -rf $(OBJS) webMining.exe
	make clean -C 3rdparty/libtidy
	
%.o	:	%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
%.o	:	%.cxx
%.o	:	%.cc
	