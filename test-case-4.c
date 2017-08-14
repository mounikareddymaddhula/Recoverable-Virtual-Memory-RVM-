/* test-case-4.c - Begin transactions and commit transactions without about_to_modify() - should not create any redo logs */

#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_STRING1 "hello, world"
#define TEST_STRING2 "bleg!"
#define OFFSET2 1000


int main(int argc, char **argv)
{
     rvm_t rvm;
     char *seg;
     char *segs[1];
     trans_t trans, trans2;
     
     rvm = rvm_init("rvm_segments");
     
     rvm_destroy(rvm, "testseg");
     
     
     segs[0] = (char *) rvm_map(rvm, "testseg", 10000);
     
     seg = segs[0];
     
     /* write some data and commit it */
     trans = rvm_begin_trans(rvm, 1, (void**) segs);
          
     rvm_commit_trans(trans);

     /* start writing some different data, but abort */
     

     /* test that the data was restored
     if(strcmp(seg+OFFSET2, TEST_STRING1)) {
	  printf("ERROR: second hello is incorrect (%s)\n",
		 seg+OFFSET2);
	  exit(2);
     }

     if(strcmp(seg, TEST_STRING1)) {
	  printf("ERROR: first hello is incorrect (%s)\n",
		 seg);
	  exit(2);
     }*/
     

     rvm_unmap(rvm, seg);
     printf("OK\n");
     exit(0);
}

