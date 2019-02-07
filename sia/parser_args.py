from core import production


@production('args : arg args_tail')
def p_args(yprod):
    yprod[0] = [yprod[1]] + yprod[2]


@production('args : empty')
def p_args_empty(yprod):
    yprod[0] = []


@production('args_tail : comma arg args_tail')
def p_args_tail(yprod):
    yprod[0] = [yprod[2]] + yprod[3]


@production('args_tail : empty')
def p_args_tail_empty(yprod):
    yprod[0] = []


@production('arg : name colon name')
def p_arg(yprod):
    from parser_ast import Arg, Name
    yprod[0] = Arg(aname=Name(value=yprod[1]), tname=Name(value=yprod[3]))
