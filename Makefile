CC     = g++
PWD    = $(shell pwd)
OUTPUT = $(shell pwd)
OUTDIR = .
CFLAGS = $(ARCH) -c -DNDEBUG -DHAVE_CONFIG_H -fPIC -DLINUX -I htmlcxx-0.84 -DPCRE_STATIC `xml2-config --cflags`
LDFLAGS= $(ARCH) -pthread `xml2-config --libs`
LDLIBS=-lpcrecpp -lpcre 
MYLIBS=

EXT_LIBS = /htmlcxx-0.84/html

JUSTEXTMODULE=parseFSM.cpp justext.cpp tools.cpp

all: $(EXT_LIBS) $(OUTDIR)/jusTextTest

clean:
	@rm -f -r $(OUTPUT)/*/*.o $(OUTDIR)/goldMinerTest

$(EXT_LIBS):
	@make -C .$@ -f $(PWD)/LIBRARY.Makefile obj lib ARCH=$(ARCH) OUTPUT=$(OUTPUT)$@/ CFLAGS="$(CFLAGS)"

$(OUTDIR)/jusTextTest:
	g++ test.cpp $(JUSTEXTMODULE) -I htmlcxx-0.84 -o $(OUTDIR)/jusTextTest  $(EXT_LIBS:%=$(OUTPUT)%/*.o) -Wl,-static -Wl,-Bdynamic $(LDFLAGS) $(LDLIBS) $(MYLIBS)
