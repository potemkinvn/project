#include "elementType.h"

typedef struct node {
    elementType element;
    struct node *pNext;
} node;

node* CreateNode(elementType el);

int AddTop(node **root, elementType el);

int IsEmpty(node *temp);

elementType RemoveTop(node **root);

void RemoveNode(node **root, node *nodeToDel);

void FreeList(node **root);

void BubbleSort(node *top);

node* ShowFirst(node **root);

int GetListLength(node *root);

void PrintList(node **root);
