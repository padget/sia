from dataclasses import dataclass

import ply.lex as lex
import ply.yacc as yacc

from sia.core.core import production, token

# SIA LEXER

lexems = {
    'number': r'\d+',
    'equal': r'=',
    'colon': ':',
    'comma': r',',
    'lbracket': r'\(',
    'rbracket': r'\)',
    'lbrace': r'\{',
    'rbrace': r'\}',
    'point': r'\.',
    'arrow': r'\->'
}

reserved = {
    'type': 'type',
    'casefn': 'casefn',
    'fn': 'fn',
    'alias': 'alias',
    'native': 'native'
}

tokens = ['name', 'string'] + [l for l in lexems] + \
         [r for r in reserved.values()]

t_number = lexems['number']
t_equal = lexems['equal']
t_colon = lexems['colon']
t_comma = lexems['comma']
t_lbracket = lexems['lbracket']
t_rbracket = lexems['rbracket']
t_lbrace = lexems['lbrace']
t_rbrace = lexems['rbrace']
t_point = lexems['point']
t_arrow = lexems['arrow']


@token(r'[a-zA-Z_]([0-9a-zA-Z_])*')
def t_name(t):
    t.type = reserved.get(t.value, 'name')
    return t


@token(r'"[^"]*"')
def t_string(t):
    return t


@token(r'\n+')
def t_newline(t):
    t.lexer.lineno += len(t.value)


@token(r'\s+')
def t_ignored(t):
    pass


def t_error(t):
    print(f"Illegal character '{t.value[0]}'")
    t.lexer.skip(1)


lexer = lex.lex()


@dataclass
class Sia:
    declarations: list


class Expression:
    pass


class Declaration(Expression):
    pass


@dataclass
class Number(Expression):
    value: int


@dataclass
class String(Expression):
    value: str


@dataclass
class Name(Expression):
    value: str


@dataclass
class FnCall(Expression):
    fname: Name
    params: list


@dataclass
class MemberAccess(Expression):
    expr: Expression
    member: Name


@dataclass
class SpreadFnCall(Expression):
    expr: Expression
    fname: Name


@dataclass
class FnDeclaration(Declaration):
    fname: Name
    args: list
    rtype: Name
    aliases: list
    retexpr: Expression


@dataclass
class FnDeclarationNative(Declaration):
    fname: Name
    args: list
    rtype: Name


@dataclass
class CaseFnDeclaration(Declaration):
    pass


@dataclass
class TypeDeclaration(Declaration):
    tname: Name
    args: list


@dataclass
class Arg:
    aname: Name
    tname: Name


@dataclass
class Param:
    expr: Expression


@dataclass
class Alias:
    alname: Name
    expr: Expression


@production('alias_statement : alias name equal expression')
def p_alias_statement(yprod):
    yprod[0] = Alias(alname=Name(value=yprod[2]), expr=yprod[4])


@production('args : arg args_tail')
def p_args(yprod):
    yprod[0] = [yprod[1]] + yprod[2]


@production('args : empty')
def p_args_empty(yprod):
    yprod[0] = []


@production('args_tail : comma arg args_tail')
def p_args_tail(yprod):
    yprod[0] = [yprod[2]] + yprod[3]


@production('args_tail : empty')
def p_args_tail_empty(yprod):
    yprod[0] = []


@production('arg : name colon name')
def p_arg(yprod):
    yprod[0] = Arg(aname=Name(value=yprod[1]), tname=Name(value=yprod[3]))


@production(
    'casefn_declaration : casefn name lbracket simple_params rbracket colon expression  ')
def p_casefn_declaration(yprod):
    pass


@production('expression : number')
def p_expression_number(yprod):
    yprod[0] = Number(value=yprod[1])


@production('expression : string')
def p_expression_string(yprod):
    yprod[0] = String(value=yprod[1])


@production('expression : name')
def p_expression_name(yprod):
    yprod[0] = Name(value=yprod[1])


@production('expression : fn_call')
def p_expression_fn_call(yprod):
    yprod[0] = yprod[1]


