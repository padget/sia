drop view if exists v_types_boundaries;

create view if not exists 
v_types_boundaries ("begin", "end", "distance") 
as 
	select 
		tk1.id as "begin", 
		tk2.id as "end", 
		min(tk2.id - tk1.id)
	from 
		t_token as tk1, 
		t_token as tk2
	where 
		tk1."type" = 12 
	and tk2."type" = 4
	and tk2.id - tk1.id > 0
	group by tk1.id;