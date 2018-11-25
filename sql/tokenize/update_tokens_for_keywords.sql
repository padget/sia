update tkn_token set type=(select key from tkn_token_type where value = 'fn') where value = 'fn' ;
update tkn_token set type=(select key from tkn_token_type where value = 'type') where value = 'type' ;
