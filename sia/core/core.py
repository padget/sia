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


def pipe(fn, *fns):
    def apply(*args, **kwargs):
        if not fns:
            return fn(*args, **kwargs)
        else:
            return pipe(*fns)(fn(*args, **kwargs))

    return apply


def foreach(fn, *fns):
    def apply(seq):
        for item in seq:
            pipe(fn, *fns)(item)

    return apply


def filter(pred):
    def apply(seq):
        for item in seq:
            if pred(item):
                yield item

    return apply


def to(mapper):
    def apply(obj):
        return mapper(obj)

    return apply
