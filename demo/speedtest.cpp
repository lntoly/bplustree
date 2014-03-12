#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "bplustree.h"

#define BPTREE_ORDER 4
#define BPTREE_KEY_LENGTH 8
#define TOTAL_RECORD_NUM 500000

int main(int argc, char ** argv)
{
	BPlusTree bptree(BPTREE_ORDER, BPTREE_KEY_LENGTH);

	struct timeval start;
	struct timeval end;

	printf("Total record number: %d\n", TOTAL_RECORD_NUM);
	int i = 0;
	gettimeofday(&start, NULL);
	for(i = 1; i <= TOTAL_RECORD_NUM; i++)
	{
		bptree.Insert(i, i);
	}
	gettimeofday(&end, NULL);
	long costtime = 1000000*(end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
	printf("Insert time: %ldus\n", costtime);

	gettimeofday(&start, NULL);
	for(i = 1; i <= TOTAL_RECORD_NUM; i++)
	{
		//int key = i+1;
		int key = rand()%TOTAL_RECORD_NUM;
		record * rcd = bptree.Find(key, false);
		if(rcd != NULL)
		{
			//printf("key: %d, value: %d\n", key, rcd->value);
		}
		else
		{
			printf("key: %d not found\n", key);
		}
	}
	gettimeofday(&end, NULL);
	costtime = 1000000*(end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
	printf("Select time: %ldus\n", costtime);

    gettimeofday(&start, NULL);
    for(i = 1; i <= TOTAL_RECORD_NUM; i++)
    {
        int key = rand()%TOTAL_RECORD_NUM;
        bptree.Delete(key);
    }
    gettimeofday(&end, NULL);
	costtime = 1000000*(end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    printf("Delete time: %ldus\n", costtime);

    bptree.PrintBPTreeLeaves();

	return 0;
}


