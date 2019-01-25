from core import production


@production('fn_declaration : fn name lbracket args rbracket colon name lbrace aliases expression rbrace')
def p_fn_declaration(yprod):
    pass


@production('''
    aliases : alias_statement aliases
            | empty''')
def p_aliases(yprod):
    pass
