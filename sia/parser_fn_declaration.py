from core import production


@production('''fn_declaration : fn_signature colon fn_lines''')
def p_fn_declaration(yprod):
    pass


@production('''fn_signature : fn name lbracket args rbracket arrow name''')
def p_fn_signature(yprod):
    pass


@production('''fn_lines : fn_line fn_lines 
                        | empty''')
def p_fn_lines(yprod):
    pass

@production('fn_line : indent fn_line_tail')
def p_fn_line(yprod):
    pass 


@production('''fn_line_tail : return expression 
                            | alias_statement''')
def p_fn_line_tail(yprod):
    pass 
