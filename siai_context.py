class ScopedContext:
    def __init__(self, name: str, type: str, subs=[], parent=None):
        """

        :param parent:
        :param name:
        :param subs:
        """

        self.parent = parent
        self.name = name
        self.type = type
        self.subs = subs

    def append(self, name):
        """

        :param name:
        :return:
        """

        sub = ScopedContext(name=name)
        self.subs.append(sub)
        return sub

    def __contains__(self, key: str, type: str = None) -> bool:
        """

        :param key:
        :return:
        """

        return self.name == key or self.__in_subs(key) or self.__in_parent(key)

    def find(self, key: str):
        """

        :param key:
        :return:
        """
        if self.name == key:
            return self
        else:
            asub = next(sub for sub in self.subs if sub.name == key)
            if asub is not None:
                return asub
            else:
                return self.parent.find(key)

    def __in_parent(self, key: str) -> str:
        """

        :param key:
        :return:
        """

        return self.parent is not None and self.parent.find(key)

    def __in_subs(self, key: str) -> bool:
        """

        :param key:
        :return:
        """

        keys = (sub for sub in self.subs if key == sub.name)
        return next(keys, None) is not None


if __name__ == '__main__':
    scope = ScopedContext(name='global')
    scope.append('age')
    scope.append('name')
    scope.append('firstname')
    print('firstname' in scope)
    print('global' in scope)
    print('globale' in scope)
