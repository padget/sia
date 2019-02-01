from core import production


@production('expression : number')
def p_expression_number(yprod):
    from parser_ast import number
    yprod[0] = number(value=yprod[1])


@production('expression : string')
def p_expression_string(yprod):
    from parser_ast import string
    yprod[0] = string(value=yprod[1])


@production('expression : name')
def p_expression_name(yprod):
    from parser_ast import name
    yprod[0] = name(value=yprod[1])


@production('expression : fn_call')
def p_expression_fn_call(yprod):
    yprod[0] = yprod[1]


@production('expression : fn_declaration')
def p_expression_fn_declaration(yprod):
    yprod[0] = yprod[1]


@production('expression : member_access')
def p_expression_member_access(yprod):
    yprod[0] = yprod[1]


@production('member_access : expression point name')
def p_member_access(yprod):
    from parser_ast import member_access
    yprod[0] = member_access(expr=yprod[1], member=yprod[3])


@production('expression : member_access')
def p_expression_member_access(yprod):
    yprod[0] = yprod[1]


@production('expression : spread_fn_call')
def p_expression_spread_fn_call(yprod):
    yprod[0] = yprod[1]


@production('spread_fn_call : expression point point name lbracket rbracket')
def p_member_spread_fn_call(yprod):
    from parser_ast import spread_fn_call
    yprod[0] = spread_fn_call(expr=yprod[1], fname=yprod[4])


@production('fn_call : lbracket params rbracket point name lbracket rbracket')
def p_fn_call(yprod):
    from parser_ast import fn_call
    yprod[0] = fn_call(fname=yprod[5], params=yprod[2])


@production('params : expression params_tail')
def p_params(yprod):
    yprod[0] = [yprod[1]] + yprod[2]


@production('params : empty')
def p_params_empty(yprod):
    yprod[0] = []


@production('params_tail : comma expression params_tail')
def p_params_tail(yprod):
    yprod[0] = [yprod[2]] + yprod[3]


@production('params_tail : empty')
def p_params_tail_empty(yprod):
    yprod[0] = []


@production('simple_expression : number')
def p_simple_expression_number(yprod):
    from parser_ast import number
    yprod[0] = number(value=yprod[1])


@production('simple_expression : string')
def p_simple_expression_string(yprod):
    from parser_ast import string
    yprod[0] = string(value=yprod[1])


@production('simple_expression : name')
def p_simple_expression_name(yprod):
    from parser_ast import name
    yprod[0] = name(value=yprod[1])


@production('simple_params : simple_expression simple_params_tail')
def p_simple_params(yprod):
    yprod[0] = [yprod[1]] + yprod[2]


@production('simple_params : empty')
def p_simple_params_empty(yprod):
    yprod[0] = []


@production('simple_params_tail : comma simple_expression simple_params_tail')
def p_simple_params_tail(yprod):
    yprod[0] = [yprod[2]] + yprod[3]


@production('simple_params_tail : empty')
def p_simple_params_tail_empty(yprod):
    yprod[0] = []
