/* test-case-5.c - Trying to map the same segment twice with different sizes*/

#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_STRING1 "hello, world"
#define TEST_STRING2 "bleg!"
#define OFFSET2 100
#define OFFSET3 1100


int main(int argc, char **argv)
{
     rvm_t rvm;
     char *seg;
     char *segs[1];
     trans_t trans, trans2;
     
     rvm = rvm_init("rvm_segments");
     
     rvm_destroy(rvm, "testseg");
     
     segs[0] = (char *) rvm_map(rvm, "testseg", 1000);
     

     seg = segs[0];
     /* write some data and commit it */
     trans = rvm_begin_trans(rvm, 1, (void**) segs);
     printf("First transaction id: %d \n", trans);
     
     rvm_about_to_modify(trans, seg, 0, 100);
     sprintf(seg, TEST_STRING1);
     
     //rvm_about_to_modify(trans, seg, OFFSET2, 100);
     //sprintf(seg+OFFSET2, TEST_STRING1);
     
     rvm_commit_trans(trans);
     rvm_unmap(rvm, seg);
    
     segs[0] = (char *) rvm_map(rvm, "testseg", 2000);
     seg = segs[0];
     /* start writing some different data, but abort */
     trans = rvm_begin_trans(rvm, 1, (void**) segs);
     //rvm_about_to_modify(trans, seg, 0, 100);
     //sprintf(seg, TEST_STRING2);
     
     rvm_about_to_modify(trans, seg, OFFSET3, 100);
     sprintf(seg+OFFSET3, TEST_STRING2);

     rvm_commit_trans(trans);
     

     rvm_unmap(rvm, seg);
     rvm_truncate_log(rvm);

     printf("OK\n");
     exit(0);
}

