create table if not exists t_token (
    id integer primary key,
    filename text not null,
    line integer not null,
    column integer not null,
    value text not null,
    type integer not null
);