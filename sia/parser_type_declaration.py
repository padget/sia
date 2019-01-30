from core import production


@production('type_declaration : type name lbracket args rbracket')
def p_type_declaration(yprod):
    from parser_ast import type_declaration
    yprod[0] = type_declaration(tname=yprod[2], args=yprod[4])
