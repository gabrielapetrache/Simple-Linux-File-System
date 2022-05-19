#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"

List *ll_create();
int ll_add_node(List *list, TreeNode *info);
ListNode *ll_remove_node(List *list, TreeNode *info);
void ll_print(List *list);
ListNode *ll_search(List *list, char *node);