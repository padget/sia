from sia.grammar.definition import Sia, TypeDeclaration
from sia.core.core import pipe, foreach, filter, to
from sia.pyno import PynoEntry, Pyno


def save_types_table(sia: Sia, types: Pyno):
    """ Save all found type declarations

    :param sia: Sia object where search the type
                declarations
    :param types: The Pyno dictionary to store the
                  type declarations
    """

    pipe(
        to(lambda sia: sia.declarations),
        filter(lambda x: isinstance(x, TypeDeclaration)),
        foreach(
            lambda tp: PynoEntry(tp.tname.value, 'type', tp),
            lambda pynotype: types.insert(pynotype)))(sia)
