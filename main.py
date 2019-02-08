import sia.steps.check_types as chtypes
import sia.grammar.definition as definition

import sia.pyno as pyno
from sia.grammar.definition import parser, TypeDeclaration, FnDeclaration, lexer
from sia.core.core import new, _


class PynoFnDeclaration(pyno.PynoEntry):
    def __init__(self, fn: FnDeclaration):
        super().__init__(fn.fname.value, 'function', fn)


class PynoTypeDeclaration(pyno.PynoEntry):
    def __init__(self, tp: TypeDeclaration):
        super().__init__(tp.tname.value, 'type', tp)


def main():
    text = '''
    type int()
    type float()
    type str()
    type person(name : str, firstname: str)
    type person2(name : str2, firstname: str)
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

    DECLARATION_TYPES = {
        TypeDeclaration: PynoTypeDeclaration,
        FnDeclaration: PynoFnDeclaration
    }

    for decl in declarations:
        pynodecl = new(DECLARATION_TYPES[decl.__class__], decl)
        data[(pynodecl.id, 'type')] = pynodecl.data

    data.save()

    for tp in [tp.data for tp in data[(_, 'type')]]:
        for tname in [arg.tname for arg in tp.args]:
            data[(tname.value, _)]

    import os
    os.remove('d:/pyno.db')


if __name__ == "__main__":
    main()
