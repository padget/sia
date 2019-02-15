import functools

from sia.grammar.string_view import StringView
from sia.grammar.token import Token


def terminal_functions(mod):
    return (getattr(mod, name) for name in dir(mod) if name.startswith('t_'))


def error_function(mod):
    if 'on_error' in dir(mod):
        return getattr(mod, 'on_error')
    else:
        return None


def ignore_function(mod):
    if 'on_ignore' in dir(mod):
        return getattr(mod, 'on_ignore')
    else:
        return None


def terminal(detect_token):
    def decorate(react):
        tname = react.__name__[2:]

        @functools.wraps(react)
        def wrapper(view: StringView):
            new_view: StringView = detect_token(view)
            token = Token(new_view.chars(view.pos(), new_view.pos()), tname, new_view)

            if new_view.pos() > view.pos():
                react(token)

            return token

        return wrapper

    return decorate


def ignore(ignored: str):
    def decorate(react):
        tname = 'ignore'

        @functools.wraps(react)
        def wrapper(view: StringView):
            while view.char() in ignored:
                view = view.clone(pos=view.pos() + 1)
                token = Token(view.char(), tname, view)
                react(token)

            return Token(view.char(), tname, view)

        return wrapper

    return decorate
