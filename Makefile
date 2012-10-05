CC = cc
CFLAGS = -g -Wall -Wpointer-arith -Wreturn-type -Wstrict-prototypes
LIBS = -lccn -lcrypto

PROGRAMS = nlsr
INSTALL_PATH=/usr/local/bin/

NLSR_SRCS=nlsr.c nlsr_ndn.c nlsr_npl.c  nlsr_adl.c nlsr_lsdb.c nlsr_route.c nlsr_npt.c nlsr_fib.c utility.c


all: $(PROGRAMS)

nlsr: $(NLSR_SRCS)
	$(CC) $(CFLAGS) $(NLSR_SRCS) -o nlsr $(LIBS) -lm

install: $(PROGRAMS)
	cp $(PROGRAMS) $(INSTALL_PATH)
	cd $(INSTALL_PATH); chown root:0 $(PROGRAMS); chmod 755 $(PROGRAMS)

uninstall:
	cd $(INSTALL_PATH); rm -f $(PROGRAMS)

clean:
	rm -f *.o
	rm -f $(PROGRAMS)
	rm -rf $(PROGRAMS).dSYM

.c.o:
	$(CC) $(CFLAGS) -c $<

.PHONY: all clean
