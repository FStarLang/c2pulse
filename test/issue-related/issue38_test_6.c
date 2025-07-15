struct foo; /* forward declaration */
typedef struct foo *foo_ptr;
struct foo {
        int left;
        foo_ptr next;
};
