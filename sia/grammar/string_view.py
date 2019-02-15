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

    def pos(self):
        return self.__pos

    def char(self, i: int = None):
        return self.__value[i or self.pos()]

    def chars(self, f, t):
        return self.__value[f:t]

    def len(self):
        return self.__len

    def clone(self, pos=None, value=None):
        return StringView(pos or self.__pos, value or self.__value)
