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
    def __init__(self, funcs):
        self.funcs = funcs


class Signature:
    def __init__(self, types, func):
        self.types = types
        self.func = func


native_funcs = {
    'add int int': lambda x, y: x + y,
    'add str str': lambda x, y: x + y,
    'add str int': lambda x, y: f'{x}{y}',
    'add int str': lambda x, y: f'{x}{y}'
}


def build_funcs(lst: List = None):
    funcs = native_funcs.copy()

    return funcs


def type_from(o):
    return {
        Number: 'int',
        String: 'str'
    }.get(type(o))


def is_function_in_sialcontext(ident: Ident, ctx: SialContext):
    return ident.value in ctx.funcs


def get_function_from_signature(ident: Ident, args, ctx: SialContext):
    types = [type_from(arg) for arg in args]
    Signature = signature = ' '.join([ident] + types)


def interpret_list(lst: List, ctx: SialContext):
    first = lst[0]
    tails = lst[1:]

    # il faut d'abort intéprété tails avant de pouvoir intéprété first

    if isinstance(first, Ident):
        if is_function_in_sialcontext(first, ctx):
            return interpret_fcall(first, tails, ctx)
        else:
            return interpret_ident(first, ctx)
    elif isinstance(first, Number):
        return interpret_number(first)
    elif isinstance(first, String):
        return interpret_string(first)


def interpret_fcall(ident: Ident, args, ctx: SialContext):
    pass


def interpret_ident(ident: Ident, ctx: SialContext):
    pass


def interpret_number(n: Number):
    return n


def interpret_string(s: String):
    return s


def interpret(lst: List, ctx: SialContext):
    if lst.expressions:
        first = lst.expressions[0]

        if isinstance(first, Ident):
            ident = first.value
            args = [ex for ex in lst.expressions[1:]]
            signature = ' '.join([ident] + [type_from(arg) for arg in args])

            if signature in ctx.funcs:
                react: list = ctx.funcs[signature]
                return react(*(arg.value for arg in args))
            else:
                print(
                    f'pas de signature disponible correspondant à {signature}')


if __name__ == '__main__':
    simple_add = '(add 1 "2")'

    ctx = SialContext(build_funcs(None))
    result = parser.parse(input=simple_add, lexer=lexer)

    sum = interpret(result, ctx)
    print(sum)
