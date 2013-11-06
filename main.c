#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
void *mymalloc(size_t);
void myfree(void *);
void dump_memory_map(void);

int main(int argc, char **argv) {
 
myfree(NULL);
    void *m1 = mymalloc(250000);  // should allocate 64 bytes
    dump_memory_map(); 
	printf("\n");
	void *m2 = mymalloc(250000); // should allocate 128 bytes
    dump_memory_map(); 
	printf("\n");
    void *m3 = mymalloc(250000);
	    dump_memory_map(); 
	printf("\n"); 
 //myfree(m1);
    myfree(m1);
    dump_memory_map(); 
	printf("\n");
     //myfree(m3);
   // dump_memory_map(); 
	//printf("\n");
 myfree(m2);
	////void *m6 = mymalloc (2000);
//	void *m5 = mymalloc(300);
    /*void *m3 = mymalloc(56);  // should allocate 64 bytes
    //void *m7 = mymalloc(500000);  // should allocate 64 bytes
    void *m8 = mymalloc(100000);  // should allocate 64 bytes
   void *m11 = mymalloc(100000);  // should allocate 64 bytes
   // void *m10 = mymalloc(500000);  // should allocate 64 bytes
    void *m9 = mymalloc(250000);  // should allocate 64 bytes
    void *m4 = mymalloc(11);  // should allocate 32 bytes
    myfree(m3);
    void *m5 = mymalloc(30);  // should allocate 64 bytes
    void *m6 = mymalloc(120); // should allocate 128 bytes
    myfree(m2);
    //myfree(m7);
    */
	return 0;
}

