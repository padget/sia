from sia.core.core import _
from sia.pyno import Pyno


def check_name_in_functions(declarations: Pyno):
    fndecls = [fn.data for fn in declarations[(_, 'function')]]
    aliases = __map_to_aliases(fndecls)

    for alias in aliases:
        __detect_errors(alias)


def __map_to_aliases(al):
    pass


def __get_all_name_in_body(FnDeclaration):
    pass


def __detect_errors(al):
    pass
