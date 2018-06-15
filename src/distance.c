

#include "chess_index.h"

PG_FUNCTION_INFO_V1(hamming_int64);
PG_FUNCTION_INFO_V1(hamming_arr_byvalue);
PG_FUNCTION_INFO_V1(jaccard_uint64);
PG_FUNCTION_INFO_V1(jaccard_arr_byvalue);

/******************************************************************************
*   hamming distance
******************************************************************************/
  /*{{{*/
//see -- https://github.com/fake-name/pg-spgist_hamming
int
hamming_uint64(uint64 a, uint64 b)
{
    /*
    Compute number of bits that are not common between `a` and `b`.
    return value is a plain integer
    */
    uint64          x = (a ^ b);
    int             ret=0;

#ifdef  __GNUC__ //if gcc you get to go fast
    ret = __builtin_popcountll (x); 
#else // else you go slow....
    //_mm_popcnt_u64 is for windows V.S.
    for (; x > 0; x >>= 1) {
        ret += x & 1;
    }
#endif
    return ret;
}


Datum
hamming_int64(PG_FUNCTION_ARGS)
{
    const int64         a = PG_GETARG_INT64(0);
    const int64         b = PG_GETARG_INT64(1);
    
    PG_RETURN_INT32(hamming_uint64(a, b));
}

static int64
_hamming_arr_byvalue(PG_FUNCTION_ARGS, const Datum * a, bool * nulls_a, const Datum * b, bool * nulls_b, const size_t len)
{
    int             result=0;

   for (int i=0; i<len; i++){
       if (nulls_a[i] || nulls_b[i] || a[i] != b[i]) 
           result++;
   }
   return result;
}

static int
_get_array_arg(PG_FUNCTION_ARGS, const size_t idx, Datum ** valsContent, bool ** valsNullFlags)
{
    //https://github.com/pjungwir/aggs_for_arrays/blob/master/array_to_max.c
	ArrayType 			*vals;            // Our arguments:
	Oid 				valsType;           // The array element type:
	int16 				valsTypeWidth;      // The array element type widths for our input array:
	bool 				valsTypeByValue;    // The array element type "is passed by value" flags (not really used):
	char 				valsTypeAlignmentCode; // The array element type alignment codes (not really used):
	int32               valsLength;         // The size of the input array:

	if (PG_ARGISNULL(idx)) { ereport(ERROR, (errmsg("Null arrays not accepted"))); } 
	vals = PG_GETARG_ARRAYTYPE_P(idx);
	if (ARR_NDIM(vals) == 0) { ereport(ERROR, (errmsg("array has zero dimensions"))); }
	if (ARR_NDIM(vals) > 1) { ereport(ERROR, (errmsg("One-dimesional arrays are required"))); }

	valsType = ARR_ELEMTYPE(vals);
	valsLength = (ARR_DIMS(vals))[0];
	get_typlenbyvalalign(valsType, &valsTypeWidth, &valsTypeByValue, &valsTypeAlignmentCode);
	deconstruct_array(vals, valsType, valsTypeWidth, valsTypeByValue, valsTypeAlignmentCode, valsContent, valsNullFlags, &valsLength);
    return valsLength;
}

Datum
hamming_arr_byvalue(PG_FUNCTION_ARGS)
{
	Datum 				*a=0, *b=0;
	bool 				*nulls_a=0, *nulls_b=0;
    int                 len_a, len_b;

    len_a = _get_array_arg(fcinfo, 0, &a, &nulls_a);
    len_b = _get_array_arg(fcinfo, 1, &b, &nulls_b);

    if (len_a != len_b)
        CH_ERROR("array lengths must be the same");

    PG_RETURN_INT64(_hamming_arr_byvalue(fcinfo, a, nulls_a, b, nulls_b, len_a));
}


/*}}}*/
/******************************************************************************
*   jaccard index
******************************************************************************/

//https://en.wikipedia.org/wiki/Jaccard_index#Similarity_of_asymmetric_binary_attributes
static double
_jaccard_uint64(const uint64 a, const uint64 b)
{

#ifdef  __GNUC__ //if gcc you get to go fast
    return __builtin_popcountll(a&b) /(double) __builtin_popcountll(a|b);
#else // else you go slow....
    uint64         in = a&b;
    uint64         un = a|b;
    int         n=0, d=0;

    //_mm_popcnt_u64 is for windows V.S.
    CH_NOTICE("n:%d d:%d in:%ld un:%ld", n,d,in,un);
    while (in) { n += in & 1; in >>= 1;}
    while (un) { d += un & 1; un >>= 1;}
    CH_NOTICE("n:%d d:%d in:%ld un:%ld", n,d,in,un);
    return n / (double)d;
#endif
}

Datum
jaccard_uint64(PG_FUNCTION_ARGS)
{
    const uint64         a = PG_GETARG_INT64(0);
    const uint64         b = PG_GETARG_INT64(1);
    CH_NOTICE("a:%ld b:%ld", a, b);
    
    PG_RETURN_FLOAT4(_jaccard_uint64(a, b));
}

//https://en.wikipedia.org/wiki/Jaccard_index#Generalized_Jaccard_similarity_and_distance
static double
_jaccard_arr_byvalue(PG_FUNCTION_ARGS, const Datum * a, bool * nulls_a, const Datum * b, bool * nulls_b, const size_t len)
{
    int64           min=0, max=0;

   for (int i=0; i<len; i++){
       if (nulls_a[i] || nulls_b[i])
           continue;
       if (a[i] > b[i]) {
           max += a[i];
           min += b[i];
        }
       else if (b[i] > a[i]) {
           max += b[i];
           min += a[i];
        } else {
           max += b[i];
           min += a[i];
        }
   }
   return (double)min / max;
}

Datum
jaccard_arr_byvalue(PG_FUNCTION_ARGS)
{
	Datum 				*a=0, *b=0;
	bool 				*nulls_a=0, *nulls_b=0;
    int                 len_a, len_b;

    len_a = _get_array_arg(fcinfo, 0, &a, &nulls_a);
    len_b = _get_array_arg(fcinfo, 1, &b, &nulls_b);

    if (len_a != len_b)
        CH_ERROR("array lengths must be the same");

    PG_RETURN_FLOAT8(_jaccard_arr_byvalue(fcinfo, a, nulls_a, b, nulls_b, len_a));
}
