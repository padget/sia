from sia.pyno import Pyno
from sia.core.core import _

def check_types(declarations: Pyno):
    type_declarations = [tp.data for tp in declarations[(_, 'type')]]
    fn_declarations = [fn.data for fn in declarations[(_, 'function')]]

    type_names = [tp.tname.value for tp in type_declarations]

    for type_declaration in type_declarations:
        for arg in type_declaration.args:
            if arg.tname.value not in type_names:
                print(f'{arg.tname.value} n\'est pas dans la liste des types disponibles')

    for fn_declaration in fn_declarations:
        for arg in fn_declaration.args:
            if arg.tname.value not in type_names:
                print(f'{arg.tname.value} n\'est pas dans la liste des types disponibles')

        if fn_declaration.rtype.value not in type_names:
            print(f'{fn_declaration.rtype.value} n\'est pas dans la liste des types disponibles')
