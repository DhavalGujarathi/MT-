#include<bits/stdc++.h>
#include <sstream>
// #include <string>
using namespace std;
namespace planner_info
{
int sz=10,qtot=3;
float prefix_length,pre_len,suf_len=numeric_limits<float>::infinity();
vector<int> dest;  //stores final states

/***djkistra with qtot system states with loops,until,next,few corrections,integer hash improvements,consider node in product-graph for book-keeping***/
//correct the adj updations
struct point
{
    int x,y;
    point(int a,int b)
    {
        x=a;
        y=b;
    }
    point()
    {

    }
};


int nrow,ncol;
vector<struct point> suffix_path,prefix_path,cur_path;
vector<vector<struct point> > stored_suffix_paths;
unordered_map<int,int> fin_aut_state;
// int dir=9,total_system_states=0,comp_red=0,tot_comp=0,grid_sz;
// int nb[][2]={{0,0},{0,-1},{-1,0},{1,0},{0,1},{-1,-1},{1,-1},{-1,1},{1,1}};
int dir=5,total_system_states=0,comp_red=0,tot_comp=0,grid_sz;
int nb[][2]={{0,0},{0,-1},{-1,0},{1,0},{0,1}};


unordered_map<long long int,int> grid;
vector< vector< vector< vector<int> > > > trans(qtot,vector< vector< vector<int> > >(qtot) );
vector< vector< vector< vector<int> > > > negtrans(qtot,vector< vector< vector<int> > >(qtot) );
vector<int> neg_trans_to_neighbour;
vector<vector<int> > pos_system_state;  // I added new one so delete this
vector<unordered_map<int,float> > adj;
vector<unordered_map<int,int> > updated;

unordered_map<string,vector<int> > prop;   // I added new one so delete this
vector<vector<int> > qsystate;  // I changed it
vector< vector< vector<int> > > qsytrans(qtot,vector< vector<int> >(qtot));
unordered_map<long long int,int> systate_no;
vector<int> path;
vector<int> prevpath;
vector<vector< vector<int> > > prop_sys_states(1000); 



long long int   key(vector<int> &v)
{
    long long int val;
    if(v.size()==3) 
    val = v[0]*sz*sz+v[1]*sz+v[2];
    else
    val = v[0]*1000+v[1];

    return val;
}
/*unsigned long long int key(vector<int> &vec)
{
    size_t b=0;
    hash_range(b,vec.begin(), vec.end());
    //cout<<"code="<<b<<"\n";
    return b;
}*/




string conv_vec_to_string(vector<int> &vec)
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
        s+=",";
    }
    return s;
}


struct trans_system_node
{
    float g,h,f;
    point coord;
};


void printvec(vector<int> &vec)
{
    for(int i=0;i<vec.size();i++)
        cout<<vec[i];
}

bool valid(int y,int x)
{
    if(x>=0 && x<ncol && y>=0 && y<nrow && grid[y*sz+x]!=-1)
        return 1;  // return true if node is valid
    return 0;
}

//high level nodes
struct prod_graph_node
{
    prod_graph_node* par;    // to be used in dijekstra
    prod_graph_node* child;  // to be used in dijkstra
    point coord;               
    float g,h,f;
    bool visit;
    int state,pstate;
};

//actual nodes in path
struct trans_system_path_node
{
    trans_system_path_node* par;
    trans_system_path_node* child;
    point coord;
    float g,h,f;
    bool visit;
    int state;
};


trans_system_path_node* newtrans_system_path_node(int x,int y,int s)
{
    trans_system_path_node* nd = new trans_system_path_node;
    nd->coord.x=x;
    nd->coord.y=y;
    nd->f = 0.0;
    nd->g = 0.0;
    nd->h = 0.0;
    nd->visit = 0;
    nd->state = s;
    nd->par=NULL;
    return nd;
}

