.POSIX:
.SUFFIXES:
.PHONY: install uninstall clean run test

BIN       = flbng
CC        = cc
CFLAGS    = -Wall -Wextra -Werror -Os
PREFIX    = /usr/local
MANPREFIX = $(PREFIX)/share/man

OBJ       = main.o

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LDFLAGS) -o $(BIN) $(OBJ) $(LDLIBS)

run: $(BIN)
	./$(BIN)

test: $(BIN)
	./test.sh

install: $(BIN)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	install -m 775 $(BIN) $(DESTDIR)$(PREFIX)/bin/
	install -m 644 $(BIN).1 $(DESTDIR)$(MANPREFIX)/man1/

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(BIN)
	rm -f $(DESTDIR)$(MANPREFIX)/man1/$(BIN).1

clean:
	rm -f $(BIN) *.o

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<
