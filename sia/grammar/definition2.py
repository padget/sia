import functools
from string import ascii_letters, digits
import ply

letters = ascii_letters + '_'
alphanums = letters + digits


class StringView:
    '''
    Une string view est une représentation logique
    d'une string en cours de lecture. La position de
    lecture est donc conservée. Une StringView conserve
    aussi la longueur de la chaine ainsi que la valeur
    même de la chaine
    '''

    def __init__(self, pos: int, value: str):
        self.__pos = pos
        self.__value = value
        self.__len = len(value) if value else 0

    def get_range(self):
        return range(self.__pos, self.__len)

    def clone(self, pos=None, value=None):
        return StringView(pos or self.__pos, value or self.__value)


class Token:
    def __init__(self, value: str, name: str, view: StringView):
        self.value = value
        self.name = name
        self.view = view


def get_terminals(module_name):
    module = __import__(module_name)
    for name in dir(module):
        attr = getattr(module, name)
        if callable(attr):
            if attr.__name__.startswith('t_'):
                yield attr


def get_error(module_name):
    mod = __import__(module_name)
    if 'on_error' in dir(mod):
        return getattr(mod, 'on_error')
    else:
        return None


def get_ignore(module_name):
    mod = __import__(module_name)
    if 'on_ignore' in dir(mod):
        return getattr(mod, 'on_ignore')
    else:
        return None


class Grammar:
    def __init__(self, terminals=[]):
        self.terminals = terminals


def terminal(detect_token):
    def decorate(react):
        tname = react.__name__[2:]

        @functools.wraps(react)
        def wrapper(view: StringView):
            new_view: StringView = detect_token(view)
            token = Token.from_view(new_view.__pos, tname, view.pos)

            if new_view.pos > view.pos:
                react(token)

            return token

        return wrapper

    return decorate


def ignore(ignored: str):
    def decorate(react):
        tname = 'ignore'

        @functools.wraps(react)
        def wrapper(view: StringView):
            while view.value[view.pos] in ignored:
                view = view.clone(pos=view.pos + 1)
                token = Token(view.value[view.pos], tname, view)
                react(token)

            return Token(view.value[view.pos], tname, view)

        return wrapper

    return decorate


def __while(view: StringView, pred):
    for i in view.get_range():
        if not pred(view.value[i]):
            return view.clone(pos=i)
        elif i == view.len - 1:
            return view.clone(pos=view.len)

    return view


def __detect_name(view: StringView):
    first_view: StringView = __while(view, lambda c: c in letters)

    if first_view.pos == view.pos:
        return view
    else:
        return __while(view, lambda c: c in alphanums)


def __detect_number(view: StringView):
    return __while(view, lambda c: c in digits)


@terminal(__detect_number)
def t_number(token: Token):
    print(token.value)


@terminal(__detect_name)
def t_name(token: Token):
    print(token.value)


@ignore(' \n\t')
def on_ignore(token: Token):
    print(f'jignore un espace {token}')


def on_error(token: Token):
    print(f'une erreur est survenue {token}')


def tokens(input: str, module_name: str):
    terminals = [term for term in get_terminals(module_name)]
    ignore = get_ignore(module_name)
    error = get_error(module_name)
    view: StringView = StringView(0, input)

    while view.pos < view.len:
        current_pos = view.pos
        view = ignore(view).view
        for term in terminals:
            token = term(view)
            if current_pos != token.view.pos:
                view = token.view
                yield token
        if current_pos == view.pos:
            error(Token.from_view(view, 'error'))
            view = StringView(view.pos + 1, view.value)


if __name__ == '__main__':
    value = 'coucou je65465q4sd__ suis un nom^^^^'

    tokens(value, __name__)
