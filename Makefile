all:
	gcc -o server server.c -lpthread
	gcc -o client client.c -lncurses

clean:
	rm server client
