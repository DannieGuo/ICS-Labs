#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "mm.h"
#include "memlib.h"

team_t team =
{
    "D",
    "D",
    "D",
    "",
	""
};

#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define WSIZE 4
#define DSIZE 8 
#define CHUNKSIZE (1<<12)


#define MAX(x, y)    ((x) > (y)? (x) : (y))
#define PACK(size, alloc)    ((size) | (alloc))
#define GET(p)  (*(unsigned int *)(p))
#define PUT(p, val)  (*(unsigned int *)(p) = (val))
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p)    (GET(p) & 0x1)

#define HDRP(bp)    ((char*)(bp) - WSIZE) //头tag 
#define FTRP(bp)    ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)//尾tag 
#define PRE_BPFREE(bp) ((char*)(bp)) //链里上一个空的块 
#define NEXT_BPFREE(bp) ((char*)(bp) + WSIZE)//链里下一个空的块 
#define NEXT_BLKP(bp)   ((char*)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
//物理位置的下一块 
#define PREV_BLKP(bp)   ((char*)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))
//物理位置的上一块 

#define by1 (1<<3)
#define by2 (1<<4)
#define by3 (1<<5)
#define by4 (1<<6)
#define by5 (1<<7)
#define by6 (1<<8)
#define by7 (1<<9)
#define by8 (1<<10)
#define by9 (1<<11)
#define by10 (1<<12)


void *Extend(size_t dwords);
void *Combine(void *bp);
void *Find(size_t size);
void Place(void *bp, size_t asize);
void Insert(char *bp);
void Move(char *bp);
int Find_Line(size_t size);

int mm_init(void)
{
	void* heap_begin = mem_heap_lo();
    if((heap_begin = mem_sbrk(14 * WSIZE))==(void*)-1) return -1;

    PUT(heap_begin, 0); //0-2^3 //1
    PUT(heap_begin + (1 * WSIZE), 0);//2^4 //2
    PUT(heap_begin + (2 * WSIZE), 0);//2^5  //3
    PUT(heap_begin + (3 * WSIZE), 0);//6  //4
    PUT(heap_begin + (4 * WSIZE), 0);//7   //5
    PUT(heap_begin + (5 * WSIZE), 0);//8   //6
    PUT(heap_begin + (6 * WSIZE), 0);//9   //7
    PUT(heap_begin + (7 * WSIZE), 0);//10  //8
    PUT(heap_begin + (8 * WSIZE), 0);//11  //9
    PUT(heap_begin + (9 * WSIZE), 0);//12  //10
    PUT(heap_begin + (10 * WSIZE), 0);//到无穷  //11
    PUT(heap_begin + (11 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_begin + (12 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_begin + (13 * WSIZE), PACK(0, 1));

    if((Extend(CHUNKSIZE/DSIZE)) == NULL) return -1;

    return 0;

}
void *Extend(size_t dwords)
{
    char *bp;
    size_t size;
    size = (dwords % 2) ? (dwords + 1) * DSIZE : dwords * DSIZE;

    if((long)(bp = mem_sbrk(size)) == (void*)-1)
        return NULL;

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(NEXT_BPFREE(bp), 0);
    PUT(PRE_BPFREE(bp), 0);
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    return Combine(bp);
}
void *Combine(void *bp)
{
    size_t  prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t  next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if(prev_alloc && next_alloc) {}
    else if(prev_alloc && !next_alloc)
    {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        Move(NEXT_BLKP(bp));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if(!prev_alloc && next_alloc)
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        Move(PREV_BLKP(bp));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else
    {
        size += GET_SIZE(FTRP(NEXT_BLKP(bp))) + GET_SIZE(HDRP(PREV_BLKP(bp)));
        Move(PREV_BLKP(bp));
        Move(NEXT_BLKP(bp));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    Insert(bp);
    return bp;
}
void Insert(char *bp)
{
	size_t size = GET_SIZE(HDRP(bp));
	int num = Find_Line(size);
	
	//对应链的next 
    char *next = GET((char*)mem_heap_lo() + ((num - 1) * WSIZE));
    
    if(next != NULL)
        PUT(PRE_BPFREE(next), bp);
        
    PUT(NEXT_BPFREE(bp), next);
    PUT((char*)mem_heap_lo() + ((num - 1) * WSIZE), bp);
}
void Move(char *bp)
{
    char *pre = GET(PRE_BPFREE(bp));
    char *next = GET(NEXT_BPFREE(bp));
    
    if(pre == NULL)
    {
        if(next != NULL)
			PUT(PRE_BPFREE(next), 0);
		
		size_t size = GET_SIZE(HDRP(bp));
		int num = Find_Line(size);
        PUT((char*)mem_heap_lo() + (num - 1) * WSIZE, next);
    }
    else
    {
        if(next != NULL)
			PUT(PRE_BPFREE(next), pre);
        PUT(NEXT_BPFREE(pre), next);
    }
    PUT(NEXT_BPFREE(bp), 0);
    PUT(PRE_BPFREE(bp), 0);
}
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp;
    
    if(size == 0) 
		return NULL;
		
    if(size <= DSIZE)
    {
        asize = 2 * DSIZE;
    }
    else
    {
        asize = DSIZE * ((size + DSIZE + (DSIZE - 1)) / (DSIZE));
    }

    if((bp = Find(asize)) != NULL)
    {
        Place(bp, asize);
        return bp;
    }
    
    extendsize = MAX(asize, CHUNKSIZE);
    if((bp = Extend(extendsize / DSIZE)) == NULL)
    {
        return NULL;
    }
    Place(bp, asize);
    return bp;
}
void *Find(size_t size) 
{
	int num = Find_Line(size);
	
    char *bp = GET((char*)mem_heap_lo() + (num - 1) * WSIZE);
    char *most = NULL;
    size_t min;
    int b = 1;
    while(most == NULL)
    {	
		if(num <= 11)
    		bp = GET((char*)mem_heap_lo() + (num - 1) * WSIZE);
    	else
    		break;
    		
	    while(bp != NULL)
	    {
		    if(b == 1)
	    	{
	    		if(GET_SIZE(HDRP(bp)) >= size)
	    		{
	    			most = bp;
	    			min = GET_SIZE(HDRP(bp));
	    			b = 0;
				}
			}
			else
			{
				if(GET_SIZE(HDRP(bp)) >= size && GET_SIZE(HDRP(bp)) < min)
				{
					most = bp;
					min = GET_SIZE(HDRP(bp));
				}
			}
			bp = GET(NEXT_BPFREE(bp));
	    }
	    num++;
	}
    return most;
}
int Find_Line(size_t size)
{
	int num = 0;
	
	if(size <= by1) num = 1;
	else if(size <= by2) num = 2;
	else if(size <= by3) num = 3;
	else if(size <= by4) num = 4;
	else if(size <= by5) num = 5;
	else if(size <= by6) num = 6;
	else if(size <= by7) num = 7;
	else if(size <= by8) num = 8;
	else if(size <= by9) num = 9;
	else if(size <= by10) num = 10;
	else num = 11;
	
	return num;
}
void Place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));
    Move(bp);
    if((csize - asize) >= (2*DSIZE))
    {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize-asize, 0));
        PUT(FTRP(bp), PACK(csize-asize, 0));
        PUT(NEXT_BPFREE(bp), 0);
        PUT(PRE_BPFREE(bp), 0);
        Combine(bp);
    }
    else
    {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}
