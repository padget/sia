from core import production


@production('''fn_declaration : fn_signature lbrace fn_lines expression rbrace''')
def p_fn_declaration(yprod):
    from parser_ast import fn_declaration, alias, expression
    aliases = yprod[3]

    yprod[0] = fn_declaration(
        fname=yprod[1][0],
        args=yprod[1][1],
        rtype=yprod[1][2],
        aliases=yprod[3],
        retexpr=yprod[4])


@production('''fn_signature : fn name lbracket args rbracket arrow name''')
def p_fn_signature(yprod):
    yprod[0] = (yprod[2], yprod[4], yprod[7])


@production('fn_lines : alias_statement fn_lines')
def p_fn_lines(yprod):
    yprod[0] = [yprod[1]] + yprod[2]


@production('fn_lines : empty')
def p_fn_lines_empty(yprod):
    yprod[0] = []