prod_graph_node* new_prod_graph_node(int p,int s,int y,int x)
{
    //p-system state s- automata state
    prod_graph_node* nd = new prod_graph_node;
    nd->coord.y=y;
    nd->coord.x=x;
    nd->f = 0.0;
    nd->g = 0.0;
    nd->h = 0.0;
    nd->visit = 0;
    nd->pstate = p;  // location of node coords in pos_system_state
    nd->state = s;   // buchi state associated with this product node
    nd->par=NULL;
    return nd;
}

struct prod_graph_comparator
{
    bool operator()(prod_graph_node *a , prod_graph_node *b)
    {
        return (a->f > b->f);
    }
};



float cal_heuristic_cost(point a,point b)
{
    float mx = max(abs(a.y-b.y),abs(a.x-b.x));
    float mn = min(abs(a.y-b.y),abs(a.x-b.x));
    return mn*1.5+(mx-mn);

}



//priority_queue for suffixes
priority_queue<prod_graph_node*,vector<prod_graph_node*>,prod_graph_comparator> loopf;

struct trans_system_node_cost_comparator
{
    bool operator()(trans_system_node a , trans_system_node b)
    {
        return (a.f > b.f);
    }
};

struct trans_system_path_node_cost_comparator
{
    bool operator()(trans_system_path_node *a , trans_system_path_node *b)
    {
        return (a->f > b->f);
    }
};



//********** New Variables added *******************

// for function initgrid //
int No_Robots = 1; // default value
vector<point> INI_LOC;
int max_no_robots = 10;
// stores data as per individual propositions
vector<vector<vector<int> >> rob_pos_system_state;   // just stores states where some prop is true
vector<unordered_map<string,vector<int> >> rob_prop; // string(coord) with prop true at it
vector<vector<vector< vector<int> > >> rob_prop_sys_states; //## remember to update this as per TS space
set<int> lit_set;  // stores all the literals for all the robots

//########### DS for joint TS (Designed like a NewworkX graph) ############################
unordered_map<string, int> Joint_N; // node values
unordered_map< string, unordered_map <string , int> > Joint_E; // E[From_node][To_node] = weight 
vector<vector<int>> Neighbours;

// vector<vector<vector<int> >> new_pos_system_state;  // I changed it
vector<string> new_pos_system_state;  // I changed it
unordered_map<string,unordered_set<int> > new_prop;
unordered_map<int , unordered_set<string>> new_prop_sys_states;
unordered_map<string,int> new_systate_no;
int eta = 1;
unordered_set<int> new_dest;

unordered_map<string,int> new_ystate_no;
vector<string> new_suffix_path,new_prefix_path,new_cur_path;
vector<vector<string> > new_stored_suffix_paths;

unordered_map<string, int> Product_N; // node values
unordered_map<string, unordered_map <string , int> > Product_E; // E[From_node][To_node] = weight
unordered_set<string> Final_states;

unordered_map< string , vector<string> > djk_stored_suffix_paths;

int lowest_suffix_value = 500000;



string new_key( string s , int state)
{
    stringstream ss;
    ss << state;
    s = s+"-"+ss.str();
    return s;
}

struct M_dkj_node
{
    int g,h,f;
    M_dkj_node* parent;
    string coord;
};

M_dkj_node* M_new_dkj_node(string s)
{
    M_dkj_node* N = new M_dkj_node;
    N->g=0;
    N->h=0;
    N->f=0;
    N->coord = s;
    N->parent = NULL;
    return N;
};

struct M_dkj_node_comparator
{
    bool operator()(M_dkj_node* a , M_dkj_node* b)
    {
        return (a->f > b->f);
    }
};

priority_queue< M_dkj_node*,vector<M_dkj_node*>,M_dkj_node_comparator> djk_loopf;   // priority queue 

struct M_trans_system_node
{
    float g,h,f;
    string coord;
};

struct M_trans_system_node_cost_comparator
{
    bool operator()(M_trans_system_node a , M_trans_system_node b)
    {
        return (a.f > b.f);
    }
};

struct M_trans_system_path_node
{
    M_trans_system_path_node* par;
    M_trans_system_path_node* child;
    string coord;
    float g,h,f;
    bool visit;
    int state;
};




