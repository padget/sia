from core import production


@production('type_declaration : type name lbracket args rbracket')
def p_type_declaration(yprod):
    from parser_ast import TypeDeclaration, Name
    yprod[0] = TypeDeclaration(tname=Name(value=yprod[2]), args=yprod[4])
