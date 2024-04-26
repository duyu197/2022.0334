To correctly run the program, the format of input file and the command line parameters are strictly restricted as follows:

1. The format of input file is :
 number of total elements (n), number of selecting elements (m)
for each non-zero element in turn:
      i, j, q(i,j) {=q(j,i) as the matrix is symmetric}

Eg.  for the following 5*5 matrix, and select out 3 elements:

0    1    2    0    4
1    0    5    8    6
2    5    0   -5    3
0    8   -5    0    -3
4    6    3   -3    6

its corresponding input file:

5 3
1 2 1
1 3 2
1 5 4
2 3 5
2 4 8
2 5 6
3 4 -5
3 5 3
4 5 -3
5 5 6

Note that the diagonal elements will be ignored, i.e., q(5,5) will be set to 0 in program.

2. The command line includes 5 parameters: 

ExecutiveFileName  InputFileName  OutputFileName  TimeLimit  index_diff  

//The index_diff can be called the start index, for the above example, the index_diff is 1.

3. The format of output file is:
seed, the objective value, time to obtain the best objective value
Best solution:
The index of m elements in best solutions.
