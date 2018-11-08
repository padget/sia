-- selection de tous les types commencant par 'type' et finissant par ')'
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
group by tk1.id
;


-- creation vue permettant la selection des triplets (debut, fin, distance) des types
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
	group by tk1.id
;

-- suppression de la vue v_types_boundaries
drop view if exists v_types_boundaries
;



select bd.rowid as typeid, tk.* 
from t_token as tk
join v_types_boundaries as bd 
on (tk.id between bd.begin and bd.end) 
;



-- selection des paramètre de type avec le nom et le type du paramètre
select 
	name.id as nameid, 
	name.value as namevalue, 
	type.id as typeid, 
	type.value as typevalue
from 
	t_token as colon
join t_token as type on colon.id + 1 = type.id
join t_token as name on colon.id - 1 = name.id
join v_types_boundaries as bd on colon.id between bd.begin and bd.end
where 
	colon."type" = (select tp.key from t_token_type as tp where tp.value = 'colon')
;

-- selection du nom de chaque type
select value 
from t_token as tk
join v_types_boundaries as bd on tk.id = bd.begin + 1
;



-- combien de paramètre dans chaque type ?
-- Sont-ils bien construits ?
-- Sont-ils bien séparés ?
select 
	bd.begin, 
	( -- Combien de ':' ?
		select count(*) 
		from t_token as colon
		where colon.type = (select tp.key from t_token_type as tp where tp.value = 'colon')
		and colon.id BETWEEN bd.begin and bd.end
	) as nb_params, 
  (-- combien de ',' ?
		select count(*) 
		from t_token as comma
		where comma.type = (select tp.key from t_token_type as tp where tp.value = 'comma')
		and colon.id BETWEEN bd.begin and bd.end
	) as nb_comma,
	(
		select count(*)
	)

from v_types_boundaries as bd
;


-- construction vue permettant la selection pour chaque token 
-- de son prédécesseur et de son successeur
drop view if exists v_token_pred_succ ;

create view if not exists 
v_token_pred_succ ("pred_id", "pred_type", "tk_id", "tk_type", "succ_id", "succ_type")
as 
select null, null, first.id, first.type, second.id, second.type 
from 
	t_token as first
inner join t_token as second
	on second.id = first.id + 1
where first.id = (select min(id) from t_token) 
union
select left.id, left.type, middle.id, middle.type, right.id, right.type 
from 
	t_token as middle
inner join t_token as left 
	on left.id = middle.id - 1
inner join t_token as right 
	on right.id = middle.id + 1
union
select before_last.id, before_last.type, last.id, last.type, null, null 
from 
	t_token as last
inner join t_token as before_last
	on before_last.id = last.id - 1
where last.id = (select max(id) from t_token) 
;

