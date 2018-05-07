
/* Datum array_to_elems(PG_FUNCTION_ARGS) 
 * from joe conway presenatoin
{
	ArrayType 		*v = PG_GETARG_ARRAYTYPE_P(0);
	int 			nitems, *dims, ndims;
	char 			*p;
	Oid 			element_type;
	int 			typlen;
	bool 			typbyval;
	char 			typalign;
	char 			typdelim;
	Oid 			typelem;
	Oid 			typiofunc
	FmgrInfo 		proc;
	int i;

	p = ARR_DATA_PTR(v);
	ndims = ARR_NDIM(v);
    if (ndims > 1)
        CH_ERROR("only one dimensinal arrays supported");
	dims = ARR_DIMS(v);
	nitems = ArrayGetNItems(ndims, dims);

	element_type = ARR_ELEMTYPE(v);
	get_type_io_data(element_type, IOFunc_output, &typlen, &typbyval, &typalign, &typdelim, &typelem, &typiofunc);
	fmgr_info_cxt(typiofunc, &proc, fcinfo->flinfo->fn_mcxt);

	for (i = 0; i < nitems; i++)
	{
		Datum itemvalue;
		char *value;

		itemvalue = fetch_att(p, typbyval, typlen);
		value = DatumGetCString(FunctionCall3(&proc, itemvalue, ObjectIdGetDatum(typelem), Int32GetDatum(-1)));

		// Do something with value here 
		p = att_addlength(p, typlen, PointerGetDatum(p));
		p = (char *) att_align(p, typalign);
    }
*/

/*
static ArrayType * make_array( char *typname, size_t size, Datum * data)
{
	ArrayType	*result;
	Oid			element_type = TypenameGetTypid(typname);
	if (!OidIsValid(element_type))
		elog(ERROR, "could not find '%s' type.", typname);

	int16		typlen;
	bool		typbyval;
	char		typalign;

	get_typlenbyvalalign(element_type, &typlen, &typbyval, &typalign);

	result = construct_array(data, size, element_type, typlen, typbyval, typalign);
	if (!result)
		elog(ERROR, "constructing array failed");
	return result;
}

*/

/*
#define PG_RETURN_ENUM(typname, label) return enumLabelToOid(typname, label)
static Oid enumLabelToOid(const char *typname, const char *label)
{
Oid enumtypoid;
HeapTuple tup;
Oid ret;

enumtypoid = TypenameGetTypid(typname);
Assert(OidIsValid(enumtypoid));

tup = SearchSysCache2(ENUMTYPOIDNAME,
ObjectIdGetDatum(enumtypoid),
CStringGetDatum(label));
Assert(HeapTupleIsValid(tup));

ret = HeapTupleGetOid(tup);

ReleaseSysCache(tup);

return ret;
}
*/