void mm_free(void *bp)
{
    if(bp == 0)
        return;

    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(NEXT_BPFREE(bp), 0);
    PUT(PRE_BPFREE(bp), 0);
    Combine(bp);
}
void *mm_realloc(void *ptr, size_t size)
{ 
    size_t oldsize = GET_SIZE(HDRP(ptr));
    void *newptr;

    if(size == 0)
    {
        mm_free(ptr);
        return 0;
    }

    if(ptr == NULL)
    {
        return mm_malloc(size);
    }

    if(size <= DSIZE) size = 2 * DSIZE;
    	else size = DSIZE * ((size + DSIZE + (DSIZE - 1)) / (DSIZE));
    	
    if(oldsize >= size) 
		return ptr;
	else 
	{
		int what = Find_Realloc(ptr, size);
		if(what == 1)//重新分配 
		{
			newptr = mm_malloc(size);
		    if(!newptr) return 0;
		    oldsize = GET_SIZE(HDRP(ptr));
		    memcpy(newptr, ptr, oldsize);
		    mm_free(ptr);
		    return newptr;
		}
		else
		if(what == 2)
		{
			PUT(HDRP(ptr), PACK(GET_SIZE(HDRP(ptr)), 1));
   			PUT(FTRP(ptr), PACK(GET_SIZE(HDRP(ptr)), 1));
			return ptr;
		}
		else
		if(what == 3)
		{
			newptr = PREV_BLKP(ptr);
			memcpy(newptr, ptr, oldsize);
			PUT(HDRP(newptr), PACK(GET_SIZE(HDRP(newptr)), 1));
   			PUT(FTRP(newptr), PACK(GET_SIZE(HDRP(newptr)), 1));
			return newptr;
		}
	}
}
int Find_Realloc(void* ptr, size_t newsize)
{
	size_t  prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(ptr)));
    size_t  next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
    size_t size = GET_SIZE(HDRP(ptr));
    
    if(prev_alloc && next_alloc) {}
    else if(prev_alloc && !next_alloc)
    {
        size += GET_SIZE(HDRP(NEXT_BLKP(ptr)));
        if(size >= newsize)
        {
        	Move(NEXT_BLKP(ptr));
			PUT(HDRP(ptr), PACK(size, 1));
        	PUT(FTRP(ptr), PACK(size, 1));	
        	return 2;
		}
    }
    else if(!prev_alloc && next_alloc)
    {
        size += GET_SIZE(HDRP(PREV_BLKP(ptr)));
        if(size >= newsize)
        {
        	Move(PREV_BLKP(ptr));
        	PUT(HDRP(PREV_BLKP(ptr)), size);
        	PUT(FTRP(PREV_BLKP(ptr)), size);
        	return 3;
		}
    }
    else
    {
        size += GET_SIZE(FTRP(NEXT_BLKP(ptr))) + GET_SIZE(HDRP(PREV_BLKP(ptr)));
        if(size >= newsize)
        {
	        Move(PREV_BLKP(ptr));
	        Move(NEXT_BLKP(ptr));	
	        PUT(FTRP(NEXT_BLKP(ptr)), PACK(size, 1));
        	PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 1));
        	return 3;
		}
    }
	return 1;
}
