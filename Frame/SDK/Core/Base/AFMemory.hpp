/*
* This source file is part of ArkGameFrame
* For the latest info, see https://github.com/ArkGame
*
* Copyright (c) 2013-2018 ArkGame authors.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#pragma once

#include "AFPlatform.hpp"

//From:
//https://github.com/DavidLiRemini/MemoryPool

template <typename T>
struct ListNode
{
    ListNode<T>* prev;
    ListNode<T>* next;
    T data;
    ListNode() : prev(nullptr), next(nullptr) {}
};

template <typename T>
class ListPool
{
protected:
    typedef ListNode<T> ListNode;
public:
    typedef ListNode* LinkType;
protected:
    LinkType node;
private:
    LinkType Create_Node()
    {
        LinkType tmp = new ListNode();
        return tmp;
    }
public:
    ListPool()
    {
        Empty_initialize();
    }
    void Empty_initialize()
    {
        node = Create_Node();
        node->prev = node;
        node->next = node;
    }
    void Remove(ListNode& link)
    {
        assert(link.prev != nullptr);
        assert(link.next != nullptr);
        link.prev->next = link.next;
        link.next->prev = link.prev;

        link.next = nullptr;
        link.prev = nullptr;
    }
    void AddNode(const T& x)
    {
        LinkType tmp = Create_Node();
        tmp->data = x;
        tmp->next = node;
        tmp->prev = node->prev;
        node->prev->next = tmp;
        node->prev = tmp;
    }

    uint32_t ListSize() const
    {
        uint32_t t = 0;
        LinkType ptr = node->next;
        while (ptr != node)
        {
            ptr = ptr->next;
            ++t;
        }
        return t;
    }
};

struct MemoryBlock
{
    //����ǰ��ָ��
    MemoryBlock*prev;
    MemoryBlock* next;
    //����MemoryBlock�ܴ�С��
    int mSize;
    //δ��������
    int free;
    //�׸�δ���������
    int first;
    //Padding
    char mPad[2];
    MemoryBlock(uint16_t initblock = 1, uint16_t unitSize = 0);
    static void* operator new(size_t, uint16_t, uint16_t);
    static void operator delete(void*);
};

class MemoryPool
{
private:
    static uint16_t poolMapIndex;
    //���䲻ͬ�ڴ��ʱ���Ӧ��ӳ���
    std::map<int, int>poolMap;
    //�ڴ�ض����С��
    const int POOLALIGNMENT = 8;
    //��ʼ���ڴ��
    int initBlockCount;
    //�ڴ�鲻�������Ŀ�����
    int growBlockcount;
    //�����ڴ���±�
    unsigned firstIndex;
    //ĩ�ڴ���±ꡣ
    unsigned lastIndex;
    //���16�ֲ�ͬ�ڴ���С��Ҳ����˵���ڴ�������16����
    MemoryBlock* memoryHashMap[16];
    MemoryBlock** mpPtr;
    //���㲻ͬ�ڴ���Ӧ��hashCode
    int Hash(int);
    //�����ֽ�
    int AlignBytes(int);
    //���ط�����С��
    int GetUnitSize(int);
protected:
    static MemoryPool* memoryPoolInstance;
    MemoryPool(int initBlockSize = 1024, int growBlockSize = 256);
public:
    //�����ڴ�
    void* Alloc(int);
    //�ͷ��ڴ档
    void FreeAlloc(void*);
    static void Initialize(MemoryPool* p);
    //����ȫ���ڴ��ʵ��
    static MemoryPool* GetInstance();
    ~MemoryPool();
};