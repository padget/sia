-- dropping of ref_valid_token_sequence if exists
drop table if exists 
  ref_valid_token_sequence ;

-- Creation of ref_valid_token_sequence if not exists
create table if not exists 
  ref_valid_token_sequence (
    id integer primary key,
    first_token_type text,
    first_token_type_id integer,
    second_token_type text,
    second_token_type_id integer
) ;
-- populating ref_valid_token_sequence with values
insert into
    ref_valid_token_sequence 
      (first_token_type, second_token_type)
values
    ('name', 'colon'),
    ('colon', 'name'),
    ('type', 'name'),
    ('name', 'lbracket'), 
    ('name', 'rbracket'),
    ('lbracket', 'name'),
    ('lbracket', 'rbracket'),
    ('name', 'comma'), 
    ('comma', 'name');

-- updating ref_valid_token_sequence with corresponding id
-- from t_token_type
update 
  ref_valid_token_sequence
set first_token_type_id = 
      (select key 
       from t_token_type 
       where value = first_token_type), 
   second_token_type_id = 
      (select key 
       from t_token_type 
       where value = second_token_type);

-- deleting all sequence whose type is not known
delete from 
  ref_valid_token_sequence
where 
  first_token_type_id is null 
  or second_token_type_id is null;


-- dropping view view_token_successor if exists
drop view if exists 
  view_token_successor;

-- creating view wich determines what 
-- is the successor of each token
create view if not exists 
  view_token_successor 
  (first_token_id, first_token_type, 
   second_token_id, second_token_type)
as
    select 
      current.id, current.type, 
      next.id, next.type 
    from 
      t_token as current
    inner join t_token as next 
      on next.id = current.id + 1
  union
    select last.id, last.type, null, null
    from 
      t_token as last
    where 
      last.id = (select max(id) from t_token);


-- dropping view view_token_successor_is_valid if exists
drop view if exists
  view_token_successor_is_valid;

-- creating view wich determines if each token successor 
-- is permitted by the table ref_valid_token_sequence
create view if not exists
  view_token_successor_is_valid 
  (first_token_id, first_token_type, second_token_id, second_token_type, is_valid)
as 
  select 
    token_succ.first_token_id, 
    token_succ.first_token_type, 
    token_succ.second_token_id,
    token_succ.second_token_type, 
    (select count(*) 
     from ref_valid_token_sequence as valid_seq 
     where valid_seq.first_token_type_id = token_succ.first_token_type
     and valid_seq.second_token_type_id = token_succ.second_token_type) 
     as is_valid
  from 
    view_token_successor as token_succ; 