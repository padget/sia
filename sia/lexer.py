import ply.lex as lex
from core import token

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
    'return': 'return'
}

tokens = ['indent', 'name'] + [l for l in lexems] + [r for r in reserved.values()]

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

@token(r'\n+')
def t_newline(t):
    t.lexer.lineno += len(t.value)

@token(r'^[ ]+')
def t_indent(t):
    return t

@token(r'\s+')
def t_ignored(t):
    pass


def t_error(t):
    print(f"Illegal character '{t.value[0]}'")
    t.lexer.skip(1)


import re
lexer = lex.lex(reflags=re.MULTILINE)
