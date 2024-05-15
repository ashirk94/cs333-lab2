CC = gcc
DEBUG = -g
CFLAGS = $(DEBUG) -Wall -Wextra -Wshadow -Wunreachable-code -Wredundant-decls \
-Wmissing-declarations -Wold-style-definition \
-Wmissing-prototypes -Wdeclaration-after-statement \
-Wno-return-local-addr -Wunsafe-loop-optimizations \
-Wuninitialized -Werror
LDFLAGS = -lz
PROG1 = viktar
PROGS = $(PROG1)

all: $(PROGS)

$(PROG1): $(PROG1).o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(PROG1).o: $(PROG1).c
	$(CC) $(CFLAGS) -c $<

clean cls:
	rm -f $(PROGS) *.o *~ \#* *.viktar *.txt *.bin *.err *.out *.jerr *.serr *.sout *.jout

tar:
	tar cvfa Lab2_${LOGNAME}.tar.gz *.[ch] [mM]akefile *.bash

git:
	git add .; \
	git commit -m "Makefile commit message"; \
	git push