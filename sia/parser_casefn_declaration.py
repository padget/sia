from core import production


@production('casefn_declaration : casefn name lbracket simple_params rbracket colon expression  ')
def p_casefn_declaration(yprod):
    pass
