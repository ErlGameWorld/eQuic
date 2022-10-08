#ifndef __QUIC_LLIST_H_
#define __QUIC_LLIST_H_

#include<erl_nif.h>

typedef struct LLNode_r {
    void* data;
    struct LLNode_r* next;
} LLNode;

bool addLList(LLNode** LPtr, void* Data) {
    if (NULL == (*LPtr)) {
        LLNode* NewNode = (LLNode*)enif_alloc(sizeof(LLNode));
        if (NULL == (NewNode)) return -1;
        NewNode->next = NewNode;
        NewNode->data = Data;
        (*LPtr) = NewNode;
        return true;
    }
    else {
        LLNode* NewNode = (LLNode*)enif_alloc(sizeof(LLNode));
        if (NULL == (NewNode)) return -1;
        NewNode->next = (*LPtr)->next;
        NewNode->data = Data;
        (*LPtr)->next = NewNode;
        return true;
    }
}

void* delLList(LLNode** LPtr, void* Key, bool(*compareFun)(void*, void*)) {
    if (NULL == (*LPtr)) {
        return NULL;
    }
    else if (((*LPtr) == (*LPtr)->next) && compareFun((*LPtr)->data, Key)) {
        void* TempData = (*LPtr)->data;
        enif_free(*LPtr);
        (*LPtr) = NULL;
        return TempData;
    }
    else {
        LLNode* TempNode = *LPtr;
        do {
            if (compareFun(TempNode->next->data, Key)) {
                LLNode* FreeNode = TempNode->next;
                void* TempData = TempNode->next->data;
                if ((*LPtr) == TempNode->next) {
                    (*LPtr) = TempNode->next->next;
                }
                TempNode->next = TempNode->next->next;
                enif_free(FreeNode);
                return TempData;
            }
            TempNode = TempNode->next;
        } while (TempNode != (*LPtr));
        return NULL;
    }
}

int lenLList(LLNode** LPtr) {
    int i = 0;
    if (NULL == (*LPtr)) {
        return i;
    }
    else {
        LLNode* TempNode = (*LPtr);
        do {
            i++;
            TempNode = TempNode->next;
        } while (TempNode != (*LPtr));
    }
    return i;
}

void* nextLList(LLNode** LPtr) {
    if (NULL != (*LPtr)) {
        void* TempData = (*LPtr)->data;
        (*LPtr) = (*LPtr)->next;
        return TempData;
    }
    else {
        return NULL;
    }
}

void* getLList(LLNode** LPtr, void* Key, bool(*compareFun)(void*, void*)) {
    if (NULL == (*LPtr)) {
        return NULL;
    }
    else {
        LLNode* TempNode = (*LPtr);
        do {
            if (compareFun(TempNode->data, Key)) return TempNode->data;
            TempNode = TempNode->next;
        } while (TempNode != (*LPtr));
    }
    return NULL;
}

void clearLList(LLNode** LPtr, void(*dataFreeFun)(void*)) {
    if (NULL == (*LPtr)) {
        return;
    }
    else {
        LLNode* TempNode = (*LPtr);
        LLNode* FreeNode = (*LPtr);
        do {
            FreeNode = TempNode;
            TempNode = TempNode->next;
            dataFreeFun(FreeNode->data);
            enif_free(FreeNode);
        } while (TempNode != (*LPtr));
    }
    (*LPtr) = NULL;
    return;
}

#endif
