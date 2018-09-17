gcc -c src/sia/wstring.c -o sia_wstring.o -Ilib/include
gcc -c src/sia/string.c -o sia_string.o -Ilib/include
gcc -c test/sia/string.test.c -o test_sia_string.o -Ilib/include

gcc -o test_sia_string.exe test_sia_string.o lib/obj/sqlite3.o sia_wstring.o sia_string.o
rm -f *.o