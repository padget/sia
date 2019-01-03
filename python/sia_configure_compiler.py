# coding=utf-8

import sqlite3
import sys

def drop_table(conn, table_name):
  query = f'drop table if exists {table_name}'
  conn.execute(query)

def create_table(conn, table_name, columns):
  columns_str = ', '.join([' '.join(column) for (column) in columns])
  query       = f'create table if not exists {table_name} ({columns_str})'
  conn.execute(query)

def parse_properties_file(filename):
  with open(filename, 'r') as properties:
    return [tuple(line.split('=')) for line in properties.readlines()]

if __name__ == '__main__':
  print('début configuration compilateur sia')

  if len(sys.argv) >= 3: 
    filename = sys.argv[2]
    conn     = sqlite3.connect('sia.config.db')

    drop_table(conn, 'sia_configuration')
    create_table(conn, "sia_configuration", 
          [("key", "text", "primary key"), 
          ("value", "text", "not null")])

    conn.commit()

    for (key, value) in parse_properties_file(filename):
      conn.execute(f'''
        insert into sia_configuration (key, value)
        values ('{key}', '{value}')''')

    conn.commit()
    conn.close()

  print('fin configuration compulateur sia')