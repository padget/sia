from sia.pyno import Pyno, PynoEntry
from sia.grammar.definition import Sia, FnDeclaration, FnDeclarationNative
from sia.core.core import pipe, foreach, filter, to


def save_fns_table(sia: Sia, fns: Pyno):
    """ Save all found fn declarations

    :param sia: Sia object where search the fn
                declarations
    :param fns: The Pyno dictionary to store the
                  fn declarations
    """

    pipe(
        to(lambda sia: sia.declarations),
        filter(lambda x: isinstance(x, (FnDeclaration, FnDeclarationNative))),
        foreach(
            lambda fndecl: PynoEntry(fndecl.fname.value, 'function', fndecl),
            lambda pynofn: fns.insert(pynofn)))(sia)
