echo 'prout'
export PATH=.:$PATH
./sia_configure_compiler.exe --config sia.properties
./sia_prepare_database.exe
./tkn_inject_file.exe
./tkn_tokenize.exe
./stx_detect_types.exe
./stx_detect_functions.exe


