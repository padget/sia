echo ' -- sqlite3.o'
mkdir -p lib/obj
gcc  -c lib/include/sqlite3.c -o lib/obj/sqlite3.o -Ilib/Iinclude

echo ' -- sia_configure_compiler.exe'

g++ -std=c++2a -c src/sia_configure_compiler.cpp -o sia_configure_compiler.o -Ilib/include -Iinclude -fconcepts
g++ -std=c++2a -o sia_configure_compiler.exe sia_configure_compiler.o lib/obj/sqlite3.o

echo ' -- sia_prepare_database.exe'

g++ -std=c++2a -c src/sia_prepare_database.cpp -o sia_prepare_database.o -Ilib/include -Iinclude -fconcepts
g++ -std=c++2a -o sia_prepare_database.exe sia_prepare_database.o lib/obj/sqlite3.o

echo ' -- tkn_inject_file.exe'

g++ -std=c++2a -c src/tkn_inject_file.cpp -o tkn_inject_file.o -Ilib/include -Iinclude -fconcepts
g++ -std=c++2a -o tkn_inject_file.exe tkn_inject_file.o lib/obj/sqlite3.o

echo ' -- tkn_tokenize.exe'

g++ -std=c++2a -c src/tkn_tokenize.cpp -o tkn_tokenize.o -Ilib/include -Iinclude -fconcepts
g++ -std=c++2a -o tkn_tokenize.exe tkn_tokenize.o lib/obj/sqlite3.o

echo ' -- stx_detect_types.exe'

g++ -std=c++2a -c src/stx_detect_types.cpp -o stx_detect_types.o -Ilib/include -Iinclude -fconcepts
g++ -std=c++2a -o stx_detect_types.exe stx_detect_types.o lib/obj/sqlite3.o

echo ' -- stx_detect_functions.exe'

g++ -std=c++2a -c src/stx_detect_functions.cpp -o stx_detect_functions.o -Ilib/include -Iinclude -fconcepts
g++ -std=c++2a -o stx_detect_functions.exe stx_detect_functions.o lib/obj/sqlite3.o

echo ' -- clean *.o'

rm -f *.o

echo ""
