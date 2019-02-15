from sia.grammar.string_view import StringView


class Token:
    def __init__(self, value: str, name: str, view: StringView):
        self.value = value
        self.name = name
        self.view = view
