all: xkbdrelay

xkbdrelay: xkbdrelay.o
	gcc -o xkbdrelay xkbdrelay.o -L/usr/X11R6/lib -lX11 -lXtst -lc

xkbdrelay.o: xkbdrelay.c
	gcc -c xkbdrelay.c -o xkbdrelay.o

clean:
	rm -f *.o xkbdrelay
