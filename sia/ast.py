from dataclasses import dataclass


@dataclass
class sia:
    declaration: list


class declaration:
    pass


@dataclass
class fn_declaration(declaration):
    fname: name
    args: list
    rtype: name
    aliases: list
    retexpr: expression


class casefn_declaration(declaration):
    pass

@dataclass
class type_declaration(declaration):
    tname: str 
    args: list


class expression:
    pass

@dataclass
class fn_call(expression):
    fname: str
    params: list

@dataclass
class number(expression):
    value: int

@dataclass
class string(expression):
    value: str

@dataclass
class name(expression):
    value: str

@dataclass
class arg:
    aname: name 
    tname: name

@dataclass
class param:
    expr: expression

@dataclass
class alias:
    alname: name
    expr: expression