M_trans_system_path_node* M_newtrans_system_path_node(string z,int s)
{
    M_trans_system_path_node* nd = new M_trans_system_path_node;
    nd->coord = z;
    nd->f = 0.0;
    nd->g = 0.0;
    nd->h = 0.0;
    nd->visit = 0;
    nd->state = s;
    nd->par=NULL;
    return nd;
}

struct M_prod_graph_node
{
    M_prod_graph_node* par;    // to be used in dijekstra
    M_prod_graph_node* child;  // to be used in dijkstra
    string coord;               
    float g,h,f;
    bool visit;
    int state,pstate;
};
M_prod_graph_node* M_new_prod_graph_node(int p,int s , string coord)
{
    //p-system state s- automata state
    M_prod_graph_node* nd = new M_prod_graph_node;
    nd->coord=coord;
    nd->f = 0.0;
    nd->g = 0.0;
    nd->h = 0.0;
    nd->visit = 0;
    nd->pstate = p;  // location of node coords in pos_system_state
    nd->state = s;   // buchi state associated with this product node
    nd->par=NULL;
    return nd;
}

struct M_prod_graph_comparator
{
    bool operator()(M_prod_graph_node *a , M_prod_graph_node *b)
    {
        return (a->f > b->f);
    }
};

struct M_trans_system_path_node_cost_comparator
{
    bool operator()(M_trans_system_path_node *a , M_trans_system_path_node *b)
    {
        return (a->f > b->f);
    }
};

priority_queue<M_prod_graph_node*,vector<M_prod_graph_node*>,M_prod_graph_comparator> new_loopf;

bool valid_joint_state(vector<vector<int>> &v)
{

    for(int r=0 ; r<No_Robots ; r++)
    {
        int x = v[r][0];
        int y = v[r][1];    
        if(x<0 || x>=nrow || y<0 || y>=ncol || grid[y*sz+x]==-1)
            return 0; 
    }
    return 1;
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
    // printing neighbour list ########
    int count = 0;
    for(int i=0; i<Neighbours.size();i++){
        count++;
        for (int j=0; j<Neighbours[i].size(); j++){
            //cout << Neighbours[i][j] << " " ;
        }
        //cout << endl; 
    }
    //cout << "Total neighbours: " << count << endl;
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

string generate_init_string()
{
    vector<vector<int>> v(No_Robots);
    for(int i=0; i<No_Robots;i++)
    {
        v[i].push_back(INI_LOC[i].x);
        v[i].push_back(INI_LOC[i].y);
    }
    return joint_state_to_string(v);
}

void string_to_joint_state(string s , vector<vector<int>> &v)
{
    for(int i=0; i<No_Robots;i++)
    {
        char s2[] = {s[i*6] , s[i*6+1] , '\0' };
        v[i].push_back(atoi(s2));
        char s1[] = {s[i*6+3] , s[i*6+4] , '\0' };
        v[i].push_back(atoi(s1));
    }    
    return;
}

// class M_TS_graph{
// public:
    
//     unordered_map<string, int> N; // node values
//     unordered_map< string, unordered_map <string , int> > E // E[From_node][To_node] = weight

//     void Add_Node(string s)
//     {
//         // if( N.find(s) == N.end())
//         // {
//         //     N[s] = 1; // Node has been added successfully;
//         //     return true;
//         // }
//         // return false; // Node was already present
//         N[s] = 1;    
//     }
    
//     bool Add_Edge(string s , string t, int wt)
//     {
//         E[s][t] = wt;
//     }
    
//     long long int Node_count()
//         return (long long int)N.size();


// };
//########################################################

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

int new_cal_heuristic_cost(string a,string b)  // computes Heuristic cost for multi-robot node
{
    vector<vector<int>> VA(No_Robots);
    vector<vector<int>> VB(No_Robots);

    string_to_joint_state(a,VA);
    string_to_joint_state(b,VB);

    int cost = 0;
    for(int i=0;i<No_Robots;i++)
    {
        cost += abs(VA[i][0] - VB[i][0]) + abs(VA[i][1] - VB[i][1]); 
    }
    return cost;

}

//***************** END ****************************

}
