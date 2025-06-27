void null_deref() {
    int *p = NULL;
    *p = 1;  // UB: Dereferencing NULL.
}
