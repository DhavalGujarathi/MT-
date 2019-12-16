#include<bits/stdc++.h>
using namespace std;
namespace planner_info
{
int sz=10,qtot=3;
vector<vector<long long int>> min_suffix_cycles;
vector<vector<vector<long long int>>> pseudo_suffix_cycle; 
int nrow,ncol;
int dir=5,total_system_states=0,comp_red=0,tot_comp=0,grid_sz;
int nb[][2]={{0,0},{0,-1},{-1,0},{1,0},{0,1}};
unordered_map<long long int,int> grid;
vector< vector< vector< vector<int> > > > trans(qtot,vector< vector< vector<int> > >(qtot) );
vector< vector< vector< vector<int> > > > negtrans(qtot,vector< vector< vector<int> > >(qtot) );
vector<int> neg_trans_to_neighbour;

unordered_set<int> dest;
int dim = 2;
int No_Robots = 1; // default value
int max_no_robots = 11;
vector<vector<vector<int> >> rob_pos_system_state;   // just stores states where some prop is true
vector<unordered_map<int,vector<int> >> rob_prop; // string(coord) with prop true at it
vector<vector<vector< vector<int> > >> rob_prop_sys_states; //## remember to update this as per TS space
vector<set<int>> rob_literal;
vector<int> INI_LOC;  // y * sz + x

int redc_node_multiplier = 1000;
vector<vector<int>> Neighbours; // list of neighbours
vector<vector<vector<int>>> Permutations;
int max_propositions_in_formulae = 8;
vector<vector<vector<int>>> n_c_r;  // n: No of robots    r: propositions that must me satisfied by the robots
unordered_map<string , vector<vector<int>> > prop_state_count_list;

long long int cur_joint_size=0;
unordered_map<long long int,int> N_Joint_N; // node secret no with valid index 0: valid , -1: Invalid
vector<vector<vector<int> > > N_Joint_mapping; // Node_secret_no with coords
unordered_map< long long int, unordered_map <long long int , long long int> > N_Joint_E; // E[From_node][To_node] = weight 
unordered_map< long long int, unordered_map <long long int , bool> > N_Updated; // E[From_node][To_node] = True/False

unordered_map<string,long long int> N_Reverse_Joint_N;
unordered_map<long long int, int> Redc_N; // node values
unordered_map<long long int, unordered_map <long long int , long long int> > Redc_E; // E[From_node][To_node] = weight
unordered_map<long long int, unordered_map <long long int ,  vector<vector<vector<int>>>> > Redc_E_Trans; // E[From_node][To_node] = weight
//[node][node][Transition index][Robot wise][Propositions that are expected to be satisfied]
unordered_set<long long int> Final_states;
long long int Total_cycles = 0;

int mpl = 8; // Maximum number of positive literals in a transition
vector<vector<vector<vector<vector<int>>>>> tasks(mpl , vector<vector<vector<vector<int>>>> (mpl) ); 
// [n:#positive literals][k:tasks][list of possible ways ] [][]
vector<vector<vector<vector<int>>>> master_table(mpl) ;
// [# Positive Literals] list of (vector<vector<int>>)
int max_no_of_prop_def = 100;  //largest no of prop defined

vector<unordered_map< vector<bool> , vector<int> >>  prop_bit_map ; //  list per robot of  [bitmap] -> coord(y * sz + x)
long long int min_suffix_cost = 5000000;

// single robot transitions
struct node
{
    node* par;
    int coord;  // y*sz+x
    int d;      // actual dist
    int h;      // heuristic value
    int f;      // d+h
};


node* newnode(int coord)
{
    node* nd = new node;
    nd->coord = coord;
    nd->d = 0;
    nd->h = 0;
    nd->f = 0;
    nd->par = NULL;
    return nd;
}

struct comp
{
    bool operator()(node *a , node *b)
    {
        return (a->f > b->f);
    }
};

struct VectorHasher {
    int operator()(const vector<long long int> &V) const {
        int hash=0;
        for(int i=0;i<V.size();i++) {
            hash^=V[i]; // Can be anything
        }
        return hash;
    }
};
vector<unordered_map<vector<long long int>, long long int, VectorHasher > > memo;

bool single_robot_valid(int x, int y)
{
    if(x<0 || x>=nrow || y<0 || y>=ncol || grid[y*sz+x]==-1)
        return false;
    return true;   
}

bool Are_neighbour(long long int node, vector<vector<int>> &state)
{

    for(int i=0; i<No_Robots; i++)
    {
        if(state[i][0]==-1)
            continue;
        if(N_Joint_mapping[node][i][0]==-1)
            continue;
        if(  abs(state[i][0] - N_Joint_mapping[node][i][0] ) + abs(state[i][1] - N_Joint_mapping[node][i][1] ) > 1   )
            return false;
    }

    return true;
}

void prop_power_set_mapping( vector<int> &prop_set, vector<bool> P, int index, unordered_map<vector<bool>,vector<int>> &sol, int coord )
{
    if( index == prop_set.size() )
    {
        vector<bool> z (max_no_of_prop_def, false);
        if( P !=  z)
            sol[P].push_back(coord);
        return;
    }
    prop_power_set_mapping( prop_set , P , index+1, sol, coord);
    P[ prop_set[index] ] = true;
    prop_power_set_mapping( prop_set , P , index+1, sol, coord);
}

void generate_prop_power_set_mapping()
{
    for( int i=0; i<No_Robots ; i++ )
    {
        unordered_map< vector<bool>,vector<int>> sol;
        for(unordered_map<int,vector<int>>::iterator itr = rob_prop[i].begin(); itr!=rob_prop[i].end() ; itr++ )
        {
            int coord = itr->first;
            vector<int> prop_set = itr->second;
            vector<bool> P(max_no_of_prop_def, false);
            prop_power_set_mapping( prop_set, P , 0 ,sol , coord);
        }
        prop_bit_map.push_back(sol);
    }

    // cout << "Printing  prop_bit_map ---------------------------------------------------------------" <<endl;    
    // for( int i = 0; i < No_Robots; i++)
    // {
    //     for( unordered_map< vector<bool> , vector<int> >::iterator itr = prop_bit_map[i].begin(); itr != prop_bit_map[i].end(); itr++ )
    //     {
    //         cout << "Robot: " << i+1 << " Bitmap: ";
    //         for(int j=0; j<max_no_of_prop_def;j++)
    //             if(itr->first[j]!=false)
    //                 cout << j << " ";
    //         cout << " ---------- " <<endl;
    //         for(vector<int>::iterator j= itr->second.begin(); j != itr->second.end(); j++)
    //             cout << *j%sz  << "," << *j/sz << endl;
    //         cout << "====================" << endl;
    //     }
    // }
}

void generate_master_table()
{
    int r = No_Robots;
    for(int pe=1; pe<=mpl ; pe++ )
    {
        for(int t=1; t<=tasks[pe-1].size(); t++)
        {
            if(t>r)
                continue;
            for(int j=0; j<tasks[pe-1][t-1].size() ; j++)
            {
                for(int n=0; n < n_c_r[t-1].size();n++)
                {
                    vector<int> locs = n_c_r[t-1][n];
                    // cout << "I am here 1" << endl;
                    for(int p=0; p<Permutations[t-1].size() ; p++)
                    {
                        vector<vector<int>> g_state(No_Robots,{-1});
                        
                        for(int l=0; l<locs.size();l++)
                        {
                            // cout << "I am here 2 " <<  locs[l] << " "  << Permutations[t-1][p][l] << endl;
                            g_state[ locs[l] ] = tasks[pe-1][t-1][j][  Permutations[t-1][p][l] ];
                            // cout << "I am here 3" << endl;
                        }
                        // cout << "I am here 4" << endl;
                        master_table[pe-1].push_back(g_state);
                        // cout << "I am here 5" << endl;

                    }
                }
            }

        }
    }

    //################## Printing the master table ###########################

    // cout << "Printing Master table "  <<  master_table.size() << endl;
    // for(int i=0; i<master_table.size(); i++)
    // {
    //     cout << "Robots: "  << No_Robots << " | # Positive iterals are  " << " " << i+1 << endl;
    //     for(int j=0; j<master_table[i].size(); j++)
    //     {
    //         for(int k=0; k<master_table[i][j].size() ; k++ )
    //         {
    //             cout << "{";
    //             for(int l=0; l<master_table[i][j][k].size(); l++)
    //                 cout << master_table[i][j][k][l] << " " ;
    //             cout << "} ";
    //         }
    //         cout << endl;
    //     }
    //     cout << "---------------------------------------------------------------------------------------------------------------------------" << endl;

    // }

}

void task_aasign(int n , int k)
{
    if(n-1 >= 0)
        for(int i=0; i<tasks[n-1][k].size();i++)
        {
            for(int j=0; j<=k ; j++)
            {
                tasks[n-1][k][i][j].push_back( n );
                tasks[n][k].push_back( tasks[n-1][k][i] );
                tasks[n-1][k][i][j].pop_back();
            }
        }
    if(n-1 >= 0 && k-1 >=0 )
        for(int i=0; i<tasks[n-1][k-1].size() ; i++)
        {
            tasks[n-1][k-1][i].push_back( {n} );
            tasks[n][k].push_back( tasks[n-1][k-1][i] );
            tasks[n-1][k-1][i].pop_back();
        }
    return;
}

void generate_tasks()
{
    vector<vector<int>> t = {{0}};
    tasks[0][0].push_back(t);
    for(int n = 2; n <= mpl; n++)
    {
        vector<vector<vector<vector<int>>>> temp_n;
        for(int k=1; k<=n; k++)
        {
            vector<vector<vector<int>>> temp_k;
            task_aasign(n-1, k-1);
        }
    }
    // cout << "##########################################################################" << endl;
    // for(int n=0; n < mpl ; n++)
    // {
    //     for(int k=0; k<tasks[n].size() ; k++)
    //     {
    //         cout << "n: " << n+1 << " k: " << k+1 << endl;
    //         for(int t=0; t<tasks[n][k].size(); t++)
    //         {
    //             for(int l=0; l<tasks[n][k][t].size() ; l++)
    //             {
    //                 cout << "{" ;
    //                 for(int j=0; j<tasks[n][k][t][l].size() ; j++ )
    //                     cout << tasks[n][k][t][l][j] << " ";
    //                 cout << "}";
    //             }
    //             cout << " | ";
    //         }
    //         cout << endl;
    //     }
    //     cout << "-----------------------------" << endl;
    // }
}

string joint_state_to_string(vector<vector<int>> &v)
{
    int m = v[0].size();
    string s = ""; 
    for(int i =0; i<No_Robots; i++)
    {
        for(int j=0; j<m; j++)
        {
            stringstream ss;
            ss << v[i][j];
            if(v[i][j]/10 == 0) // coord is single digit
                s = s+"0"+ss.str();
            else
                s = s+ss.str();
            s+=",";
        }
    }

    return s;
}

void generate_prop_state(vector<int> &s, int index , vector<vector<int>> &Sol, vector<int> P )
{
    if(index==s.size())
    {
        Sol.push_back(P);
        return;
    }

    // if(s[index]==-1)
    // {
    //     P.push_back(0);
    //     generate_prop_state(s, index+1, Sol, P);
    //     return;
    // }

    for(int i=0; i<s[index]; i++)
    {
        P.push_back(i);
        generate_prop_state(s, index+1, Sol, P);
        P.pop_back();
    }
    return;

}
void generate_prop_state_count( vector<int > &s , string S )
{
    vector<vector<int>> Sol;
    vector<int > P;
    generate_prop_state(s, 0, Sol, P);
    prop_state_count_list[S] = Sol;

    // for(int i=0; i<Sol.size(); i++)
    // {
    //     for(int j=0; j<Sol[i].size();j++ )
    //         cout << Sol[i][j] << " ";
    //     cout << endl;
    // }
}

string vec_to_string(vector<int> &vec)
{
    string s="";
    for(int i=0;i<vec.size();i++)
    {
        stringstream ss;
        ss << vec[i];
        if(vec[i]/10 == 0) // coord is single digit
            s = s+"0"+ss.str();
        else
            s = s+ss.str();
        // s+=",";
    }
    return s;
}

void generate_n_c_r(vector<vector<int>> &Sol, vector<int> P , int r , int index)
{
    if(r==0)
    {
        Sol.push_back(P);
        return;
    }  
    if(index >=No_Robots)
        return;
    
    P.push_back(index);
    generate_n_c_r(Sol, P, r-1, index+1);
    P.pop_back();
    generate_n_c_r(Sol, P, r, index+1);

    return;
}

void generate_n_c_r_list()
{
    for(int r=1; r<=No_Robots; r++)
    {
        vector<vector<int>> Sol;
        vector<int> P;
        generate_n_c_r(Sol, P , r , 0);
        n_c_r.push_back(Sol);
    }

    // for(int i=0; i<n_c_r.size(); i++)
    // {
    //     int count = 0;
    //     cout << "n: " << No_Robots << " r: " << i+1  << endl;
    //     for(int j=0; j<n_c_r[i].size();j++)
    //     {
    //         count++;
    //         for(int k=0; k<n_c_r[i][j].size();k++)
    //         {
    //             cout << n_c_r[i][j][k] << " " ;
    //         }
    //         cout << endl;
    //     }
    //     cout << "count is " << count << endl; 
    // }

}

void generate_permutations(vector<int> &a,const int size, const int n, vector<vector<int>> &Sol)
{ 
    if (size == 1) 
    { 
        Sol.push_back(a);
        return; 
    } 
  
    for (int i=0; i<size; i++) 
    { 
        generate_permutations(a,size-1,n, Sol); 
        if (size%2==1) 
            swap(a[0], a[size-1]); 
        else
            swap(a[i], a[size-1]); 
    } 
    return;
} 

void create_permutations()
{
    Permutations.clear();
    
    for(int n=1;n<=max_propositions_in_formulae;n++)
    {
        vector<int> P,S;
        vector<vector<int>> Sol;
        for(int i=0; i<n; i++) 
            S.push_back(i);
        generate_permutations(S,n,n,Sol);
        Permutations.push_back(Sol);
    }
    // ############## printing neighbour list ########
    // for(int n=0;n<Permutations.size();n++)
    // {
    //     int count = 0;
    //     for(int i=0; i<Permutations[n].size();i++){
    //         count++;
    //         for (int j=0; j<Permutations[n][i].size(); j++){
    //             cout << Permutations[n][i][j] << " " ;
    //         }
    //         cout << endl; 
    //     }
    //     cout << "Total neighbours: " << count << endl;
    // }
}

void generate_neighbour_list(vector<int> &S, vector<int> prefix, const int n, const int lenght)
{
    if (lenght == 1){
        for (int j = 0; j < n; j++){
            prefix.push_back(S[j]);
            Neighbours.push_back(prefix);
            prefix.pop_back();
        }
    }
    else{
        for (int i = 0; i < n; i++)    {
            prefix.push_back(S[i]);
            generate_neighbour_list(S, prefix, n, lenght - 1);
            prefix.pop_back();
        }
    }
}

void create_neighbour_list()
{
    Neighbours.clear();
    vector<int> S , P;
    for(int i=0; i<dir; i++) 
        S.push_back(i);
    generate_neighbour_list(S,P,dir,No_Robots);
    // ############## printing neighbour list ########
    // int count = 0;
    // for(int i=0; i<Neighbours.size();i++){
    //     count++;
    //     for (int j=0; j<Neighbours[i].size(); j++){
    //         cout << Neighbours[i][j] << " " ;
    //     }
    //     cout << endl; 
    // }
    // cout << "Total neighbours: " << count << endl;
}

size_t split(const string &txt, std::vector<std::string> &strs, char ch)
{
    size_t pos = txt.find( ch );
    size_t initialPos = 0;
    strs.clear();

    // Decompose statement
    while( pos != std::string::npos ) {
        strs.push_back( txt.substr( initialPos, pos - initialPos ) );
        initialPos = pos + 1;

        pos = txt.find( ch, initialPos );
    }

    // Add the last one
    strs.push_back( txt.substr( initialPos, std::min( pos, txt.size() ) - initialPos + 1 ) );

    return strs.size();
}

}