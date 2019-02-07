from core import production


@production('expression : number')
def p_expression_number(yprod):
    from parser_ast import Number
    yprod[0] = Number(value=yprod[1])


@production('expression : string')
def p_expression_string(yprod):
    from parser_ast import String
    yprod[0] = String(value=yprod[1])


@production('expression : name')
def p_expression_name(yprod):
    from parser_ast import Name
    yprod[0] = Name(value=yprod[1])


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
    from parser_ast import MemberAccess, Name
    yprod[0] = MemberAccess(expr=yprod[1], member=Name(value=yprod[3]))


@production('expression : member_access')
def p_expression_member_access(yprod):
    yprod[0] = yprod[1]


@production('expression : spread_fn_call')
def p_expression_spread_fn_call(yprod):
    yprod[0] = yprod[1]


@production('spread_fn_call : expression colon colon name lbracket rbracket')
def p_member_spread_fn_call(yprod):
    from parser_ast import SpreadFnCall, Name
    yprod[0] = SpreadFnCall(expr=yprod[1], fname=Name(value=yprod[4]))


@production('fn_call : lbracket params rbracket point name lbracket rbracket')
def p_fn_call(yprod):
    from parser_ast import FnCall, Name
    yprod[0] = FnCall(fname=Name(value=yprod[5]), params=Name(value=yprod[2]))


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
    from parser_ast import Number
    yprod[0] = Number(value=yprod[1])


@production('simple_expression : string')
def p_simple_expression_string(yprod):
    from parser_ast import String
    yprod[0] = String(value=yprod[1])


@production('simple_expression : name')
def p_simple_expression_name(yprod):
    from parser_ast import Name
    yprod[0] = Name(value=yprod[1])


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
