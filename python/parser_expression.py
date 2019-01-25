from core import production


@production('''
    expression : number
               | name
               | fn_call''')
def p_expression(yprod):
    pass


@production('fn_call : lbracket params rbracket point name lbracket rbracket')
def p_fn_call(yprod):
    pass


@production('''
    params : expression params_tail
           | empty''')
def p_params(yprod):
    pass


@production('''  
    params_tail : comma expression params_tail
                | empty''')
def p_params_tail(yprod):
    pass
