from core import production


@production('alias_statement : alias name equal expression')
def p_alias_statement(yprod):
    from parser_ast import alias
    yprod[0] = alias(alname=yprod[2], expr=yprod[4])
