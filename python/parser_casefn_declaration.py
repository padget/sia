from core import production

@production('casefn_declaration : casefn name lbracket params rbracket lbrace aliases expression rbrace')
def p_casefn_declaration(yprod):
    pass