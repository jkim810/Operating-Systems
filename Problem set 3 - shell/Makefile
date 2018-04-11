mshell.exe: minishell.o
	gcc -o minishell.exe minishell.o

mshell.o: minishell.c
	gcc -c minishell.c

debug:
	gcc -g -o mshellDebug.exe minishell.c

clean:
	rm -f *.exe *.o

backup:
	test -d backups || mkdir backups
	cp *.c backups
	cp Makefile backups
