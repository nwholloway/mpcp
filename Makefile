all: mpcp

clean:
	rm -f *.o

clobber: clean
	rm -f mpcp

mpcp: mpcp.o
	gcc -o $@ $<

