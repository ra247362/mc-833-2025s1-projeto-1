all: main.c
	gcc main.c -Llib -lcsv -o main


clean:
	rm main