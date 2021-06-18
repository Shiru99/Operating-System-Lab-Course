#  A Simple Filesystem


### To test all test cases

```
    $ /autograder.sh testcases/ expected_output/
```


### To test individual Test Case 

```
    $ gcc testcase.c simplefs-ops.c simplefs-disk.c 
    $ ./a.out
    $ diff -w -B a.out expected.out
```