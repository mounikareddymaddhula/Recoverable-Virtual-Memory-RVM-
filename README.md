1. Build the rvm library and test cases : 
   To build just the rvm library, use "make rvm".
   To build the rvm library and all the test cases, use "make all" or "make".
   Individual test cases can be built by the command "make test_case", where test_case is the name of the testcase.
2. Clean the directory using the command "make clean".
3. To run a testcase, use the command "./test_case", where test_case is the name of the testcase.
   The testcases present are basic, abort, multi, multi-abort, truncate
   test-case-1 : test that looping two rvm_begin_trans fails
   test-case-2.c - Trying to map the same segment twice
   test-case-3.c - Calling about_to-modify() before rvm_begin_trans()
   test-case-4.c - Begin transactions and commit transactions without about_to_modify() - should not create any redo logs
   test-case-5.c - Trying to map the same segment twice with different sizes
4. By default, verbose logs are enabled. This can be toggled by calling the function rvm_verbose(int enable_flag).
5. We are not deleting the rvm directory or its contents when we do "make clean". 
