
select '## Types';

select
    string_agg('### ' || name || E'\n\n' || coalesce(descr, 'TODO'), E'\n\n')
from
(
	SELECT n.nspname as "Schema",
	  pg_catalog.format_type(t.oid, NULL) AS "name",
	  pg_catalog.obj_description(t.oid, 'pg_type') as "descr"
	FROM pg_catalog.pg_type t
		 LEFT JOIN pg_catalog.pg_namespace n ON n.oid = t.typnamespace
	WHERE (t.typrelid = 0 OR (SELECT c.relkind = 'c' FROM pg_catalog.pg_class c WHERE c.oid = t.typrelid))
	  AND NOT EXISTS(SELECT 1 FROM pg_catalog.pg_type el WHERE el.oid = t.typelem AND el.typarray = t.oid)
		  AND n.nspname <> 'pg_catalog'
		  AND n.nspname <> 'information_schema'
	  AND pg_catalog.pg_type_is_visible(t.oid)
	ORDER BY 1, 2
) t;

select '## Functions';

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
