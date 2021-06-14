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

testClient:
	bin/aurras status
	bin/aurras transform samples/sample-1-so.m4a music1.mp3 alto lento
	bin/aurras transform samples/sample-2-miei.m4a music2.mp3 baixo
	bin/aurras transform samples/sample-3-lcc.m4a music3.mp3 rapido eco

testServer:
	bin/aurrasd etc/aurrasd.conf bin/aurrasd-filters
	
	
	
