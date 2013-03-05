CC = gcc
CFLAGS = -Wall -gdwarf-2 -g3 
LIBS = -lccn -lccnsync -lcrypto 

PROGRAMS = nlsrc nlsr
INSTALL_PATH=/usr/local/bin/

NLSR_SRCS=nlsr.c nlsr_ndn.c nlsr_npl.c  nlsr_adl.c nlsr_lsdb.c nlsr_route.c nlsr_npt.c nlsr_fib.c nlsr_sync.c  nlsr_face.c nlsr_km.c nlsr_km_util.c utility.c
NLSRC_SRCS=nlsrc.c

all: $(PROGRAMS)

nlsr: $(NLSR_SRCS)
	$(CC) $(CFLAGS) $(NLSR_SRCS) $(LIBS) -lm -o nlsr

nlsrc: $(NLSRC_SRCS)
	$(CC) $(CFLAGS) $(NLSRC_SRCS) $(LIBS) -lm -o nlsrc

install: $(PROGRAMS)
	cp $(PROGRAMS) $(INSTALL_PATH)
	cd $(INSTALL_PATH); chown root:0 $(PROGRAMS); chmod 755 $(PROGRAMS)

uninstall:
	cd $(INSTALL_PATH); rm -f $(PROGRAMS)

clean:
	rm -f *.o
	rm -f $(PROGRAMS)
	rm -rf *.dSYM

.c.o:
	$(CC) $(CFLAGS) -c $<

.PHONY: all clean
