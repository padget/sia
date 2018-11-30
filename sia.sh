export PATH=.:$PATH
configure_compiler --config sia.properties
prepare_database
inject_file
tokenize
detect_types
