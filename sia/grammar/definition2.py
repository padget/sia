from string import ascii_letters, digits

from sia.grammar.lexer import terminal, ignore, ignore_function, error_function, \
    terminal_functions
from sia.grammar.string_view import StringView
from sia.grammar.token import Token

LETTERS = ascii_letters + '_'
ALPHA_NUMS = LETTERS + digits


def __while(view: StringView, pred):
    for i in view.get_range():
        if not pred(view.char(i)):
            return view.clone(pos=i)
        elif i == view.len() - 1:
            return view.clone(pos=view.len())

    return view


def __detect_name(view: StringView):
    first_view: StringView = __while(view, lambda c: c in LETTERS)

    if first_view.pos() == view.pos():
        return view
    else:
        return __while(view, lambda c: c in ALPHA_NUMS)


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


def tokens(input: str, module):
    terminals = [term for term in terminal_functions(module)]
    ignore = ignore_function(module)
    error = error_function(module)
    view: StringView = StringView(0, input)
    length = len(input)

    while view.pos() < length:
        current_pos = view.pos()
        view = ignore(view).view

        for term in terminals:
            token = term(view)

            if current_pos != token.view.pos():
                view = token.view
                yield token

        if current_pos == view.pos():
            error(Token(view.char(), 'error', view))
            view = view.clone(pos=view.pos() + 1)


