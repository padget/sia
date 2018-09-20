g++ -std=c++17 -c src/tokenizer/tokenize.cpp -o tokenize.o -Ilib/include
g++ -std=c++17 -o tokenize.exe tokenize.o lib/obj/sqlite3.o
rm -f *.o