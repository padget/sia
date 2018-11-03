create table if not exists t_type_member (
    id number PRIMARY KEY,
    name text not null,
    type integer not null,
    parent integer not null
);