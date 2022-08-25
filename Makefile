C:=g++
SRCEXT:=cpp
SRCDIR:=src
BIN:=bin/program

CFLAGS:=-std=c++20 -Wpedantic -Wall -Wextra -Wconversion -O3
SRCS:=
ifeq '$(OS)' 'Windows_NT'
BIN:=$(BIN).exe
CFLAGS+= -lWs2_32
endif

.PHONY: client server dirs clean
client: SRCS+=$(SRCDIR)/client.$(SRCEXT)
client: $(BIN)
server: SRCS+=$(SRCDIR)/server.$(SRCEXT)
server: $(BIN)
udpclient: SRCS+=$(SRCDIR)/udpclient.$(SRCEXT)
udpclient: $(BIN)
udpserver: SRCS+=$(SRCDIR)/udpserver.$(SRCEXT)
udpserver: $(BIN)

$(BIN): clean dirs
	$(C) $(SRCS) -o $@ $(CFLAGS)
dirs:
	mkdir -p $(dir $(BIN))
clean:
	rm -rf $(BIN)
ifneq '$(OS)' 'Windows_NT'
INSTALLPATH=/usr/local/bin/$(notdir $(BIN))
.PHONY: install uninstall
install: release
	install -m 755 $(BIN) $(INSTALLPATH)
uninstall:
	rm -f $(INSTALLPATH)
endif
