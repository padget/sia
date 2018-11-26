drop view if exists stx_types_boundaries;

create view if not exists 
stx_types_boundaries ("begin", "end", "distance") 
as 
	select 
		tk1.id as "begin", 
		tk2.id as "end", 
		min(tk2.id - tk1.id)
	from 
		tkn_token as tk1, 
		tkn_token as tk2
	where 
		tk1."type" = 'type' 
	and tk2."type" = 'rbracket'
	and tk2.id - tk1.id > 0
	group by tk1.id;