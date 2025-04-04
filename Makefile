all: main.c
	gcc main.c -Llib -lsqlite3 -lcjson -o main.exe
	gcc server.c -Llib -lsqlite3 -lcjson -pthread -o server.exe
	gcc client.c -Llib -lsqlite3 -lcjson -o client.exe


clean:
	rm main.exe server.exe