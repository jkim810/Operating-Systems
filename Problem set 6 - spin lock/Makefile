ftest: 
	gcc ftest.c fifo.c cv.c spinlock.c tas64.S -o ftest.exe

cvtest:
	gcc cvtest.c cv.c spinlock.c tas64.S -o cvtest.exe

spintest:
	gcc spintest.c spinlock.c tas64.S -o spintest.exe

clean:
	rm -f *.exe *.o

backup:
	test -d backups || mkdir backups
	cp *.c backups
	cp Makefile backups
