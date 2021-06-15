all: server client

server: bin/aurrasd

client: bin/aurras

bin/aurrasd: obj/aurrasd.o
	gcc -g obj/aurrasd.o -o bin/aurrasd

obj/aurrasd.o: src/aurrasd.c
	gcc -Wall -g -c src/aurrasd.c -o obj/aurrasd.o

bin/aurras: obj/aurras.o
	gcc -g obj/aurras.o -o bin/aurras

obj/aurras.o: src/aurras.c
	gcc -Wall -g -c src/aurras.c -o obj/aurras.o

clean:
	rm obj/* bin/aurras bin/aurrasd tmp/*

status:
	bin/aurras status

transform1:
	bin/aurras transform samples/sample-1-so.m4a music1.mp3 lento alto

transform2:
	bin/aurras transform samples/sample-2-miei.m4a music2.mp3 baixo rapido eco 

transform3:
	bin/aurras transform samples/sample-3-lcc.m4a music3.mp3 rapido rapido alto  

transform4:
	bin/aurras transform samples/Ievan-Polkka-Loituma.m4a music4.mp3 rapido rapido eco

openServer:
	bin/aurrasd etc/aurrasd.conf bin/aurrasd-filters
	
	
