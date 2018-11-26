echo ' -- sqlite3.o'

# gcc  -c lib/include/sqlite3.c -o lib/obj/sqlite3.o -Ilib/Iinclude

echo ' -- tokenize.exe'

g++ -std=c++2a -c src/tokenize.cpp -o tokenize.o -Ilib/include -Iinclude -fconcepts
g++ -std=c++2a -o tokenize.exe tokenize.o lib/obj/sqlite3.o

echo ' -- detect_types.exe'

g++ -std=c++2a -c src/detect_types.cpp -o detect_types.o -Ilib/include -Iinclude -fconcepts
g++ -std=c++2a -o detect_types.exe detect_types.o lib/obj/sqlite3.o

echo ' -- clean *.o'

rm -f *.o

echo ""