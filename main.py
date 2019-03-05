from sia.grammar.definition import parser, lexer
from sia.pyno import open_pyno
from sia.steps.check_types import check_types
from sia.steps.fn_table import save_fns_table
from sia.steps.types_table import save_types_table

if __name__ == '__main__':
    with open('main.sia', 'r') as file:
        inp = file.read()

    parse_result = parser.parse(input=inp, lexer=lexer)
    py = open_pyno('local_dev')
    save_fns_table(parse_result, py)
    save_types_table(parse_result, py)
    check_types(py)
