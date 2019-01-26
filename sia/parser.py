import ply.yacc as yacc

# set current lexer
from lexer import lexer
# import tokens from lexer
from lexer import tokens

from core import production

# Import des regles de production pour une expression :
#  - function call
#  - params
from parser_expression import *

# Import des regles de production pour les args : 
#   - args
#   - arg
from parser_args import *

# Import des regles de production pour la déclaration d'un type : 
#   - type declaration
from parser_type_declaration import *

# Import des regles de production pour un alias : 
#   - alias
from parser_alias_statement import *

# Import des regles de production pour la déclaration d'une fonction : 
#   - fn declaration
#   - aliases
from parser_fn_declaration import *

# Import des regles de production pour la déclaration d'une case fonction : 
#   - casefn declaration
from parser_casefn_declaration import *


# SIA PARSER

start = 'sia'

@production('empty :')
def p_empty(yprod):
    pass

@production('sia : declarations')
def p_sia(yprod):
    yprod[0] = 12


@production('''
    declarations : declaration declarations
                 | empty''')
def p_declarations(yprod):
    pass


@production('''
    declaration : type_declaration
                | fn_declaration
                | casefn_declaration''')
def p_declaration(yprod):
    pass


parser = yacc.yacc()

text = '''
type person(name : str, firstname: str)
type person(name : str, firstname: str)
fn to_name(p: person):str {
    alias p = (p, p, 12, (toto, 12).add()).to_string()
    ().to_strint()
}
casefn name(12, 12, 12){12}
'''

print(parser.parse(text, lexer=lexer))