@production('expression : fn_declaration')
def p_expression_fn_declaration(yprod):
    yprod[0] = yprod[1]


@production('expression : member_access')
def p_expression_member_access(yprod):
    yprod[0] = yprod[1]


@production('member_access : expression point name')
def p_member_access(yprod):
    yprod[0] = MemberAccess(expr=yprod[1], member=Name(value=yprod[3]))


@production('expression : spread_fn_call')
def p_expression_spread_fn_call(yprod):
    yprod[0] = yprod[1]


@production('spread_fn_call : expression colon colon name lbracket rbracket')
def p_member_spread_fn_call(yprod):
    yprod[0] = SpreadFnCall(expr=yprod[1], fname=Name(value=yprod[4]))


@production('fn_call : lbracket params rbracket point name lbracket rbracket')
def p_fn_call(yprod):
    yprod[0] = FnCall(fname=Name(value=yprod[5]), params=Name(value=yprod[2]))


@production('params : expression params_tail')
def p_params(yprod):
    yprod[0] = [yprod[1]] + yprod[2]


@production('params : empty')
def p_params_empty(yprod):
    yprod[0] = []


@production('params_tail : comma expression params_tail')
def p_params_tail(yprod):
    yprod[0] = [yprod[2]] + yprod[3]


@production('params_tail : empty')
def p_params_tail_empty(yprod):
    yprod[0] = []


@production('simple_expression : number')
def p_simple_expression_number(yprod):
    yprod[0] = Number(value=yprod[1])


@production('simple_expression : string')
def p_simple_expression_string(yprod):
    yprod[0] = String(value=yprod[1])


@production('simple_expression : name')
def p_simple_expression_name(yprod):
    print('coucou')
    yprod[0] = Name(value=yprod[1])


@production('simple_params : simple_expression simple_params_tail')
def p_simple_params(yprod):
    yprod[0] = [yprod[1]] + yprod[2]


@production('simple_params : empty')
def p_simple_params_empty(yprod):
    yprod[0] = []


@production('simple_params_tail : comma simple_expression simple_params_tail')
def p_simple_params_tail(yprod):
    yprod[0] = [yprod[2]] + yprod[3]


@production('simple_params_tail : empty')
def p_simple_params_tail_empty(yprod):
    yprod[0] = []


@production(
    '''fn_declaration : fn_signature lbrace fn_lines expression rbrace''')
def p_fn_declaration(yprod):
    yprod[0] = FnDeclaration(
        fname=yprod[1][0],
        args=yprod[1][1],
        rtype=yprod[1][2],
        aliases=yprod[3],
        retexpr=yprod[4])


@production('fn_declaration : fn_signature native')
def p_fn_declaration_native(yprod):
    yprod[0] = FnDeclarationNative(
        fname=yprod[1][0],
        args=yprod[1][1],
        rtype=yprod[1][2])


@production('''fn_signature : fn name lbracket args rbracket arrow name''')
def p_fn_signature(yprod):
    yprod[0] = (Name(value=yprod[2]), yprod[4], Name(value=yprod[7]))


@production('fn_lines : alias_statement fn_lines')
def p_fn_lines(yprod):
    yprod[0] = [yprod[1]] + yprod[2]


@production('fn_lines : empty')
def p_fn_lines_empty(yprod):
    yprod[0] = []


@production('type_declaration : type name lbracket args rbracket')
def p_type_declaration(yprod):
    yprod[0] = TypeDeclaration(tname=Name(value=yprod[2]), args=yprod[4])


# SIA PARSER
start = 'sia'


@production('empty :')
def p_empty(yprod):
    pass


@production('sia : declarations')
def p_sia(yprod):
    yprod[0] = Sia(yprod[1])


@production('declarations : declaration declarations')
def p_declarations(yprod):
    yprod[0] = [yprod[1]] + yprod[2]


@production('declarations : empty')
def p_declarations_empty(yprod):
    yprod[0] = []


@production('''
    declaration : type_declaration
                | fn_declaration
                | casefn_declaration''')
def p_declaration(yprod):
    yprod[0] = yprod[1]


parser = yacc.yacc()
