
# MT- Multi-Robot Motion Planning with Temporal Goals

### **Files :**
**1. query.dat**  : LTL query must be specified in this file.

**2. map1_9_2.dat** : 2D workspace descriptor file.

Naming Convention :
mapx_y_z.dat => This is a descriptor file for 2D workspace x, whose size is y times y and for z number of robots.

Input format :
nr - number of rows
nc - number of columns
nobs - no of obstacles
nobs coordinates - 'nobs' lines  mentioning the co-ordinates of the obstacles
r - number of robots
r coordinates - 'r' lines mentioning the initial locations of 'r' robots
npx - no of coordinates where a certain proposition is true for robot x
np coordinates - 'npx' lines mentioning the coordinates and the propositions true at it this co-ordinate for robot x.
(above two fields are repeated for r robots)
Example file : map1_9_2.dat
<pre>	
9            -----> nr
9            -----> nc
27           -----> nobs
1 0          -----> 'nobs' lines (ex. these is an obstacle at coord (1,2) in the workspace)
2 0
3 0
4 0
5 0
6 0
7 0
8 0
0 8
1 8
2 8
3 8
4 8
5 8
6 8
7 8
2 3
2 5
3 3
3 4
3 5
4 4
5 3
5 4
5 5
6 3
6 5
2             -----> r
0 1           -----> 'r' lines (ex. initial location of robot 1 is (8,7))
8 7	
6             -----> np1 number of coordinates where some proposition is true for robot 1
0 0 1 7 10 15 -----> 'np' (ex. at (0,0), proposition 1,7,10,15 are true for robot 1)
8 8 1 8 10 16
2 4 2 3 9 11
6 4 2 4 9 12
4 3 2 5 9 13
4 5 2 6 9 14
6             -----> np2 number of coordinates where some proposition is true for robot 2
0 0 1 7 18 23 -----> 'np' (ex. at (0,0), propositions 1,7,18,23 are true for robot 2)
8 8 1 8 18 24
2 4 2 3 17 19
6 4 2 4 17 20
4 3 2 5 17 21
4 5 2 6 17 22
</pre>
Multiples workspace descriptor files are present in Maps folder. Copy and paste them in a main folder to use them.

**3. MT_star.cpp** : MT&ast; algorithm for 2D workspace

**4. MTplanner_variables.h** : Helper file for MT_star

**5. ltl2tgba** : LTL to Buchi Automaton converter binary file. 
Install Spot-2.6 tool for LTL2TGBA converter. Copy the ltl2tgba file from the bin folder of spot installation and copy it to the current folder. Check if the tool is working by running following command in the current folder using the command line:
./ltl2tgba \--spin ' [ ] (<>p && <>q) '
This command should give us the Buchi automata transitions for the given query

**6. Basic_Solution.cpp**: Basic Solution algorithm for 2D workspace. This is the Dijkstra's  algorithm based solution to the mentioned problem.

**7. Basic_planner_variable.h** : Helper file for Basic_Solution

----
### Execute Algorithm

**Compile**:  Compile Basic_Solution.cpp and MT_star.cpp using g++ command with flag -std=c++11 to generate the binaries djk and MT_star(on Ubuntu OS) . 
For example -
g++ -std=c++11 Basic_Solution.cpp -o djk
g++ -std=c++11 MT_star.cpp -o MT_star

**Execute**:  Execute the generated binaries using the following commands
./djk map1_9_2.dat
./MT_star map1_9_2.dat

------------
