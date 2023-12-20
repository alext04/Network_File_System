#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MOD 20011
#define MOD2 1000000007
typedef long long ll;

typedef struct HM_Node
{
    int hash;
    int idx;
    struct HM_Node *next;
} HM_Node;

typedef struct
{
    HM_Node *arr[MOD];
} HashTable;

int hash1(char *str, int n);
int hash2(char *str, int n);
HashTable *CreateHashTable();
void DeleteNode(HM_Node *node);
void DeleteHashTable(HashTable *ht);
void InsertNode(HashTable *ht, char *str, int idx);
void InsertNodeDirect(HashTable *ht, int hash, int idx);
void RemoveNode(HashTable* ht, int hash);

#ifdef HASHMAP_IMPL
    typedef long long ll;

    const int primes[] = {
        1487219, 2141299, 396427, 4889407, 12721,
        4962143, 3146459, 3754811, 2117441, 6291247,
        9768881, 2797453, 4124611, 4633217, 3293651,
        964793, 9922907, 5353529, 5437043, 6812483,
        5586023, 4758659, 2244761, 2899373, 1130803,
        4232279};

    int add(int a, int b)
    {
        return (int)(((ll)a + (ll)b) % MOD);
    }

    int mul(int a, int b)
    {
        return (int)(((ll)a * (ll)b) % MOD);
    }
    int mul2(int a, int b)
    {
        return (int)(((ll)a * (ll)b) % MOD2);
    }

    int hash1(char *str, int n)
    {
        int hash = 1;
        for (int i = 0; i < n; i++)
        {
            hash = mul(hash, primes[str[i] - 'a']);
        }
        return hash;
    }

    int hash2(char *str, int n)
    {
        int hash = 1;
        for (int i = 0; i < n; i++)
        {
            hash = mul2(hash, primes[str[i] - 'a']);
        }
        return hash;
    }

    HashTable *CreateHashTable()
    {
        HashTable *ht = (HashTable*) malloc(sizeof(HashTable));
        for (int i = 0; i < MOD; i++)
        {
            ht->arr[i] = NULL;
        }
        return ht;
    }
    void DeleteNode(HM_Node *node)
    {
        if (node->next)
        {
            DeleteNode(node->next);
        }
        free(node);
    }
    void DeleteHashTable(HashTable *ht)
    {
        for (int i = 0; i < MOD; i++)
        {
            if (ht->arr[i])
            {
                DeleteNode(ht->arr[i]);
            }
        }

        free(ht);
    }

    void InsertNode(HashTable *ht, char *str, int idx)
    {
        int len = strlen(str);
        int hash = hash1(str, len);

        HM_Node *node = (HM_Node*)malloc(sizeof(HM_Node));
        node->hash = hash2(str, len);
        node->idx = idx;

        node->next = ht->arr[hash % MOD];
        ht->arr[hash % MOD] = node;
    }
    
    void InsertNodeDirect(HashTable *ht, int hash, int idx)
    {

        HM_Node *node = (HM_Node*)malloc(sizeof(HM_Node));
        node->hash = hash;
        node->idx = idx;

        node->next = ht->arr[hash % MOD];
        ht->arr[hash % MOD] = node;
    }
    void RemoveNode(HashTable* ht, int hash){
        HM_Node* node = ht->arr[hash%MOD];
        if (node == NULL) return;
        if (node->hash == hash){
            ht->arr[hash%MOD] = node->next;
            DeleteNode(node);
            return;
        }
        while(node->next != NULL){
            if (node->next->hash == hash){
                HM_Node* temp_node = node->next;
                node->next = temp_node->next;
                DeleteNode(temp_node);
                return;
                
            }
        } 
    }
#endif