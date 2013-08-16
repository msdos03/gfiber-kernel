ezxml.c
1.	Original file line 25, removed “#include <stdlib.h>”, it is for user space
2.	Original file line 32, removed “#ifndef EZXML_NOMMAP” block, and add linux kernel header files
3.	Original file line 38, added function ezxml_realloc/malloc/realloc/free/stat definitions
4.	Original file line 59, added function strdup
5.	Original file line 604, remove function ezxml_parse_fp
6.	Original file line 626, changed ezxml_parse_fd implementation to kernel style
7.	Original file line 656, changed ezxml_parse_file implementation to kernel style
8.	Original file line 814, removed “#ifndef EZXML_NOMMAP” block, not used code 
9.	Original file line 999, removed “#ifdef EZXML_TEST” block, not used code 
10.	Original file line 999, added function ezxml_realloc implementation
 
ezxml.h
1.	Original file line 28, removed user space header files
2.	Original file line 42, added FILE definition
3.	Original file line 62, added function ezxml_t ezxml_parse_file( char *file) declaration
4.	Original file line 62, added “#ifdef EZXML_TEST” 
5.	Original file line 68, removed function ezxml_t ezxml_parse_file( char *file) declaration
6.	Original file line 122, 130, 137, 145, changed strdup to MMP_OS_STRDUP
