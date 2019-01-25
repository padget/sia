import sqlite3

def dao_detect_type_boundaries(sqlcursor):
  query = '''
    insert into stx_types_boundaries (begin, end)
    select begin, end from (                  
      select                                  
    		tk1.id as \"begin\",                    
    		tk2.id as \"end\",                    
     		min(tk2.id - tk1.id)                  
     	from                                    
     		tkn_token as tk1,                      
     		tkn_token as tk2                      
     	where                                   
     		tk1.\"type\" = 'type'                 
     	and tk2.\"type\" = 'rbracket'           
     	and tk2.id - tk1.id > 0                 
     	group by tk1.id);                       
  '''

  sqlcursor.execute(query)

  
if __name__ == '__main__':
  conn   = sqlite3.connect('lol2.sia.db')
  cursor = conn.cursor()
  dao_detect_type_boundaries(cursor)
  conn.commit()
  conn.close()