from core import production


@production('''
    args : arg args_tail 
         | empty''')
def p_args(yprod):
    pass


@production('''
    args_tail : comma arg args_tail
              | empty''')
def p_args_tail(yprod):
    pass


@production('arg : name colon name')
def p_arg(yprod):
    pass
