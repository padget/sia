from core import production


@production('''fn_declaration : fn_signature colon fn_lines''')
def p_fn_declaration(yprod):
    from parser_ast import fn_declaration, alias, expression
    lines = yprod[3]
    aliases = [l for l in lines if isinstance(l, alias)]
    retexpr = [l for l in lines if isinstance(l, expression)]

    yprod[0] = fn_declaration(
        fname=yprod[1][0],
        args=yprod[1][1],
        rtype=yprod[1][2],
        aliases=aliases,
        retexpr=retexpr[-1] if retexpr else None)


@production('''fn_signature : fn name lbracket args rbracket arrow name''')
def p_fn_signature(yprod):
    yprod[0] = (yprod[2], yprod[4], yprod[7])


@production('fn_lines : fn_line fn_lines')
def p_fn_lines(yprod):
    yprod[0] = [yprod[1]] + yprod[2]


@production('fn_lines : empty')
def p_fn_lines_empty(yprod):
    yprod[0] = []


@production('fn_line : indent fn_line_tail')
def p_fn_line(yprod):
    yprod[0] = yprod[2]


@production('''fn_line_tail : alias_statement
                            | expression''')
def p_fn_line_tail(yprod):
    yprod[0] = yprod[1]
