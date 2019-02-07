from dataclasses import dataclass


@dataclass
class Sia:
    declarations: list


class Expression:
    pass


class Declaration(Expression):
    pass


@dataclass
class Number(Expression):
    value: int


@dataclass
class String(Expression):
    value: str


@dataclass
class Name(Expression):
    value: str


@dataclass
class FnCall(Expression):
    fname: Name
    params: list


@dataclass
class MemberAccess(Expression):
    expr: Expression
    member: Name


@dataclass
class SpreadFnCall(Expression):
    expr: Expression
    fname: Name


@dataclass
class FnDeclaration(Declaration):
    fname: Name
    args: list
    rtype: Name
    aliases: list
    retexpr: Expression


@dataclass
class CaseFnDeclaration(Declaration):
    pass


@dataclass
class TypeDeclaration(Declaration):
    tname: Name
    args: list


@dataclass
class Arg:
    aname: Name
    tname: Name


@dataclass
class Param:
    expr: Expression


@dataclass
class Alias:
    alname: Name
    expr: Expression
