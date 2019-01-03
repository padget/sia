# coding=utf-8

import sqlite3
import string

def dao_lines_from_filename(sqlcursor, filename):
  query = '''select filename, line, num, length 
             from tkn_file_lines 
             where filename=?'''
  return sqlcursor.execute(query, (filename,)).fetchall()


def dao_insert_one_token(sqlcursor, filename, line, column, value, tkntype): 
  query = f'''
    insert into tkn_token
    (filename, line, column, value, type)   
    values ('{filename}', {line}, {column}, '{value}', '{tkntype}');\n'''
  sqlcursor.execute(query)


class view(object):

  def __init__(self, data, current = 0):
    self.__current = current
    self.__data    = data


  def head(self):
    return self.__data[:self.__current + 1]


  def tail(self):
    return self.__data[self.__current:]


  def current(self):
    return self.__data[self.__current]


  def indice(self):
    return self.__current


  def next(self):
    return view(self.__data, self.__current + 1)

  
  def has_current(self):
    return self.__current < len(self.__data)


def is_one(v: view, one):
  return v.current() == one 


def is_in(v: view, ins):
  return v.current() in ins


def skip_some(v: view, is_functor):
  if v.has_current() and is_functor(v):
      return skip_some(v.next(), is_functor)
  else:
    return v


def skip_multi(v: view, skip_functors):
  return skip_multi(skip_functors[0](v), skip_functors[1:]) if skip_functors else v


def detect_token(v: view, skip_functors, tktype):
  v_skipped =  skip_multi(v, skip_functors)
  return (v_skipped, tktype, v.indice(), v_skipped.indice()) if v_skipped.indice() > v.indice() else None


def detect_one_among(v: view, detect_functors):
  if v.has_current():
    for fn in detect_functors:
      tkn = fn(v)
      if tkn: 
        return tkn
  return None


def detect_tokens_tr(tkns, v: view, detect_functors):
  tkn = detect_one_among(v, detect_functors)
  
  if tkn:
    return detect_tokens_tr(tkns + [tkn], tkn[0], detect_functors)
  else: 
    return tkns


def detect_tokens(v: view, detect_functors):
  return detect_tokens_tr([], v, detect_functors)


blanks    = string.whitespace
letters   = string.ascii_lowercase + string.ascii_uppercase + '_'
alphanums = string.digits + letters


def detect_name(v: view):
  is_letters      = lambda v: is_in(v, letters)
  is_alphanums    = lambda v: is_in(v, alphanums)
  skip_alphanums  = lambda v: skip_some(v, is_alphanums)
  skip_alpha      = lambda v: skip_some(v, is_letters)
  return detect_token(v, [skip_alphanums, skip_alpha], 'name')


def detect_blank(v: view):
  is_blank   = lambda v: is_in(v, blanks)
  skip_blank = lambda v: skip_some(v, is_blank)
  return detect_token(v, [skip_blank], 'blank')


def detect_number(v: view):
  is_digit   = lambda v: is_in(v, string.digits)
  skip_digit = lambda v: skip_some(v, is_digit)
  return detect_token(v, [skip_digit], 'number')


def detect_one(letter, tkntype):
  return (lambda v: detect_token(v, [(lambda v: skip_some(v, (lambda v: is_one(v, letter))))], tkntype))


def tokenize(line):
  v = view(line)
  return detect_tokens(v, [detect_blank, detect_name,  detect_number,
     detect_one('(', 'lbracket'), 
     detect_one(')', 'rbracket'), 
     detect_one('{', 'lbrace'),
     detect_one('}', 'rbracket'), 
     detect_one(':', 'colon'), 
     detect_one('.', 'point'), 
     detect_one(',', 'comma'), 
     detect_one(';', 'semi_colon')])


if __name__ == '__main__':

  print('début tokenization du fichier source')
  filename = 'lol2.sia'
  conn = sqlite3.connect('lol2.sia.db')
  sqlite3_cursor = conn.cursor()

  for file_line in dao_lines_from_filename(sqlite3_cursor, filename):
    for tkn in [tkn for tkn in tokenize(file_line[1]) if tkn[1] != 'blank']:
      dao_insert_one_token(sqlite3_cursor, filename, file_line[2], tkn[2], file_line[1][tkn[2]:tkn[3]], tkn[1])

  sqlite3_cursor.executescript(
    '''
      update tkn_token set type='fn'     where value = 'fn'     ; 
      update tkn_token set type='casefn' where value = 'casefn' ; 
      update tkn_token set type='type'   where value = 'type'   ; 
      update tkn_token set type='alias'  where value = 'alias'  ; 
    ''')

  conn.commit()

  conn.close()

  print('fin tokenization du fichier source')