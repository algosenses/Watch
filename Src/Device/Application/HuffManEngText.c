#include <stddef.h>
#include <stdio.h>
#include "HuffmanEngText.h"

const struct node node111111 = {NULL, NULL, 'm'};
const struct node node111110 = {NULL, NULL, 'w'};
const struct node node11111 = {&node111111, &node111110, 0};
const struct node node111101 = {NULL, NULL, 'f'};
const struct node node11110011 = {NULL, NULL, 'k'};
const struct node node1111001011 = {NULL, NULL, 'j'};
const struct node node1111001010 = {NULL, NULL, 'x'};
const struct node node111100101 = {&node1111001011, &node1111001010, 0};
const struct node node1111001001 = {NULL, NULL, 'q'};
const struct node node1111001000 = {NULL, NULL, 'z'};
const struct node node111100100 = {&node1111001001, &node1111001000, 0};
const struct node node11110010 = {&node111100101, &node111100100, 0};
const struct node node1111001 = {&node11110011, &node11110010, 0};
const struct node node1111000 = {NULL, NULL, 'v'};
const struct node node111100 = {&node1111001, &node1111000, 0};
const struct node node11110 = {&node111101, &node111100, 0};
const struct node node1111 = {&node11111, &node11110, 0};
const struct node node1110 = {NULL, NULL, 't'};
const struct node node111 = {&node1111, &node1110, 0};
const struct node node11011 = {NULL, NULL, 'd'};
const struct node node11010 = {NULL, NULL, 'l'};
const struct node node1101 = {&node11011, &node11010, 0};
const struct node node1100 = {NULL, NULL, 'a'};
const struct node node110 = {&node1101, &node1100, 0};
const struct node node11 = {&node111, &node110, 0};
const struct node node1011 = {NULL, NULL, 'o'};
const struct node node101011 = {NULL, NULL, 'g'};
const struct node node101010 = {NULL, NULL, 'y'};
const struct node node10101 = {&node101011, &node101010, 0};
const struct node node101001 = {NULL, NULL, 'p'};
const struct node node101000 = {NULL, NULL, 'b'};
const struct node node10100 = {&node101001, &node101000, 0};
const struct node node1010 = {&node10101, &node10100, 0};
const struct node node101 = {&node1011, &node1010, 0};
const struct node node1001 = {NULL, NULL, 'i'};
const struct node node1000 = {NULL, NULL, 'n'};
const struct node node100 = {&node1001, &node1000, 0};
const struct node node10 = {&node101, &node100, 0};
const struct node node1 = {&node11, &node10, 0};
const struct node node011 = {NULL, NULL, ' '};
const struct node node010 = {NULL, NULL, 'e'};
const struct node node01 = {&node011, &node010, 0};
const struct node node0011 = {NULL, NULL, 's'};
const struct node node0010 = {NULL, NULL, 'h'};
const struct node node001 = {&node0011, &node0010, 0};
const struct node node0001 = {NULL, NULL, 'r'};
const struct node node00001 = {NULL, NULL, 'c'};
const struct node node00000 = {NULL, NULL, 'u'};
const struct node node0000 = {&node00001, &node00000, 0};
const struct node node000 = {&node0001, &node0000, 0};
const struct node node00 = {&node001, &node000, 0};
const struct node node0 = {&node01, &node00, 0};
const struct node root = { &node1, &node0 };

/* decoded s to
 * target should be as big as (27*2)+1
 */

void decode(char *s, int count, char *target)
{
  char i;
  char index = 0;
  char val;
  char charcount = 0;
  const struct node *n = &root;
  while(index < count)
  {
    val = s[index++];
    for(i = 0 ;i < 8; ++i)
    {
        if(!(0x80 & (val << i)))
        {
	  n = n->right;
	} 
	else
	{
	  n = n->left;
	}
	if(n->c)
	{
	  printf("%c",n->c);
	  n = &root;
	  target[++charcount] = n->c;
	  if(charcount >= count ) break;
	}
    }
    if(charcount >= count ) break;
  } 
  target[charcount] = 0;
}

