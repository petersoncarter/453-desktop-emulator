all:
	gcc -I /opt/X11/include -L /opt/X11/lib/ -lX11 *.c -o table

clean:
	rm table