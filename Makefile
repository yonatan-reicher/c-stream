name = stream
target = libstream.a

objects = char-stream.o cmd-stream.o file-stream.o stream.o \
		  str-stream.o then-stream.o trim-stream.o
headers = $(objects:.o=.h) common.h

CFLAGS = -Wall -Wextra

prefix = /usr/local

$(target) : $(target)($(objects)) # Archive member notation + implicit rules!

.PHONY: install
install : $(target)
	install -m 644 $(target) $(prefix)/lib/
	install -d $(prefix)/include/$(name)
	install -m 644 $(headers) $(prefix)/include/$(name)

.PHONY: uninstall
uninstall :
	-rm $(prefix)/lib/$(target)
	-rm -r $(prefix)/include/$(name)

.PHONY: test
test : CFLAGS += -fsanitize=undefined -fsanitize=address -fsanitize=undefined -g
test : main
	./main

main : main.c $(target)

.PHONY : clean
clean:
	-rm *.o
	-rm *.a
