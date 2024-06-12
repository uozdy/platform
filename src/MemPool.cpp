#include "MemPool.h"

#define PREMAXNODECOUNT			100
#define MAXPOOLCOUNT			10

namespace PLATFORM {

typedef struct tagMNode{
	struct tagMNode*	freePrev;
	struct tagMNode*	freeNext;
	struct tagMNode*	prev;
	struct tagMNode*	next;
	
	int	 bfree;
	int	 len;
	//char data[0];
}MNode;

typedef struct tagMPool{
	MNode		free;
	
	bool		bExtend;
	int			totallen;
	int			maxlen;
	
	MNode		frist;
}MPool;

static MPool* s_pPool[MAXPOOLCOUNT] = {NULL};

bool MP_Init(int idx, int cursize, int maxsize)
{
	if (idx < 0 || idx >= MAXPOOLCOUNT || cursize > maxsize) 
		return false;

	if (s_pPool[idx]) 
		return true;
	
	s_pPool[idx] = (MPool*)malloc(sizeof(MPool) + sizeof(MNode)*PREMAXNODECOUNT + cursize);
	
	if (cursize > maxsize) 
		s_pPool[idx]->bExtend = true;
	else 
		s_pPool[idx]->bExtend = false;
	
	s_pPool[idx]->totallen = cursize;
	s_pPool[idx]->maxlen = maxsize;
	
	s_pPool[idx]->free.freePrev = NULL;
	s_pPool[idx]->free.freeNext = &s_pPool[idx]->frist;
	
	s_pPool[idx]->frist.prev = NULL;
	s_pPool[idx]->frist.next = NULL;
	s_pPool[idx]->frist.freeNext = NULL;
	s_pPool[idx]->frist.freePrev = &s_pPool[idx]->free;
	s_pPool[idx]->frist.bfree = 1;
	
	s_pPool[idx]->frist.len = sizeof(MNode)*PREMAXNODECOUNT + cursize;
	
	return true;
}

bool MP_DeInit(int idx)
{
	if (idx < 0 || idx >= MAXPOOLCOUNT) return false;
	if (!s_pPool[idx]) return false;
	
	free(s_pPool[idx]);
	s_pPool[idx] = NULL;
	return true;
}

void* MP_malloc(int idx, int len)
{
	if (idx < 0 || idx >= MAXPOOLCOUNT) return NULL;
	
	MNode* pfree = s_pPool[idx]->free.freeNext;
	while (pfree) {
		char* pfree_data = (char*)pfree + sizeof(MNode);
		if (pfree->len > (int)(len + sizeof(MNode))) {
			MNode* pnext = (MNode*)(pfree_data + len);
			pnext->len = pfree->len - (len + sizeof(MNode));
			pnext->bfree = 1;
			pfree->bfree = 0;
			pfree->len = len;
			
			pfree->freePrev->freeNext = pnext;
			pnext->freeNext = pfree->freeNext;
			pnext->freePrev = pfree->freePrev;
			if (pfree->freeNext) pfree->freeNext->freePrev = pnext;
			
			pfree->freeNext = NULL;
			pfree->freePrev = NULL;
			
			
			pnext->next = pfree->next;
			if (pfree->next) pfree->next->prev = pnext;
			pnext->prev = pfree;
			pfree->next = pnext;
		
			return pfree_data;
		}else if (pfree->len >= len && pfree->next){
			pfree->bfree = 0;
			return pfree_data;
		}else{
			pfree = pfree->freeNext;
		}
	}
	
	return NULL;
}

bool MP_free(int idx, void* data)
{
	if (idx < 0 || idx >= MAXPOOLCOUNT || data == NULL) return false;
	
	bool bE = false;
	MNode* pNode = (MNode*)((char*)data - sizeof(MNode));
	if (pNode->bfree) return false;
	
	pNode->bfree = 1;
	if (pNode->prev && pNode->prev->bfree) {
		MNode* pr = pNode->prev;
		pr->len += pNode->len + sizeof(MNode);	
		
		pr->next = pNode->next;
		if (pNode->next) pNode->next->prev = pr;
				
		pNode = pr;
		bE = true;
	}
	
	if (pNode->next && pNode->next->bfree) {
		MNode* pn = pNode->next;
		pNode->len += pn->len + sizeof(MNode);
		pNode->next = pn->next;
		if (pn->next) pn->next->prev = pNode;
		
		if (pn->freeNext) pn->freeNext->freePrev = pn->freePrev;
		if (pn->freePrev) pn->freePrev->freeNext = pn->freeNext;
	}
	
	if (!bE) {
		MNode* pfree = &s_pPool[idx]->free;
		pNode->freeNext = pfree->freeNext;
		pNode->freePrev = pfree;
		if (pfree->freeNext) pfree->freeNext->freePrev = pNode;
		pfree->freeNext = pNode;
	}	
	return true;
}

void PrintNode(int idx)
{
	MNode* pNode = &s_pPool[idx]->frist;
	printf("all:\n");
	while (pNode) {
		printf("node: %p %d %d %p %p %p\n", pNode, pNode->len, pNode->bfree, pNode->prev, pNode->next, pNode->freeNext);
		
		pNode = pNode->next;
	}
	
	pNode = s_pPool[idx]->free.freeNext;
	printf("free:\n");
	int count = 5;
	while (pNode && count -- >= 0) {
		printf("node: %p %d %d %p %p %p\n", pNode, pNode->len, pNode->bfree, pNode->prev, pNode->next, pNode->freeNext);
		
		pNode = pNode->freeNext;
	}
	
	printf("\n\n");
}

}
