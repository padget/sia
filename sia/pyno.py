# Pyno a pour but de fournir
# une asbtraction de sqlite
# et de pickle afin de fournier
# un service de stockage de
# de type dictionnaire NoSql

from dataclasses import dataclass
from sqlite3 import Connection, connect, Cursor, Error
from enum import Enum, auto
from pickle import dumps, loads


class PynoPhase(Enum):
    READ = auto()
    WRITE = auto()


class PynoEntry:
    def __init__(self, id, tp, data):
        self.id = id
        self.tp = tp
        self.data = data

    @staticmethod
    def from_row(row):
        return PynoEntry(row[0], row[1], loads(row[2]))


class Pyno:
    def __init__(self, connection):
        self.connection = connection

    def execute_query(self, query, params=None) -> Cursor:
        try:
            return self.connection.execute(query, params)
        except Error as e:
            print(f"An error occured {e}")

    def get_one(self, id: str) -> PynoEntry:
        query = 'select p.id, p.type, p.data from pyno as p where p.id = ?'
        one = self.execute_query(query, [id])

        if one is not None:
            one = one.fetchone()

        return PynoEntry.from_row(one) if one else None

    def insert_one(self, e: PynoEntry) -> bool:
        query = 'insert into pyno(id, type, data) values (?, ?, ?)'
        self.execute_query(query, [e.id, e.tp, dumps(e.data)])

    def remove_one(self, id: str):
        query = 'delete from pyno where id = ?'
        self.execute_query(query, [id])

    def save(self):
        self.connection.commit()

    def get_all_of(self, tp: str, func):
        query = 'select * from pyno where type = ?'
        for e in self.execute_query(query, [tp]):
            yield PynoEntry.from_row(e)


def open_pyno(name: str) -> Pyno:
    connection = connect(name)
    cursor = connection.cursor()
    cursor.executescript('''
        CREATE TABLE IF NOT EXISTS pyno (
            id text primary key,
            type text not null,
            data binary not null
        )''')
    connection.commit()
    return Pyno(connection)
