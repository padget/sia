update t_token set type=(select key from t_token_type where value = 'fn') where value = 'fn' ;
update t_token set type=(select key from t_token_type where value = 'type') where value = 'type' ;
