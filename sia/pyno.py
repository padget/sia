# Pyno a pour but de fournir une asbtraction de sqlite
# et de pickle afin de fournier un service de stockage de
# de type dictionnaire NoSql

from pickle import dumps, loads
from sqlite3 import Connection, connect, Cursor, Error
from typing import Tuple, Any, List


class PynoEntry:
    """

    """

    def __init__(self, id: str, tp: str, data: object):
        """Initialize a new PynoEntry

        Args:
            id: An unique id to identify the data
            tp: A string that represents the type of the data
            data: Any python object to save under id and tp
        """

        self.id = id
        self.tp = tp
        self.data = data

    @staticmethod
    def from_row(row: Tuple[str, str, Any]):
        """Build a new PynoEntry from a row

        Args:
            row: A tuple where we can fin the tree args
            needed in the PynoEntry.__init__(...) function.

        Returns:
            A new instance of PynoEntry built with an id
            a type and a data
        """

        return PynoEntry(row[0], row[1], loads(row[2]))


class Pyno:
    def __init__(self, connection: Connection):
        """Initialize a new instance of Pyno

        A Pyno instance represents a special sqlite3
        format that behaves like a NoSql dictionnary
        database.

        Args:
            connection: a sqlite3 connection
        """

        self.connection = connection

    def __del__(self):
        """Destruction of Pyno object

        Close the remaining connection if needed
        """

        if self.connection is not None:
            self.connection.close()

    def save(self):
        """Save all the objects in the buffer

        Commit all items in the temporary buffer
        into the persistant support
        """

        self.connection.commit()

    def insert(self, entry: PynoEntry):
        """Insert a new entry into the dictionary

        :param entry: entry to insert
        """

        self[(entry.id, entry.tp)] = entry.data

    def __execquery(self, query, params=None) -> Cursor:
        """ Executes query passed by parameter with

        params

        :param query: query to execute
        :param params: parameters of the query (potentially None)
        :return: the current instance of Cursor
        """

        try:
            return self.connection.execute(query, params)
        except Error as e:
            print(f"An error occured {e}")

    def __getone(self, id: str) -> PynoEntry:
        """ Returns the object whose id is same as the id passed
        by parameter

        :param id: to identify the wanted object
        :return: the object with id
        """

        query = 'select * from pyno as p where p.id = ?'
        one = self.__execquery(query, [id])

        if one is not None:
            one = one.fetchone()

        return PynoEntry.from_row(one) if one else None

    def __getall(self, tp: str) -> List[PynoEntry]:
        """Returns all items from the Pyno that matches tp

        Args:
            tp: a string that represents the wanted class of items

        Returns:
            A sequence of the items (PynoEntry) from Pyno
            dictionnary that matches the tp parameter
        """

        query = 'select * from pyno where type = ?'
        for e in self.__execquery(query, [tp]):
            yield PynoEntry.from_row(e)

    def __getitem__(self, key: tuple):
        """Returns items in function of key parameter

        Args:
            key: A tuple of keys : one str for id and
            one str for type

        Returns:
            An item if only id is set or a list of items
            if type is set and not id
        """

        if len(key) == 2:
            if key[0]:
                return self.__getone(key[0])
            if key[1] and not key[0]:
                return self.__getall(key[1])
            if not key[0] and not key[1]:
                return None

    def __setone(self, e: PynoEntry):
        """Inserts one entry into the dictionary

        :param e: entry to insert
        """

        query = 'insert into pyno values (?, ?, ?)'
        id = e.id
        tp = e.tp
        data = dumps(e.data)
        self.__execquery(query, [id, tp, data])

    def __setitem__(self, key: tuple, data: Any):
        if len(key) == 2 and key[0] and key[1]:
            e = PynoEntry(key[0], key[1], data)
            self.__setone(e)

    def __delone(self, id: str):
        """Deletes the object whose id is id

        :param id: id of the object to delete
        """

        query = 'delete from pyno where id = ?'
        self.__execquery(query, [id])

    def __delitem__(self, id: str):
        self.__delone(id)


def open_pyno(name: str) -> Pyno:
    connection = connect(name)
    connection.executescript('''
        CREATE TABLE IF NOT EXISTS pyno (
            id text primary key,
            type text not null,
            data binary not null
        )''')
    connection.commit()
    return Pyno(connection)
