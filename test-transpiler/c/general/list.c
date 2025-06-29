struct node {
  int head;
  struct node *next;
};

DEFINE(
  /* We would copy the definition of list_live here, I think */
)

REQUIRES(list_live l)
ENSURES(list_live RES)
struct node *push(int value, struct node *l)
{
  struct node *n = malloc(sizeof(struct node));
  if (!n) {
    /* Pulse does not model allocation failures yet. This whole
    branch would disappear, I think. */
    abort();
  }

  n->head = value;
  n->next = l;
  return n;
}

REQUIRES(list_live l)
REQUIRES(l != NULL)
int peek (struct node *l)
{
  return l->head;
}

REQUIRES(list_live l)
REQUIRES(l != NULL)
ENSURES(list_live RES)
struct node *pop(struct node *l)
{
  struct node *n = l->next;
  free(l);
  return n;
}

REQUIRES(list_live l)
ENSURES(list_live l)
bool is_empty(struct node *l)
{
  return l == NULL;
}
