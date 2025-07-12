typedef struct foo {
        int left;
} *foo_ptr;

void test(foo_ptr p)
{
        p->left = 123;
}
