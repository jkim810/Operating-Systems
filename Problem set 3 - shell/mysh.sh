#!/bin/bash
cat shell.c >shellcopy
diff shellcopy shell.c
rm shellcopy
#this command will return error as there is no ll command found by this shell as ll is not aliased as ls -la
ll 2>errorlog.txt
#cd to non existent file
cd filethatdoesnotexist 2>>errorlog.txt
rm 2>>errorlog.txt
#prints out list of file in the directory
ls -la 2>>errorlog.txt
./test.sh
echo $?
#test.sh returns with value 123
./a.out <test.sh
echo $?
#./a.out returns 0
exit
