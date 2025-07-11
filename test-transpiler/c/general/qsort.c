#include <stddef.h>
#include "../../../test/include/PulseMacros.h"

REQUIRES("a |-> 'va")
REQUIRES("b |-> 'vb")
ENSURES("b |-> 'va")
ENSURES("a |-> 'vb")
static void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

REQUIRES("arr |-> 'varr")
REQUIRES("pure (Seq.length 'varr == SizeT.v len)")
REQUIRES("pure (SizeT.v len > 1)")
RETURNS("pivot_idx: SizeT.t")
ENSURES("pure (SizeT.v pivot_idx < Seq.length 'varr)")
ENSURES("exists* varr'. (arr |-> varr') ** "
    "pure (Seq.length 'varr == Seq.length varr' /\ SizeT.v pivot_idx < Seq.length varr')")
static size_t qsort_partition(ISARRAY(len) int *arr, size_t len) {
    int pivot = arr[0];
    size_t i = 1, j = len;
    while (i < j)
        INVARIANTS("invariant b. exists* varr vi vj."
        "(arr |-> varr) ** (i |-> vi) ** (j |-> vj) **"
        "pure ("
            "b == SizeT.lt vi vj /\\"
            "Seq.length varr == SizeT.v len /\\"
            "0 < SizeT.v vi /\\ SizeT.v vi <= SizeT.v vj /\\ SizeT.v vj <= SizeT.v len"
        ")")
    {
        if (arr[i] <= pivot) {
            i++;
        } else if (arr[j-1] >= pivot) {
            j--;
        } else {
            j--;
            LEMMA(to_mask arr);
            swap(&arr[i], &arr[j]);
            LEMMA(return_array_at arr _; return_array_at arr _; from_mask arr);
            i++;
        }
    }
    if (i > 1) {
        i--;
        LEMMA(to_mask arr);
        swap(&arr[0], &arr[i]);
        LEMMA(return_array_at arr _; return_array_at arr _; from_mask arr);
        return i;
    } else {
        return 0;
    }
}

REQUIRES("arr |-> 'varr")
REQUIRES("pure (Seq.length 'varr == SizeT.v len)")
ENSURES("exists* varr'. (arr |-> varr') ** pure (Seq.length 'varr == Seq.length varr')")
void qsort(ISARRAY(len) int *arr, size_t len) {
    if (len > 1) {
        size_t pivot_idx = qsort_partition(arr, len);
        LEMMA(to_mask arr);
        qsort(arr, pivot_idx);
        qsort(arr + pivot_idx + 1, len - (pivot_idx + 1));
        LEMMA(from_mask arr);
    }
}