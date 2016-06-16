all:
	#g++ -fopenmp -o Beowulf main.c attacks.c board.c checks.c comp.c computil.c eval.c moves.c parser.c pers.c probe.c rand64.c tactics.c utils.c
	gcc -o server server.c -lpthread
	gcc -o client client.c

clean:
	rm server client
