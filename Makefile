target = libstream.a
headers = $(wildcard *.h)

objects = char-stream.o cmd-stream.o file-stream.o stream.o \
		  str-stream.o then-stream.o trim-stream.o

CFLAGS = -Wall -Wextra

prefix = /usr/local

$(target) : $(target)($(objects)) # Archive member notation + implicit rules!

.PHONY: install
install: $(target)
	install -m 644 $(target) $(prefix)/lib/
	install -d 644 $(prefix)/include/stream/
	install -m 644 $(headers) $(prefix)/include/stream/

.PHONY : clean
clean:
	-rm *.o
	-rm *.a
