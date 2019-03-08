from dataclasses import dataclass

from ply import yacc
from ply.lex import lex, Lexer

from sia.core.core import token, production

lexems = {
    'number': r'\d+',
    'lbracket': r'\(',
    'rbracket': r'\)',
    'string': r'"[^"]*"',
    'ident': r'[a-zA-Z_]([0-9a-zA-Z_])*'
}

t_number = lexems['number']
t_string = lexems['string']
t_ident = lexems['ident']
t_lbracket = lexems['lbracket']
t_rbracket = lexems['rbracket']

tokens = [l for l in lexems]


@token(r'\n+')
def t_newline(t):
    t.lexer.lineno += len(t.value)


@token(r'\s+')
def t_ignored(t):
    pass


def t_error(t):
    print(f'Illegal character \'{t.value[0]}\'')
    t.lexer.skip(1)


lexer: Lexer = lex()


@dataclass(frozen=True)
class Expresssion:
    pass


@dataclass(frozen=True)
class List(Expresssion):
    expressions: list


@dataclass(frozen=True)
class String(Expresssion):
    value: str


@dataclass(frozen=True)
class Number(Expresssion):
    value: int


@dataclass(frozen=True)
class Ident(Expresssion):
    value: str


@production('list : lbracket expressions rbracket')
def p_list(yprod):
    yprod[0] = List(expressions=yprod[2])


@production(
    '''expressions : expressions expression
                   | expression''')
def p_expressions(yprod):
    if len(yprod) == 2:
        yprod[0] = [yprod[1]]
    else:
        yprod[0] = yprod[1] + [yprod[2]]


@production('expression : list')
def p_expression_list(yprod):
    yprod[0] = yprod[1]


@production('expression : atom')
def p_expression_atom(yprod):
    yprod[0] = yprod[1]


@production('atom : number')
def p_atom_number(yprod):
    yprod[0] = Number(value=int(yprod[1]))


@production('atom : ident')
def p_atom_ident(yprod):
    yprod[0] = Ident(value=yprod[1])


@production('atom : string')
def p_atom_string(yprod):
    yprod[0] = String(value=yprod[1][1:-1])


parser = yacc.yacc(start='list')


class SialContext:
    def __init__(self, funcs={}, lets={}):
        self.funcs: dict = funcs
        self.lets: dict = lets


@dataclass(frozen=True)
class Let(Expresssion):
    name: str
    value: Expresssion


@dataclass(frozen=True)
class Undefined(Expresssion):
    value: str


def reduce_expression(expr: Expresssion, ctxt: SialContext):
    if isinstance(expr, List):
        return reduce_list(expr, ctxt)
    elif isinstance(expr, Ident):
        return reduce_ident(expr, ctxt)
    else:
        return reduce_native(expr)


def reduce_native(obj):
    if isinstance(obj, Number):
        return reduce_number(obj)
    elif isinstance(obj, String):
        return reduce_string(obj)


def reduce_list(lst: List, ctxt: SialContext):
    if isinstance(lst.expressions[0], Ident):
        tails = lst.expressions[1:]
        results = [reduce_expression(tail, ctxt) for tail in tails]
        v = reduce_ident(lst.expressions[0], ctxt)

        if callable(v):
            r = v(*results)
            return r
        elif isinstance(v, Let):
            return v
    else:
        return List([reduce_expression(expr, ctxt) for expr in lst.expressions])


def reduce_ident(ident: Ident, ctxt: SialContext):
    key = ident.value

    if key in ctxt.funcs:
        return ctxt.funcs[key]
    elif key in ctxt.lets:
        return ctxt.lets[key]
    else:
        return Undefined(value=key)


def reduce_number(n: Number):
    return n


def reduce_string(s: String):
    return s


if __name__ == '__main__':
    native_funcs = {
        'add': lambda x, y: Number(x.value + y.value),
        'mult': lambda x, y: Number(x.value * y.value),
        'div': lambda x, y: Number(x.value / y.value),
        'minus': lambda x, y: Number(x.value - y.value),
    }

    ctxt = SialContext(funcs=native_funcs)


    def define_let(ident, expr):
        if isinstance(ident, Undefined):
            ctxt.lets[ident.value] = expr
            return Let(name=Ident(value=ident.value), value=expr)
        else:
            print(f'nom déja utilisé')

    ctxt.funcs['let'] = define_let

    letinp = '''(
    (let name (add 12 45)) 
    (let name 14) 
    (add 12 name))'''

    result = parser.parse(input=letinp, lexer=lexer)
    print(result)
    sum = reduce_expression(result, ctxt)
    print(sum)
