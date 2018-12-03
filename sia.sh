export PATH=.:$PATH
sia_configure_compiler --config sia.properties
sia_prepare_database
tkn_inject_file
tkn_tokenize
stx_detect_types
stx_detect_functions
