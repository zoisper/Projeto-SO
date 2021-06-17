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

test:
	bin/aurras status
	bin/aurras transform samples/sample-1-so.m4a music1.mp3 lento
	bin/aurras transform samples/sample-2-miei.m4a music2.mp3 alto lento


openServer:
	bin/aurrasd etc/aurrasd.conf bin/aurrasd-filters

status:
	bin/aurras status

transform1:
	bin/aurras transform samples/sample-1-so.m4a music1.mp3 lento 

transform2:
	bin/aurras transform samples/sample-2-miei.m4a music2.mp3 alto lento  

transform3:
	bin/aurras transform samples/sample-3-lcc.m4a music3.mp3 rapido eco baixo  

transform4:
	bin/aurras transform samples/Ievan-Polkka-Loituma.m4a music4.mp3 rapido rapido eco

transform5:
	bin/aurras transform samples/Ievan-Polkka-Loituma.m4a music5.mp3 lento eco eco

transform6:
	bin/aurras transform samples/sample-1-so.m4a music6.mp3 baixo

transform7:
	bin/aurras transform samples/sample-3-lcc.m4a music7.mp3 rapido eco baixo alto rapido eco baixo alto    

transform8:
	bin/aurras transform samples/sample-2-miei.m4a music8.mp3 eco lento


	
	
