//2018202067
#include "cachelab.h"
#include<getopt.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <unistd.h>
typedef struct CacheLine{
	int used_time;//time of not used  
	char valid;
	unsigned long tag;
}CacheLine,*Cache;

int main(int argc, char** argv)
{
	int opt, s, E, b;
	int hit_count=0, miss_count=0, eviction_count=0; 
	FILE *fp=NULL;
	while( (opt = getopt(argc, argv, "s:E:b:t:") ) != -1)
	{
		if(opt=='s') s = atoi(optarg);
		else if(opt=='E') E = atoi(optarg);
		else if(opt=='b') b = atoi(optarg);
		else if(opt=='t') fp = fopen(optarg, "r");
	}
	
	
	int S=1<<s;//set
	int Line=S*E;
	Cache cache=(Cache)malloc(sizeof(CacheLine)*(Line+1));//from 1 to Line
	for(int i=0;i<=Line;i++)
	{
		cache[i].used_time=-1;
		cache[i].valid='0';
		cache[i].tag=0;
	}

	int size;
	unsigned long address, find_tag;
	char oper[3];
	while(fscanf(fp, "%s %lx,%d", oper, &address, &size) != EOF)
	{
		if(oper[0]=='I') //instruction
			continue;
		
		find_tag=(address>>(s+b));
		int set=((address<<(64-s-b))>>(64-s));
		
		int full=0,find=0;
		int most=0,mosti,emptyi;
		
		int times=1;
		if(oper[0]=='M') times=2; //modify do 2 times
		
		for(int j=0;j<times;j++)
		{
			for(int i=set*E+1;i<=set*E+E;i++)
			{
				if(cache[i].valid=='1')
				{
					if(cache[i].tag==find_tag)//find
					{
						hit_count++;
						cache[i].used_time=0; //use
						find=1;
					}
					else
					{
						full++;	//num of valid lines
						cache[i].used_time++;//not use
						if(cache[i].used_time>=most)
						{
							most=cache[i].used_time;
							mosti=i;
						}
					}
				}
				else
				{
					emptyi=i;//available line
				}
			}
			
			if(find==0)//miss
			{
				miss_count++;
				if(full==E)//eviction
				{//replace the one with the largest time of not used
					eviction_count++;
					cache[mosti].used_time=0;
					cache[mosti].tag=find_tag;
				}
				else//fill the available line
				{
					cache[emptyi].used_time=0;
					cache[emptyi].valid='1';
					cache[emptyi].tag=find_tag;
				}
			}
		}	
	}
	fclose(fp);
    printSummary(hit_count, miss_count, eviction_count);
    free(cache);
    return 0;
}
