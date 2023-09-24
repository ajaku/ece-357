hunt.exe: hunt.c
	gcc -Wall -Wextra -o hunt.exe hunt.c

debug:
	gcc -Wall -Wextra -g -o huntDebug.exe hunt.c

clean:
	rm -f *.exe *.o *.stackdump *~

backup:
	test -d backups || mkdir backups
	cp *.c backups
	cp *.h backups
	cp Makefile backups