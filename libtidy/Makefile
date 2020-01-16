CC =gcc
CFLAGS = -Wall -O0 -g3 -DWIN32 -fPIC -std=c11
LD = ld
LDFLAGS = -r
OBJS = access.o    attrs.o     entities.o  language.o  pprint.o    tidylib.o \
alloc.o     buffio.o    fileio.o    lexer.o     sprtf.o     tmbstr.o \
attrask.o   charsets.o  gdoc.o      mappedio.o  streamio.o  utf8.o \
attrdict.o  clean.o     iconvtc.o   message.o   tagask.o    win32tc.o \
attrget.o   config.o    istack.o    parser.o    tags.o

all	:	libtidy.o
libtidy.o	:	$(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o libtidy.o
clean	:
	rm -rf $(OBJS) libtidy.o
%.o	:	%.c
	$(CC) $(CFLAGS) -c $< -o $@
