#ifndef OBJPOOL_H__
#define OBJPOOL_H__
#include <queue>
#include <vector>
#include <memory>
#include <stdexcept>
#include <algorithm>
using namespace std;

#define kDefaultChunkSize   10
template <typename T>
class CObjectPool
{
public:
    CObjectPool(unsigned int chunkSize = kDefaultChunkSize);
    ~CObjectPool();
    T* acquireObject();
    void releaseObject(T* obj);

protected:
    static void arrayDeleteObject(T* obj);
    void allocateChunk();

private:
    CObjectPool(const CObjectPool<T>& src);
    CObjectPool<T>& operator=(const CObjectPool<T>& rhs);

private:
    queue<T*> m_FreeList;
    vector<T*> m_AllObjects;
    unsigned int m_ChunkSize;
};

template <typename T>
CObjectPool<T>::CObjectPool(unsigned int chunkSize)
    : m_ChunkSize(chunkSize)
{
    allocateChunk();
}

template <typename T>
void CObjectPool<T>::allocateChunk()
{
    T* newObjects = new T[m_ChunkSize];
    m_AllObjects.push_back(newObjects);
    for (unsigned int i = 0; i < m_ChunkSize; i++) {
        m_FreeList.push(&newObjects[i]);
    }
}

template <typename T>
void CObjectPool<T>::arrayDeleteObject(T* obj)
{
    delete[] obj;
}

template <typename T>
CObjectPool<T>::~CObjectPool()
{
    for_each(m_AllObjects.begin(), m_AllObjects.end(), arrayDeleteObject);
}

template <typename T>
T* CObjectPool<T>::acquireObject()
{
    if (m_FreeList.empty()) {
        allocateChunk();
    }

    T* obj = m_FreeList.front();
    m_FreeList.pop();
    return obj;
}

template <typename T>
void CObjectPool<T>::releaseObject(T* obj)
{
    m_FreeList.push(obj);
}

#endif