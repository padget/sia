gcc -c src/tokenizer/tokenize.c -o tokenize.o -Ilib/include
gcc -o tokenize.exe tokenize.o lib/obj/sqlite3.o
rm -f *.o