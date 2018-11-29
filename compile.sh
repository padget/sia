echo ' -- sqlite3.o'

gcc  -c lib/include/sqlite3.c -o lib/obj/sqlite3.o -Ilib/Iinclude

echo ' -- configure_compiler.exe'

g++ -std=c++2a -c src/configure_compiler.cpp -o configure_compiler.o -Ilib/include -Iinclude -fconcepts
g++ -std=c++2a -o configure_compiler.exe configure_compiler.o lib/obj/sqlite3.o

echo ' -- prepare_database.exe'

g++ -std=c++2a -c src/prepare_database.cpp -o prepare_database.o -Ilib/include -Iinclude -fconcepts
g++ -std=c++2a -o prepare_database.exe prepare_database.o lib/obj/sqlite3.o

echo ' -- inject_file.exe'

g++ -std=c++2a -c src/inject_file.cpp -o inject_file.o -Ilib/include -Iinclude -fconcepts
g++ -std=c++2a -o inject_file.exe inject_file.o lib/obj/sqlite3.o

echo ' -- tokenize.exe'

g++ -std=c++2a -c src/tokenize.cpp -o tokenize.o -Ilib/include -Iinclude -fconcepts
g++ -std=c++2a -o tokenize.exe tokenize.o lib/obj/sqlite3.o

echo ' -- detect_types.exe'

g++ -std=c++2a -c src/detect_types.cpp -o detect_types.o -Ilib/include -Iinclude -fconcepts
g++ -std=c++2a -o detect_types.exe detect_types.o lib/obj/sqlite3.o

echo ' -- clean *.o'

rm -f *.o

echo ""