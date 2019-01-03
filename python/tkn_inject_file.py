# coding=utf-8

import sqlite3
import os
import sys

def insert_one_line(conn, filename, line, num):
  length      = len(line)
  query       = f'''
    insert into tkn_file_lines   
    (filename, line, num, length)  
    values ('{filename}', '{line}', {num}, {length});\n'''
  conn.execute(query)

def read_all_lines(filename):
  with open(filename, 'r') as file: 
    return [line.rstrip(os.linesep) for line in file]

def insert_all_lines(conn, filename, lines):
    query = 'insert into tkn_file_lines (filename, line, num, length) values (?, ?, ?, ?);'
    lines_generated = ((filename, line, num, len(line)) for num, line in enumerate(lines))
    conn.executemany(query, lines_generated)

if __name__ == "__main__":
  print("debut injection fichier")
  
  filename    = 'lol2.sia'
  db_filename = f'{filename}.db'
  conn        = sqlite3.connect(db_filename) 
  lines       = read_all_lines(filename)
  
  insert_all_lines(conn, filename, lines)

  conn.commit()
  conn.close()

  print('fin injection fichier')