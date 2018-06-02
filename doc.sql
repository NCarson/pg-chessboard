

SELECT
    string_agg('### ' || name || '(' || args || ') -> ' || type || E'\n\n' || descr, E'\n\n')
FROM
(
SELECT 
  pg_catalog.obj_description(p.oid) as descr,
  p.proname as "name",
  pg_catalog.pg_get_function_result(p.oid) as "type",
  pg_catalog.pg_get_function_arguments(p.oid) as "args",
 CASE
  WHEN p.proisagg THEN 'agg'
  WHEN p.proiswindow THEN 'window'
  WHEN p.prorettype = 'pg_catalog.trigger'::pg_catalog.regtype THEN 'trigger'
  ELSE 'normal'
 END as "Type"
FROM pg_catalog.pg_proc p
     LEFT JOIN pg_catalog.pg_namespace n ON n.oid = p.pronamespace
WHERE pg_catalog.pg_function_is_visible(p.oid)
      AND n.nspname <> 'pg_catalog'
      AND n.nspname <> 'information_schema'
    and pg_catalog.obj_description(p.oid) is not null
ORDER BY 4, 2
) t;
