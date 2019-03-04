from sia.core.core import _
from sia.pyno import Pyno


def check_types(declarations: Pyno):
    """

    :param declarations:
    """

    tpdecls = [tp.data for tp in declarations[(_, 'type')]]
    fndecls = [fn.data for fn in declarations[(_, 'function')]]

    types_references = [tp.tname.value for tp in tpdecls]

    __detect_error(tpdecls, types_references)
    __detect_error(fndecls, types_references)


def __map_to_args_name(declarations):
    for decl in declarations:
        for arg in decl.args:
            yield arg.tname.value


def __types_not_found(types_names, types_reference):
    for tp in types_names:
        if tp not in types_reference:
            yield tp


def __detect_error(declarations, types_references):
    arg_names = __map_to_args_name(declarations)

    for tp in __types_not_found(arg_names, types_references):
        print(f'{tp} n\'est pas dans la liste des types disponibles')
