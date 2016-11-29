#include "linkedlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

node* CreateNode(elementType el)
{
    node *pNode = (node*)malloc(sizeof(node));
    if(pNode == NULL) return NULL;
    pNode->element = el;
    pNode->pNext = NULL;
    return pNode;
}

int AddTop(node **root, elementType el)
{
    node* pNode = CreateNode(el);
    if(!pNode) return 0;
    if(*root == NULL) {
        *root = pNode;
    } else {
        pNode->pNext = *root;
        *root = pNode;
    }
    return 1;
}

int IsEmpty(node *temp)
{
    if (temp == NULL)
        return 1;
    else
        return 0;
}

elementType RemoveTop(node **root)
{
    elementType data;
    if (IsEmpty(*root)) {
        printf("\nThe stack is empty!");
        return data;
    } else {
        node* pResult = *root;
        elementType data = pResult->element;
        *root = (*root)->pNext;
        free(pResult);
        return data;
    }
}

void RemoveNode(node **root, node *nodeToDel)
{
    if(IsEmpty(*root))
        return;
    node *cur= *root;
    node *prev;
    if(cur->pNext == NULL){
        FreeList(root);
        return;
    }
    while(cur!=NULL && cur!=nodeToDel) {
        prev = cur;
        cur = cur->pNext;
    }

    if(cur==*root) {
        *root = cur->pNext;
        free(cur);
    } else if(cur!=NULL) {
        prev->pNext = cur->pNext;
        free(cur);
    }
}

void FreeList(node **root)
{
    if(IsEmpty(*root) == 0) {
        node *tmp = *root;
        while(tmp!=NULL) {
            *root = (*root)->pNext;
            free(tmp);
            tmp = *root;
        }
    }
}

void swap(node *a, node *b)
{
    elementType tmp = a->element;
    a->element = b->element;
    b->element = tmp;
}

void BubbleSort(node *top)
{
    int swapped, cmp;
    node *tmp1;
    node *tmp2 = NULL;

    if(IsEmpty(top) || top->pNext == NULL) return;

    do {
        swapped = 0;
        tmp1 = top;
        while(tmp1->pNext != tmp2) {
            cmp = strcmp(tmp1->element.name,tmp1->pNext->element.name);
            if(cmp > 0) {
                swap(tmp1,tmp1->pNext);
                swapped = 1;
            }
            tmp1 = tmp1->pNext;
        }
        tmp2 = tmp1;
    } while(swapped);
}

node* ShowFirst(node **root)
{
    if (IsEmpty(*root))
        printf("\nThe stack is empty!");
    else {
        return *root;
    }
    return NULL;
}

int GetListLength(node *root)
{
    int count = 0;
    node *tmp = root;
    if(IsEmpty(root))
        return 0;
    while(tmp!=NULL) {
        count++;
        tmp = tmp->pNext;
    }
    return count;
}

void PrintList(node **root)
{
    node* tmp = *root;
    while(tmp != NULL) {
        printf("%s %s\n", tmp->element.name, tmp->element.pass);
        tmp = tmp->pNext;
    }
}
