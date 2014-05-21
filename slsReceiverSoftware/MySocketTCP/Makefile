
TOBECLEANED	 = MySocketTCP.o

PROGRAMS	= rec send

all:		$(PROGRAMS)

clean:
		@rm -f $(TOBECLEANED) $(PROGRAMS)

rec: 	MySocketTCP.o rec.cxx
		g++ -o $@ $^
		@echo "$@ done"

send:	MySocketTCP.o send.cxx
		g++ -o $@ $^
		@echo "$@ done"

MySocketTCP.o:	 MySocketTCP.cxx MySocketTCP.h
		g++ -c $<
		@echo "$@ done"


