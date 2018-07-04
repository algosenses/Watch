#ifndef HUFF
#define HUFF
struct node 
{
  /*__data16*/ struct node const * left;
  /*__data16*/ struct node const *  right;
  char c;
};
extern const struct node root;
void decode(char *s, int count, char *target);
#endif
