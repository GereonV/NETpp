C:=g++
SRCEXT:=cpp
SRCDIR:=src
BINDIR:=bin
EXT:=
CFLAGS:=-std=c++20 -Wpedantic -Wall -Wextra -Wconversion -O3
ifeq '$(OS)' 'Windows_NT'
EXT:=.exe
CFLAGS+= -lWs2_32
endif

.PHONY: all server client udpserver udpclient dirs clean
all: server client udpserver udpclient
server: $(BINDIR)/server$(EXT)
client: $(BINDIR)/client$(EXT)
udpserver: $(BINDIR)/udpserver$(EXT)
udpclient: $(BINDIR)/udpclient$(EXT)
$(BINDIR)/%$(EXT): $(SRCDIR)/%.$(SRCEXT) $(SRCDIR)/*
	$(C) $< -o $@ $(CFLAGS)
dirs:
	mkdir -p $(dir $(BINDIR))
clean:
	rm -rf $(BINDIR)/*
