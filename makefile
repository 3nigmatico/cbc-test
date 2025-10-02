all:
	gcc -c -g -pedantic -fwrapv -Wall -O0 -S mask.c -o mask.s -fsanitize=undefined
	gcc -g -fwrapv -O0 mask.s -o mask -largon2 -fsanitize=undefined
salttest:
	gcc -c -O0 salttest.c -o salttest.o
	gcc -O0 salttest.o -o test -largon2
clean:
	rm -rf *.o
	rm -rf *.s
	rm -f -- mask
	rm -f -- test
