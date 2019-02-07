from parser_ast import Name
import pyno
from parser_ast import TypeDeclaration, FnDeclaration, Expression
import ply.yacc as yacc

# set current lexer
from lexer import lexer
# import tokens from lexer
from lexer import tokens

from core import production, _, new
from parser_expression import *
from parser_args import *
from parser_type_declaration import *
from parser_alias_statement import *
from parser_fn_declaration import *
from parser_casefn_declaration import *

# SIA PARSER

start = 'sia'


@production('empty :')
def p_empty(yprod):
    pass


@production('sia : declarations')
def p_sia(yprod):
    from parser_ast import Sia
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

text = '''
type person(name : str, firstname: str)
type person2(name : str, firstname: str)
type person3(name : str, firstname: str)
type person4(name : str, firstname: str)
fn to_name(p: person) -> str {
    alias p = (p, p, 12, (toto, 12).add()).to_string()
    alias toto = fn def() -> person {
        p.toto
    }

    fn def() -> person {
        p.toto
    }::name()
}
'''

data = pyno.open_pyno('d:/pyno.db')
declarations = parser.parse(text, lexer=lexer).declarations


class PynoFnDeclaration(pyno.PynoEntry):
    def __init__(self, fn: FnDeclaration):
        super().__init__(fn.fname.value, 'function', fn)


class PynoTypeDeclaration(pyno.PynoEntry):
    def __init__(self, tp: TypeDeclaration):
        super().__init__(tp.tname.value, 'type', tp)


pynoDeclarations = {
    FnDeclaration: PynoFnDeclaration,
    TypeDeclaration: PynoTypeDeclaration
}


for decl in declarations:
    pynodecl = new(pynoDeclarations[decl.__class__], decl)
    data[(pynodecl.id, 'type')] = pynodecl

data.save()

print('avant le del')
for tp in [tp.data for tp in data[(_, 'type')]]:
    print(tp)


del data['person']

print('après le del')

for tp in [tp.data for tp in data[(_, 'type')]]:
    print(tp)
