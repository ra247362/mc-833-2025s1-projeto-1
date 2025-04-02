all: main.c
	gcc main.c -Llib -lsqlite3 -lcjson -o main.exe
	gcc server.c -Llib -lsqlite3 -lcjson -o server.exe

clean:
	rm main.exe server.exe