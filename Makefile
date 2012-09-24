CC = cc
CFLAGS = -g -Wall -Wpointer-arith -Wreturn-type -Wstrict-prototypes
LIBS = -lcrypto -lccn

PROGRAMS = nlsr

all: $(PROGRAMS)

nlsr: nlsr.c nlsr_ndn.c nlsr_npl.c  nlsr_adl.c nlsr_lsdb.c nlsr_route.c nlsr_npt.c nlsr_fib.c utility.c
	$(CC) $(CFLAGS) nlsr.c nlsr_ndn.c nlsr_npl.c nlsr_adl.c nlsr_lsdb.c nlsr_route.c nlsr_npt.c nlsr_fib.c utility.c -o nlsr $(LIBS)

clean:
	rm -f *.o
	rm -f $(PROGRAMS)
	rm -rf $(PROGRAMS).dSYM

.c.o:
	$(CC) $(CFLAGS) -c $<

.PHONY: all clean
