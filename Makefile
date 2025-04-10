all: client.c server.c utils.c network.c database.c
	gcc -c utils.c -Llib -lsqlite3 -lcjson
	gcc -c network.c -Llib -lsqlite3 -lcjson
	gcc -c database.c -Llib -lsqlite3 -lcjson
	gcc client.c database.o network.o -std=gnu99 -Llib -lsqlite3 -lcjson -pthread -ldl -o client.exe
	gcc server.c network.o database.o utils.o -Llib -lsqlite3 -lcjson -pthread -ldl -o server.exe

clean:
	rm main.exe server.exe network.o database.o utils.o
