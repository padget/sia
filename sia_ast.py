from dataclasses import dataclass


def immutable(cls):
    return dataclass(frozen=True)(cls)


@immutable
class Sia:
    declarations: list


@immutable
class Expression:
    pass


@immutable
class TypeDeclaration(Expression):
    name: str
    attributes: list


@immutable
class Attribute:
    name: str


@immutable
class FnDeclaration(Expression):
    name: str
    arguments: list
    let_declarations: list
    expression: Expression


@immutable
class Argument:
    name: str


@immutable
class LetDeclaration(Expression):
    name: str
    expression: Expression


@immutable
class FnCall(Expression):
    name: str
    expressions: list


@immutable
class Number(Expression):
    value: int


@immutable
class String(Expression):
    value: str
