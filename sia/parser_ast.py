from dataclasses import dataclass


@dataclass
class sia:
    declarations: list


class expression:
    pass


class declaration(expression):
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
class member_access(expression):
    expr: expression
    member: name


@dataclass
class spread_fn_call(expression):
    expr: expression
    fname: name


@dataclass
class fn_declaration(declaration):
    fname: name
    args: list
    rtype: name
    aliases: list
    retexpr: expression


@dataclass
class casefn_declaration(declaration):
    pass


@dataclass
class type_declaration(declaration):
    tname: str
    args: list


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