-- est ce qu'il existe le mot clé type non suivis 
-- par un nom de type et suivi d'une '(' ?
select count(*) 
from v_token_pred_succ as hierarchy
where 
	hierarchy.tk_type = (select tp.key from t_token_type as tp where tp.value = 'type')
and 
	hierarchy.succ_type != (select tp.key from t_token_type as tp where tp.value = 'name')

-- selection l'ensemble des 'type' qui ne sont pas suivis par un nom
select hierarchy.tk_id
from v_token_pred_succ as hierarchy
where 
	hierarchy.tk_type = (select tp.key from t_token_type as tp where tp.value = 'type')
and 
	hierarchy.succ_type != (select tp.key from t_token_type as tp where tp.value = 'name')

select * from t_type;




-- populate t_type only if 'type' is followed by a name and a '('

insert into t_type (name)
select name.value 
from t_token as token 
inner join t_token as name 
	on name.id = token.id + 1
inner join t_token as lbracket 
	on lbracket.id = name.id + 1
where 
	token.type = (select tp.key from t_token_type as tp where tp.value = 'type')
and 
	name.type = (select tp.key from t_token_type as tp where tp.value = 'name')
and
	lbracket.type = (select tp.key from t_token_type as tp where tp.value = 'lbracket')
;

-- populate t_type only if 'type' is followed by a name and a '(' and terminated by ')'

insert into t_type (name)
select name.value 
from t_token as type 
inner join v_types_boundaries as boundaries
	on type.id = boundaries.begin
inner join t_token as name 
	on name.id = type.id + 1
inner join t_token as lbracket 
	on lbracket.id = name.id + 1
inner join t_token as rbracket
	on rbracket.id = boundaries.end
where 
	type.type = (select tp.key from t_token_type as tp where tp.value = 'type')
and 
	name.type = (select tp.key from t_token_type as tp where tp.value = 'name')
and
	lbracket.type = (select tp.key from t_token_type as tp where tp.value = 'lbracket')
and 
	rbracket.type = (select tp.key from t_token_type as tp where tp.value = 'rbracket')
;


-- populate t_type only if 'type' is followed by a name and a '(' and terminated by ')'
-- and if all parameters are well-formed
insert into t_type (name)
select 
	name.value, 
	param.name_value, 
	param.type_value
from t_token as type 
inner join 
	v_types_boundaries as boundaries
	on type.id = boundaries.begin
inner join 
	t_token as name 
	on name.id = type.id + 1
inner join 
	t_token as lbracket 
	on lbracket.id = name.id + 1
inner join 
	t_token as rbracket
	on rbracket.id = boundaries.end
left join (
	select 
		param_name.value as name_value, 
		param_name.id as name_id,
		param_type_name.value as type_value,
		param_type_name.id as type_id,
		(param_name.id - boundaries.begin + 1) as virtualid
	from 
		t_token as param_name
	inner join 
		v_types_boundaries as boundaries
		on param_name.id between (boundaries.begin + 3) and (boundaries.end - 1)
		and param_name.type = (select tp.key from t_token_type as tp where tp.value = 'name')
	inner join 
		t_token as param_colon
		on param_colon.id = param_name.id + 1
		and param_colon.type = (select tp.key from t_token_type as tp where tp.value = 'colon')
	inner join 
		t_token as param_type_name 
		on param_type_name.id = param_colon.id + 1
		and param_type_name.type = (select tp.key from t_token_type as tp where tp.value = 'name')
	left join 
		t_token as param_comma 
		on param_comma.id = param_type_name.id + 1
		and param_comma.type = (select tp.key from t_token_type as tp where tp.value = 'comma')
	where 
		(param_name.id - boundaries.begin + 1) % 4 = 0 
) as param 
	on param.name_id between lbracket.id and rbracket.id 
	and param.name_id != lbracket.id 
	and param.name_id != rbracket.id 
where 
	type.type = (select tp.key from t_token_type as tp where tp.value = 'type')
and name.type = (select tp.key from t_token_type as tp where tp.value = 'name')
and lbracket.type = (select tp.key from t_token_type as tp where tp.value = 'lbracket')
and rbracket.type = (select tp.key from t_token_type as tp where tp.value = 'rbracket')
;


-- selection paramètre valide dans les frontière des types : 

select 
	param_name.value as name_value, 
	param_name.id as name_id,
	param_type_name.value as type_value,
	param_type_name.id as type_id,
	(param_name.id - boundaries.begin + 1) as virtualid
from 
	t_token as param_name
inner join 
	v_types_boundaries as boundaries
	on param_name.id between (boundaries.begin + 3) and (boundaries.end - 1)
	and param_name.type = (select tp.key from t_token_type as tp where tp.value = 'name')
inner join 
	t_token as param_colon
	on param_colon.id = param_name.id + 1
	and param_colon.type = (select tp.key from t_token_type as tp where tp.value = 'colon')
inner join 
	t_token as param_type_name 
	on param_type_name.id = param_colon.id + 1
	and param_type_name.type = (select tp.key from t_token_type as tp where tp.value = 'name')
left join 
	t_token as param_comma 
	on param_comma.id = param_type_name.id + 1
	and param_comma.type = (select tp.key from t_token_type as tp where tp.value = 'comma')
where 
	(param_name.id - boundaries.begin + 1) % 4 = 0 