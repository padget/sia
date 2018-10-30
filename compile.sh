g++ -std=c++2a -c src/tokenizer/tokenize.cpp -o tokenize.o -Ilib/include -Iinclude -fconcepts
g++ -std=c++2a -o tokenize.exe tokenize.o lib/obj/sqlite3.o
rm -f *.o