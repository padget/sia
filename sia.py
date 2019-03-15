from ply.lex import Token

from sia.core.core import token, production
from ply import lex, yacc

from sia_ast import TypeDeclaration, FnDeclaration, LetDeclaration, FnCall

################
# TOKENS LEXER #
################

tokens = []


@token(r'type', tokens)
def t_type(t: Token):
    return t


@token(r'let', tokens)
def t_let(t: Token):
    return t


@token(r'fn', tokens)
def t_fn(t: Token):
    return t


@token(r'\d+', tokens)
def t_number(t: Token):
    return t


@token(r'"[^"]*"', tokens)
def t_string(t: Token):
    return t


@token(r'[a-zA-Z_]([0-9a-zA-Z_])*', tokens)
def t_ident(t: Token):
    return t


@token(r'\(', tokens)
def t_lbracket(t: Token):
    return t


@token(r'\)', tokens)
def t_rbracket(t: Token):
    return t


@token(r'\n+')
def t_newline(t: Token):
    t.lexer.lineno += len(t.value)


@token(r'\s+')
def t_ignored(t: Token):
    pass


def t_error(t: Token):
    print(f'Illegal character \'{t.value[0]}\'')
    t.lexer.skip(1)


lexer = lex.lex()


################
# RULES PARSER #
################


@production('sia : declarations')
def p_sia(p):
    p[0] = p[1]


@production('''declarations : declaration declarations 
                            | empty''')
def p_declarations(p):
    if len(p) == 3:
        p[0] = [p[1]] + p[2]
    else:
        p[0] = []


@production('''declaration : type_declaration
                           | fn_declaration''')
def p_declaration(p):
    p[0] = p[1]


@production('type_declaration : type ident lbracket attributes rbracket')
def p_type_declaration(p):
    p[0] = TypeDeclaration(
        name=p[2],
        attributes=p[4])


@production('''attributes : attribute attributes
                          | empty''')
def p_attributes(p):
    if len(p) == 3:
        p[0] = [p[1]] + p[2]
    else:
        p[0] = []


@production('attribute : ident')
def p_attribute(p):
    p[0] = p[1]


@production('empty :')
def p_empty(p):
    pass


@production(
    'fn_declaration : fn ident lbracket arguments rbracket '
    'lbracket let_declarations expression rbracket')
def p_fn_declaration(p):
    p[0] = FnDeclaration(
        name=p[2],
        arguments=p[4],
        let_declarations=p[7],
        expression=p[8])


@production('''arguments : argument arguments
                         | empty''')
def p_arguments(p):
    if len(p) == 3:
        p[0] = [p[1]] + p[2]
    else:
        p[0] = []


@production('argument : ident')
def p_argument(p):
    p[0] = p[1]


@production('''let_declarations : let_declaration let_declarations
                                | empty''')
def p_let_declarations(p):
    if len(p) == 3:
        p[0] = [p[1]] + p[2]
    else:
        p[0] = []


@production('let_declaration : let ident expression')
def p_let_declaration(p):
    p[0] = LetDeclaration(
        name=p[2],
        expression=p[3])


@production('''expression : number
                          | string
                          | ident
                          | fn_call''')
def p_expression(p):
    p[0] = p[1]  # TODO différencier les expressions avec les objets ast correspondant


@production('fn_call : ident lbracket expressions rbracket')
def p_fn_call(p):
    p[0] = FnCall(
        name=p[1],
        expressions=p[3])


@production('''expressions : expression expressions
                           | empty''')
def p_expressions(p):
    if len(p) == 3:
        p[0] = [p[1]] + p[2]
    else:
        p[0] = []


parser = yacc.yacc(start='sia')

if __name__ == '__main__':
    inp = '''
    fn add2 (a b) (
        let c 12
        add(a b c)
    )
    '''
    print(parser.parse(lexer=lexer, input=inp))
    pass
