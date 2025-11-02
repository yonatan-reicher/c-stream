main: *.c *.h
	gcc -Wall -Wextra -fsanitize=undefined -o main *.c

run: main
	./main


.PHONY: run
