import sys

from sia.grammar.definition2 import tokens

if __name__ == '__main__':
    import sia.grammar.definition2 as definition

    value = 'coucou je65465q4sd__ suis un nom'

    for token in tokens(value, definition):
        print(token.value)
