import sia.steps.check_types as chtypes
import sia.grammar.definition as definition

import sia.pyno as pyno
from sia.grammar.definition import parser, lexer
from sia.core.core import _
from sia.steps.types_table import save_types_table


def main():
    text = '''
    type int()
    type float()
    type str()
    type person(name : str, firstname: str)
    type person2(name : str2, firstname: str)
    type person3(name : str, firstname: str)
    type person4(name : str, firstname: str)
   
    '''

    data = pyno.open_pyno(':memory:')
    sia = parser.parse(text, lexer=lexer)
    print(sia)
    # save_fns_table(sia, data)
    save_types_table(sia, data)

    data.save()

    for tp in [tp.data for tp in data[(_, 'type')]]:
        for tname in [arg.tname for arg in tp.args]:
            data[(tname.value, _)]


if __name__ == "__main__":
    main()
