from core import production


@production('alias_statement : alias name equal expression')
def p_alias_statement(yprod):
    from parser_ast import Alias, Name
    yprod[0] = Alias(alname=Name(value=yprod[2]), expr=yprod[4])
