import sia.pyno as pyno
from sia.grammar.definition import parser, lexer
from sia.core.core import _, pipe, to
from sia.steps.check_types import check_types
from sia.steps.fn_table import save_fns_table
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
    fn to_string() -> str3 {
        "12"
    }
   
    '''

    declarations = pyno.open_pyno(':memory:')
    sia = parser.parse(text, lexer=lexer)
    print(sia)

    save_fns_table(sia, declarations)
    save_types_table(sia, declarations)

    declarations.save()

    check_types(declarations)


if __name__ == "__main__":
    main()
