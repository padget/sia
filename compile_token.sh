gcc -c src/sia/string.c -o sia_string.o -Ilib/include
gcc -c src/tokenizer/tokenize.c -o tokenize.o -Ilib/include
gcc -o tokenize.exe tokenize.o lib/obj/sqlite3.o sia_string.o
rm -f *.o