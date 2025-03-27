all: main.c
	gcc main.c -Llib -lsqlite3 -lcjson -o main.exe

clean:
	rm main.exe