B+ Tree!
========

- To compile use `make`
- To clean use `make clean` (Cleans everything including data in the tree)
- To run use `./btree.out < sampleQueries.txt`
- Basically, the program initially builds the tree of points given in "assgn2\_bplus\_data.txt" and then reads queries from stdin
- This tree supports the presence of a key multiple times

###Parameters:

- In file bplustree.config
- First line contains the maximum number of keys in each B+ Tree node (Should be atleast 3)
- Second line contains an integer 0/1 signifying whether there is a need to rebuild the tree
- If it is 0, then the tree is not rebuild and all the data and previous queries persist
- If it is 1, the index is rebuilt and all the points in "assgn2\_bplus\_data.txt" are re-inserted. (This takes about 3 minutes)
- Note : If you change the maximum number of keys, then the tree is rebuilt irrespective of the option in the second line. 

##Input format (from stdin):
- Insert: `0 key data`
- Point Query: `1 key`
- Range Query: `2 center radius`
- End the input by EOF (Ctrl + D)

##Output (on stdout):
- For insert query, success is displayed it it is successful (Should be successful in absence of bugs :p)
- For point query, all the keys equal to that key are printed. (and their data)
- For a range query, all the keys in the range and their data is printed
- The time and disk access statistics are displayed. (Only for the queries, not for the initial build)
