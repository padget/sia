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


def sialfn(name: str, sctx: SialContext):
    def decorator(f):
        sctx.funcs[name] = f
        return f

    return decorator


@dataclass(frozen=True)
class Undefined(Expresssion):
    value: str


def reduce_expression(expr: Expresssion, ctxt: SialContext, wait_value=False):
    r = {
        List: lambda e: reduce_list(e, ctxt),
        Ident: lambda e: reduce_ident(e, ctxt),
        Number: lambda e: reduce_native(e),
        String: lambda e: reduce_native(e)
    }[type(expr)](expr)

    if wait_value and not isinstance(r, (Number, String)):
        return reduce_expression(r, ctxt)
    else:
        return r


def reduce_ident(ident: Ident, ctxt: SialContext):
    key = ident.value

    if key in ctxt.funcs:
        return ctxt.funcs[key]
    elif key in ctxt.lets:
        return ctxt.lets[key]
    else:
        return Undefined(value=key)


def reduce_native(obj):
    if isinstance(obj, Number):
        return obj
    elif isinstance(obj, String):
        return obj


def reduce_list(lst: List, ctxt: SialContext):
    if isinstance(lst.expressions[0], Ident):
        identv = reduce_ident(lst.expressions[0], ctxt)

        if callable(identv):
            r = identv(*lst.expressions[1:])
            return r
    else:
        return List([reduce_expression(expr, ctxt) for expr in lst.expressions])


sctx = SialContext()


@sialfn('add', sctx)
def add(x: int, y: int):
    return Number(reduce_expression(x, sctx, True).value +
                  reduce_expression(y, sctx, True).value)


@sialfn('minus', sctx)
def add(x: int, y: int):
    return Number(reduce_expression(x, sctx, True).value -
                  reduce_expression(y, sctx, True).value)


@sialfn('mult', sctx)
def add(x: int, y: int):
    print("")
    return Number(reduce_expression(x, sctx, True).value *
                  reduce_expression(y, sctx, True).value)


@sialfn('div', sctx)
def add(x: int, y: int):
    return Number(reduce_expression(x, sctx, True).value /
                  reduce_expression(y, sctx, True).value)


@sialfn('let', sctx)
def define_let(ident, expr):
    found = reduce_ident(ident, sctx)
    if isinstance(found, Undefined):
        print('inside let ', expr)
        sctx.lets[found.value] = expr
    else:
        print(f'nom déja utilisé')


if __name__ == '__main__':
    letinp = '''
    (
        (let left 12)  
        (let square (mult left left))
        (add square 2)
    )
    '''

    result = parser.parse(input=letinp, lexer=lexer)
    print(result)
    sum = reduce_expression(result, sctx)
    print(sum)
