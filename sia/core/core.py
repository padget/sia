

def production(rule: str):
    def decorate(fprod):
        fprod.__doc__ = rule
        return fprod
    return decorate


def token(rx: str):
    def decorate(ftoken):
        ftoken.__doc__ = rx
        return ftoken
    return decorate


def new(cls, *args):
    ist = object.__new__(cls, *args)
    ist.__init__(*args)
    return ist


_ = None
