all: main.c
	gcc main.c -Llib -lsqlite3 -lcjson -o main.exe
	gcc server.c -Llib -lsqlite3 -lcjson -pthread -o server.exe
	gcc client.c -Llib -lsqlite3 -lcjson -o client.exe


clean:
	rm main.exe server.exe

#all: main.exe server.exe client.exe
#
#main.exe: main.c database.c db_setup.c
#	gcc main.c database.c db_setup.c -Llib -lsqlite3 -lcjson -o main.exe
#
#server.exe: server.c database.c db_setup.c
#	gcc server.c database.c db_setup.c -Llib -lsqlite3 -lcjson -pthread -o server.exe
#
#client.exe: client.c
#	gcc client.c -Llib -lsqlite3 -lcjson -o client.exe
#
#clean:
#	rm -f main.exe server.exe client.exe
