#include<bits/stdc++.h>
#include <stdexcept>
#include <sstream> 

#include"Basic_planner_variable.h"
using namespace std;
using namespace planner_info;

/***djkistra with qtot system states with loops,until,next,few corrections,integer hash improvements,consider node in productgraph for book-keeping***/
//correct the adj updations

unordered_map<unsigned long long int,vector<prod_graph_node> > node_neighbour;
unordered_map<string,vector<M_prod_graph_node> > new_node_neighbour;
clock_t t,t1;


void compute_suffix_cycles();

pair<int,int> create_automata_trans_table(char ltl_query[])
{
	
    string table_text="";
    const int BUFSIZE=128;
    char buf[BUFSIZE];
    FILE *fp;
    ofstream outfile;
    
    outfile.open("test_trans.dat");
    
    int automata_transition_cnt=0;
    pair<int,int> automata_info;

    //writing the automaton transition table returned by LTL2BA converter to trans.dat
    if ((fp = popen(ltl_query, "r")) == NULL)
    {
        printf("Error opening pipe!\n");
        return automata_info;
    }
    
    //copying the transition table to s
    while (fgets(buf, BUFSIZE, fp) != NULL) 
    {
        table_text = table_text+buf;
    }

    // cout << "table_text-" << table_text << endl;

    if(pclose(fp))  
    {
        printf("Command not found or exited with error status\n");
        return automata_info;
    }
    // cout<<"\n";

    // ################ printing table ##################### //
    // cout << "Printing the Buchi Automea  " << endl;

    // cout << table_text << endl;
    //###################################################

    //maps each automaton state to a  number
    unordered_map<string,int> mp;
    unordered_map<string,int> state; 
    vector<string> state_list; //stores all automaton states

    string tmp;
    int i=0;
    //cout<<s.size();
    while(table_text[i]!='T' && table_text[i]!='a')
        i++;

    int state_no=0;
    
    vector<string> table;  // stores transitions in temp fashion

    // 

    while(i<table_text.size())
    {
       tmp="";
       while(i<table_text.size() && table_text[i]!=':')
        {
           tmp+=table_text[i];
           i++;
        }
        if(tmp[0]=='}')
            break;
       if(state.find(tmp)==state.end())
        {
          state[tmp] = state_no;
          state_list.push_back(tmp);
          state_no++;
        }

        while(i<table_text.size() && tmp!="fi;")
        {
        tmp = "";
            while(i<table_text.size() && table_text[i]!='\t' && table_text[i]!=' ' && table_text[i]!='\n')
            {
                if(table_text[i]!='(' && table_text[i]!=')')
                tmp+=table_text[i];
                i++;
            }
            if(tmp!=" " && tmp!="" && tmp!="if" && tmp!="::" && tmp!="->" && tmp!="goto" &&  tmp!="fi;")
            table.push_back(tmp);
            i++;
        }

    }

    //###################### printing table ################
    //cout << "Prining table" << endl;
    //for(int k = 0; k<table.size();k++ )
    //    cout << "---" << table[k] << endl;

    //###################################################### 

    //cout << "Printing state_list" << endl;
    //for(int k=0; k<state_list.size();k++ )
    //    cout << "--->>>>>>" << state_list[k] << endl;

    // #####################################################

    // stores indices for each state to mp and stores final states to dest(vector<int>)
    for(i=0;i<state_list.size();i++)
    {
        mp[state_list[i]]=i;
        if(state_list[i][0]=='a' && state_list[i][1]=='c' && state_list[i][2]=='c')
        {
            dest.push_back(i);
            new_dest.insert(i);
        }
    }

   vector<vector<int> > transition_condn_list;
   vector<int> transition_condn;
   int x,y;
   int automaton_state=0;
   i=1;

   while(i<table.size())
   {
        if(table[i][0]==':')
        {
            automaton_state++;
        }
        else if(table[i]=="||")
        {
           transition_condn_list.push_back(transition_condn);
           transition_condn = vector<int>(0);
       }
       else if(table[i]=="&&")
       {
       }
       else if(mp.find(table[i])!=mp.end())  // this is transition condition
       {
           transition_condn_list.push_back(transition_condn);
           for(x=0;x<transition_condn_list.size();x++)
           {
               automata_transition_cnt++;
               outfile<<automaton_state<<" "<<mp[table[i]]<<" "<<transition_condn_list[x].size()<<" ";
               for(y=0;y<transition_condn_list[x].size();y++)
               {
                   outfile<<transition_condn_list[x][y]<<" ";
               }
               outfile<<"\n";
           }
           transition_condn_list = vector<vector<int> >(0);
           transition_condn = vector<int>(0);
       }
       else if(table[i]=="1") // full  transition
       {
           transition_condn.push_back(0);
       }
       else if(table[i][0]=='!') // 
       {
           int v=0;
           for(x=2;x<table[i].size();x++)
               v = v*10+(table[i][x]-'0');
           transition_condn.push_back(-1*v);
       }
       else if(table[i][0]=='p')
       {
           int v=0;
           for(x=1;x<table[i].size();x++)
               v = v*10+(table[i][x]-'0');
           transition_condn.push_back(v);
       }
       else if(table[i]=="skip")      
        {automata_transition_cnt++; outfile<<automaton_state<<" "<<automaton_state<<" 1 0\n";}

       i++;
    }
    outfile.close();

    automata_info.first = state_list.size();
    automata_info.second = automata_transition_cnt;

    return automata_info;
}

void initializegrid(char *grid_info_file)
{
    string s;
    int i,j,k,no_of_trans,literal,num_obstacles;

    /////////////////////////////////////
    char buff[1000];
    FILE *ft;
    ft = fopen("query.dat", "r");
    /*reading LTL query*/
    fgets(buff, 1000, (FILE*)ft);

    pair<int,int> automata_info;
    /**reading the automata***/
    automata_info = create_automata_trans_table(buff);   // generates trans.dat(transitions of Buchi Autometa)  * imp *
    no_of_trans = automata_info.second;  // no of transitions in  buchi autometa
    qtot = automata_info.first;  // no of states in buchi autometa
    t = clock();

    /**vector for storing the automata transition table**/
    trans = vector< vector< vector< vector<int> > > >(qtot,vector< vector< vector<int> > >(qtot) );
    /**storing automata transitions on conjunction of negative literals**/
    negtrans = vector< vector< vector< vector<int> > > >(qtot,vector< vector< vector<int> > >(qtot) );
    neg_trans_to_neighbour = vector<int>(qtot);
    ifstream ifile;
    ifile.open("test_trans.dat");
    cout<<"Automata States: "<<qtot<<"\n";
    cout<<"Number of automata_transitions="<<no_of_trans<<"\n";
    /**reading automata transitions**/
    vector<int> automata_states;
    vector<int> transition_condn;
    
    for(i=0;i<no_of_trans;i++)
    {
        transition_condn = vector<int>(0); //size 0 vector
        automata_states = vector<int>(2); // size 2 vector
        ifile>>automata_states[0];
        ifile>>automata_states[1];
        int trans_condn_len=0,neg_literals=0,strict_neg_literals=0;
        ifile>>trans_condn_len;
        for(j=0;j<trans_condn_len;j++)
        {
            ifile>>literal;
            transition_condn.push_back(literal);
            if(literal <= 0)
                neg_literals++;
            if(literal < 0)
                strict_neg_literals++;
        }
        /**storing transition condition  in automata**/
        trans[automata_states[0]][automata_states[1]].push_back(transition_condn);

        /**if transition condition is conjunction of negative literals**/ 
        if(neg_literals==trans_condn_len)  // All literal are -ve for a trans
        {
            if(automata_states[0]!=automata_states[1] && strict_neg_literals==trans_condn_len)
                neg_trans_to_neighbour[automata_states[0]]=1;  // stores node for which there is a -ve trans to some other node(all literals are -ve)
            negtrans[automata_states[0]][automata_states[1]].push_back(transition_condn);
        }

    }

    ////////////// printing start /////////////////

    // cout << "Printing Trans" << endl;

    // for(int i=0; i<trans.size() ; i++)
    //     for(int j=0; j<trans[i].size() ; j++)
    //         for(int k=0; k<trans[i][j].size() ; k++)
    //             for(int l=0; l<trans[i][j][k].size() ; l++)
    //                 cout << "---" << i << " " << j << " " << trans[i][j][k][l] << endl;

    // cout << "Printing negtrans" << endl;

    // for(int i=0; i<negtrans.size() ; i++)
    //     for(int j=0; j<negtrans[i].size() ; j++)
    //         for(int k=0; k<negtrans[i][j].size() ; k++)
    //             for(int l=0; l<negtrans[i][j][k].size() ; l++)
    //                 cout << "---" << i << " " << j << " " << negtrans[i][j][k][l] << endl;

    // cout << "Printing neg_trans_to_neighbour" << endl;

    // for(int i=0; i<neg_trans_to_neighbour.size(); i++)
    //     cout << "---" << neg_trans_to_neighbour[i] << endl;

    ////////////// printing end /////////////////


    /////////////// starting with the grid file //////////////////////

    ifstream grid_file;
    grid_file.open(grid_info_file);
    int x,y;
    grid_file>>nrow; // rows
    grid_file>>ncol; // columns
    sz = max(nrow,ncol); 

    grid_file>>num_obstacles;
    grid_sz = sz;
    sz = max(sz,qtot);
    /*storing obstacle coordinates*/
    for(i=0;i<num_obstacles;i++)
    {
        grid_file>>x;  // first take y coord
        grid_file>>y;  // then take x coord
        grid[y*sz+x]=-1; // update in a grid which  is a map
    }

    // ************* Adding info about no of robots *********//
    grid_file>>No_Robots;
    if(No_Robots < 1) 
        throw std::invalid_argument( "No of robots should be greater than 1");

    for(int i=0 ; i<No_Robots ; i++)
    {
        int x, y;
        grid_file>>x;
        grid_file>>y;
        point Pt(x,y);
        INI_LOC.push_back(Pt); 
    }

    ////// Printing robot initial locations ////
    // cout << "Printing robot initial locations " << endl;
    // for(int i=0 ; i<No_Robots ; i++)
    //     cout << "Robot " << i+1 << " ini_loc: " << INI_LOC[i].x << " " << INI_LOC[i].y << endl;

    ///////////////////////////////////////////
    
    int num_pos_system_states;
    vector<set<int>> rob_literal;
    vector<int> grid_state;
    
    /** reading coordinates of states and proposition true at it **/
    for(int k = 0; k<No_Robots ; k++)
    {
        vector<vector<int> > temp_pos_system_state;   // just stores states where some prop is true
        unordered_map<string,vector<int> > temp_prop; // string(coord) with prop true at it     
        vector<vector< vector<int> > > temp_prop_sys_states(1000); //## remember to update this as per TS space 
        set<int> lit;
        grid_file>>num_pos_system_states;  // no of proposition system states where some proposition is true.
        // cout << "propositions for robot " << k+1 << "has propositions true at " << num_pos_system_states << " states" << endl;
        grid_file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        for(int i=0;i<num_pos_system_states;i++)
        {
            string line;
            vector<string> v;

            getline(grid_file , line);            
            split(line, v , ' ');
            grid_state = vector<int>(2);

            stringstream geek(v[0]); 
            grid_state[0] = 0; 
            geek >> grid_state[0];

            stringstream geek1(v[1]); 
            grid_state[1] = 0; 
            geek1 >> grid_state[1];

            for(int j=2; j<v.size();j++)
            {
                stringstream geek(v[j]); 
                int literal = 0; 
                geek >> literal;   // proposition true at x, y
                s = conv_vec_to_string(grid_state); //string in which coords are seperated by comma
                /**pushing the proposition true at state s**/
                temp_prop[s].push_back(literal);
                /**storing cells associated with a proposition**/
                temp_prop_sys_states[literal].push_back(grid_state);
                /**storing the list of states with a proposition true at it**/
                if(temp_prop[s].size()==1)
                    temp_pos_system_state.push_back(grid_state);

                lit.insert(literal);
                lit_set.insert(literal);

            }
        }

        rob_prop.push_back(temp_prop);
        rob_pos_system_state.push_back(temp_pos_system_state);
        rob_prop_sys_states.push_back(temp_prop_sys_states);
        rob_literal.push_back(lit);

    }

    grid_file.close();

    adj = vector<unordered_map<int,float> >(1000);
    updated = vector<unordered_map<int,int> >(1000);

    //////////////// printing /////////////////

    // cout << "printing rob_prop " << endl;
    // for(int r = 0; r < No_Robots ; r++)
    // {
    //     cout << "rob_prop for robot no " << r+1 << endl;    
    //     for(int i=0; i<rob_pos_system_state[r].size(); i++)
    //     {
    //         cout << "-----------------"  << conv_vec_to_string (rob_pos_system_state[r][i]) << endl;
    //         for(int j=0; j<rob_prop[r][ conv_vec_to_string (rob_pos_system_state[r][i])  ].size(); j++)
    //             cout << rob_prop[r][ conv_vec_to_string (rob_pos_system_state[r][i]) ][j] << " ";
    //         cout << endl;

    //     }
    // }

    // cout << "printing rob_prop_sys_states " << endl;

    // for(int r = 0; r < No_Robots ; r++)
    // {
    //     cout << "rob_prop_sys_states for robot no " << r+1 << endl;   
    //     for(set<int>::iterator i=rob_literal[r].begin(); i!=rob_literal[r].end(); i++)
    //     {
    //         int lit = *i; 
    //         cout << "---------- literal " << lit << endl;
    //         for(int j=0; j<rob_prop_sys_states[r][lit].size(); j++)
    //             cout << "x: " <<rob_prop_sys_states[r][lit][j][0] << ", y: " <<  rob_prop_sys_states[r][lit][j][1] << endl;
    //     }
    // }

    // cout << "printing lit_set " << endl;
    // for(set<int>::iterator i=lit_set.begin(); i!=lit_set.end(); i++)
    //     cout << *i << " ";
    // cout << endl;
    // //////////////////////////////////////

    return;
}

int new_dkj_printpath(M_dkj_node* nd)
{
    //cout<<"\n\n";
    if(nd!=NULL)
    {
        int path_len = new_dkj_printpath(nd->parent);
        new_cur_path.push_back(nd->coord);
        
        if(nd->parent!=NULL)
            return path_len+Product_E[nd->parent->coord][nd->coord];
    }
    return 0;
}

void construct_joint_ts()
{
    // tasks -     1.Construct Joint TS  
    //             2.Fill Prop , pos_system_state,Prop_sys_states 
    //             3.Should be flexible for Multi-robot and also single robot

    // creating a neighbour list
    create_neighbour_list();
    int edge_count = 0;  

    vector<vector<int>> curr_state;     // prop of init state 
    unordered_set<int> curr_prop;       // prop of init state 
    for(int i=0; i<No_Robots ; i++){    // current state is init state
        vector<int> s;
        s.push_back(INI_LOC[i].x);
        s.push_back(INI_LOC[i].y);       
        curr_state.push_back(s);
        curr_prop.insert(  rob_prop[i][conv_vec_to_string(s)].begin(), rob_prop[i][conv_vec_to_string(s)].end() ); // taking union
    }
    //############# Printing the info about initial state ################
    
    // cout << "printing curr_prop" << endl;
    // for(unordered_set<int>::iterator i = curr_prop.begin(); i != curr_prop.end();i++)
    //     cout << *i << " ";
    // cout << endl;
    //####################################################################
    string curr = joint_state_to_string(curr_state);
    Joint_N[curr] = 0; // add init node to the Node_list 
    if(curr_prop.size()!=0)
    {
        new_prop[curr] = curr_prop;
        // new_pos_system_state.push_back(curr_state);
        new_pos_system_state.push_back(curr);
        for(unordered_set<int>::iterator i = curr_prop.begin(); i != curr_prop.end();i++)
            new_prop_sys_states[*i].insert(curr);
    }
    // starting bfs
    queue<string> Q;
    unordered_set<string> visited;
    Q.push(curr);
    visited.insert(curr);
    
    while(!Q.empty())
    {
        curr = Q.front();
        Q.pop();
        
        vector<vector<int>> curr_state(No_Robots);
        string_to_joint_state(curr, curr_state);     
        // cout << "Construct joint TS" << " Exploring " << curr << endl;
        //################### generating neighbours ###############
        int N = Neighbours.size();
        for( int n=0; n<N ; n++)
        {
            vector<int > Nie = Neighbours[n];
            unordered_set<int> next_prop;
            vector<vector<int>> next_state(No_Robots);
            for(int i=0; i<No_Robots; i++)  // compute neighbour
            {
                int dim = curr_state[0].size();
                for(int d=0; d<dim;d++)
                {
                    next_state[i].push_back(curr_state[i][d] + nb[Nie[i]][d]);                
                }
            }
            if(valid_joint_state(next_state))  // if neighbour is valid
            {
                string next = joint_state_to_string(next_state); 
                // cout << "Neighbour added" << next << endl;
                Joint_N[next] = 0; // add init node to the Node_list
                // Joint_E[curr][next] = No_Robots; 
                // Joint_E[next][curr] = No_Robots; 
                Joint_E[curr][next] = new_cal_heuristic_cost(curr , next); 
                Joint_E[next][curr] = new_cal_heuristic_cost(curr,next);
                edge_count += 2; 
                if(visited.find(next) == visited.end())
                {
                    Q.push(next);
                    visited.insert(next);
                    //print_joint_state(next_state);
                    
                    for(int i=0; i<No_Robots ; i++){
                        vector<int> S;
                        int dim = curr_state[0].size();
                        for(int d=0; d<dim;d++)
                            S.push_back(next_state[i][d]);
                        next_prop.insert(  rob_prop[i][conv_vec_to_string(S)].begin(), rob_prop[i][conv_vec_to_string(S)].end() ); // taking union  
                    }       
                   
                    
                    if(next_prop.size()!=0)
                    {
                        new_prop[next] = next_prop;
                        // new_pos_system_state.push_back(next_state);
                        new_pos_system_state.push_back(next);
                        for(unordered_set<int>::iterator i = next_prop.begin(); i != next_prop.end();i++)
                            new_prop_sys_states[*i].insert(next);
                    }

                }
            }    

        }
    }

    //########################################################################################

    // cout << "################### Printing new_prop" << endl;

    // for(int i =0; i<new_pos_system_state.size();i++)
    // {
    //     // string curr = joint_state_to_string(new_pos_system_state[i]);
    //     string curr = new_pos_system_state[i];
    //     cout << "----------------joint state " << curr << endl;
    //     for(unordered_set<int>::iterator i = new_prop[curr].begin(); i!=new_prop[curr].end(); i++ )
    //         cout << *i << " " ;
    //     cout << endl;

    // }
    // cout << "######################Total states in Joint TS " << Joint_N.size() << endl;
    // cout << "######################Total states with propositions " << new_pos_system_state.size() << endl;

    // cout << "################### Printing new_prop_sys_states" << endl;
    // for(set<int>::iterator i = lit_set.begin(); i!=lit_set.end();i++)
    // {
    //     cout << "----------------literal " << *i << " has nodes " << new_prop_sys_states[*i].size() << endl;
    //     for(unordered_set<string>::iterator j=new_prop_sys_states[*i].begin(); j!=new_prop_sys_states[*i].end();j++ )
    //         cout << *j << endl;  
    // }

    //########################################################################################

    // cout << "Nodes in Joint TS:" << Joint_N.size() << endl;
    // cout << "Edges in Joint TS:" << edge_count << endl;
    return;
}

void generate_product_graph()
{
    int ini_buchi_state = 0;
    string ini_ts_state = generate_init_string();
    int edge_count = 0;
    // cout << "ini_state " << ini_buchi_state << " " << ini_ts_state << endl;

    string start_state = new_key(ini_ts_state , ini_buchi_state);
    // cout << "start state " << start_state << endl;

    Product_N[start_state] = 0;

    queue< pair<string , int> > Q;   // TS state and Buchi state
    unordered_set<string> visited;
    Q.push(make_pair(ini_ts_state ,ini_buchi_state));
    visited.insert(start_state);

    while(!Q.empty())
    {
        pair<string , int>  curr = Q.front();
        string curr_T_state = curr.first;
        int curr_B_state = curr.second;
        Q.pop();
        string curr_pt = new_key(curr_T_state , curr_B_state); // joint state

        for(int j=0;j<qtot;j++)
        {
            for(int z=0;z<negtrans[curr_B_state][j].size();z++) // there could be multiple transitons bwtween two buchi states
            {
                vector<int> transition_req = negtrans[curr_B_state][j][z];

                // This is done for all_neg_literals kind of transition  // I will modify  this later 
                for(unordered_map<string, int>::iterator i=Joint_E[curr_T_state].begin(); i!=Joint_E[curr_T_state].end() ; i++)  // iterating over neighbours of curr-state
                {
                    string nbh = (*i).first;// neighbour string
                    int wt = (*i).second;  // weight of the edge

                    unordered_set<int> prop_nbh = new_prop[nbh];  // propositions true at neighbour

                    int trans_literals_satisfied=0;
                    for(int k=0;k<transition_req.size();k++)
                    {
                        
                        if(transition_req[k]>=0)
                            for(unordered_set<int>::iterator l= prop_nbh.begin() ; l!=prop_nbh.end();l++)
                            {
                                
                                if(transition_req[k]==*l)
                                {
                                trans_literals_satisfied++; break;
                                }
                            }
                        else
                        {
                            int satisfied_neg_literal=1;

                            for(unordered_set<int>::iterator l= prop_nbh.begin() ; l!=prop_nbh.end();l++)
                            {
                                
                                if(-1*transition_req[k]==*l)
                                {
                                    satisfied_neg_literal=0; break;
                                }
                            }
                            trans_literals_satisfied+=satisfied_neg_literal;
                        }
                    }
                    if(trans_literals_satisfied==transition_req.size())
                    {
                        string nbh_node = new_key(nbh, j);

                        if(new_dest.find(j)!=new_dest.end())
                            Final_states.insert(nbh_node);

                        Product_N[nbh_node] = 0;
                        Product_E[curr_pt][nbh_node] = wt;
                        edge_count++;
                        if(visited.find(nbh_node)==visited.end())
                        {
                            visited.insert(nbh_node);
                            Q.push( make_pair(nbh,j) );    
                        }
                        
                    }
                }
            }


            // adding positive transitions // 

            for(int z=0;z<trans[curr_B_state][j].size();z++)
            {
                vector<int> transition_req = trans[curr_B_state][j][z];

                for(unordered_map<string, int>::iterator i=Joint_E[curr_T_state].begin(); i!=Joint_E[curr_T_state].end() ; i++)  // iterating over neighbours of curr-state
                {
                    string nbh = (*i).first;// neighbour string
                    int wt = (*i).second;  // weight of the edge

                    unordered_set<int> prop_nbh = new_prop[nbh];  // propositions true at neighbour

                    int trans_literals_satisfied=0;
                    for(int k=0;k<transition_req.size();k++)
                    {
                        if(transition_req[k]>=0)
                            for(unordered_set<int>::iterator l= prop_nbh.begin() ; l!=prop_nbh.end();l++)
                            {
                                if(transition_req[k]==*l)
                                {
                                trans_literals_satisfied++; break;
                                }
                            }
                        else
                        {
                            int satisfied_neg_literal=1;
                            for(unordered_set<int>::iterator l= prop_nbh.begin() ; l!=prop_nbh.end();l++)
                            {
                                if(-1*transition_req[k]==*l)
                                {
                                    satisfied_neg_literal=0; break;
                                }
                            }
                            trans_literals_satisfied+=satisfied_neg_literal;
                        }
                    }
                    if(trans_literals_satisfied==transition_req.size())
                    {
                        string nbh_node = new_key(nbh, j);
                        if(new_dest.find(j)!=new_dest.end())
                            Final_states.insert(nbh_node);
                        Product_N[nbh_node] = 0;
                        Product_E[curr_pt][nbh_node] = wt;
                        edge_count++;
                        if(visited.find(nbh_node)==visited.end())
                        {
                            visited.insert(nbh_node);
                            Q.push( make_pair(nbh,j) );    
                        }
                        
                    }
                }
            }

        }

    }

    // cout << "############################################### Printing Product_E" << endl;

    // for(unordered_map<string,unordered_map<string,int>>::iterator i = Product_E.begin() ; i!=Product_E.end(); i++ )
    // {
    //     cout << "-------------------------------Printing neighbours of " << i->first << endl;
    //     for(unordered_map<string,int>::iterator j = Product_E[i->first].begin(); j!=Product_E[i->first].end();j++)
    //         cout << j->first << " --weight-- " << j->second << endl; 
    // }

    // cout << "################################################# Final states in Product_E, count is " << Final_states.size() << endl; 

    // for(unordered_set<string>::iterator i = Final_states.begin() ; i!= Final_states.end() ; i++)
    //     cout << *i << endl;

    cout << "Nodes in Product TS:" << Product_N.size() << endl;
    cout << "Edges in Product TS:" << edge_count << endl;

    return;
}


int find_path_in_prod_graph_dkj(string srcnode,string dest_node, int degen)
{
    
    unordered_map<string , M_dkj_node*> ini;  // set of vertices    
    unordered_set<string> explored;  // set of vertices whose distance from the source has been finalised 
    M_dkj_node* st = M_new_dkj_node(srcnode); // node for initial node
    vector<M_dkj_node*> closed; // to delete nodes  
    priority_queue< M_dkj_node*,vector<M_dkj_node*>,M_dkj_node_comparator> qopen;   // priority queue
    int path_valid = 0; 
    int path_found = 0;   
    
    closed.push_back(st);
    qopen.push(st);
    //if zero length paths are allowed//
    if(degen)
    {
        ini[srcnode] = st;
        path_valid=1;  //path_valid denotes path length is greater than 0 or path length of 0 is allowed in case of finding prefix only//
    }
    
    while(!qopen.empty())
    {
        M_dkj_node* cur_nd = qopen.top();
        qopen.pop();
        string cur_pt = cur_nd->coord;
        
        if(path_valid)
        {
            if(explored.find(cur_pt)!=explored.end())   // checks whether node has been expanded or not
                continue;
            else
                explored.insert(cur_pt); // Node is being explored
        }

        if(path_valid && cur_pt == dest_node)  // Destination node is reached
        {
            
            
            new_cur_path = vector<string>(0);
            int len = new_dkj_printpath(cur_nd);  // update new_current_path
            if(!degen)  // if source and destination are same
            {
                if(lowest_suffix_value > cur_nd->g)
                        lowest_suffix_value = cur_nd->g;
                    
                M_dkj_node* f = M_new_dkj_node(cur_nd->coord);
                f->f = cur_nd->g;            
                djk_stored_suffix_paths[cur_nd->coord] = new_cur_path;
                djk_loopf.push(f);
            }
            else 
                prefix_length = cur_nd->f;
                path_found = -5;
            break;

        }

        if(!path_valid)
            path_valid=1;

        if( !degen && cur_nd->g >= lowest_suffix_value)
        {
            break;
        }

        //######################  Adding the neighbours to the queue ############### //

        for(unordered_map<string,int>::iterator i = Product_E[cur_pt].begin() ; i!=Product_E[cur_pt].end() ; i++)
        {
            string nbh = i->first;
            int wt = i->second;
            M_dkj_node* nbh_nd = M_new_dkj_node(nbh); // node for initial node
            int new_dist = cur_nd->g + wt;
            nbh_nd->g = new_dist;
            nbh_nd->h = 0;  // no heuristic for now;
            nbh_nd->f = nbh_nd->g + nbh_nd->h;
            nbh_nd->parent = cur_nd;
            closed.push_back(nbh_nd);
            
            if(ini.find(nbh)==ini.end())
            {
                ini[nbh]=nbh_nd;
                qopen.push(nbh_nd);
            }
            else
            {
                M_dkj_node* old_nbh_node = ini[nbh];
                if(old_nbh_node->g > nbh_nd->g )
                {
                    ini[nbh]=nbh_nd;
                    qopen.push(nbh_nd);
                }
            }
        }
        //######################################################################### //

        
    
    }
    for(int i=0;i<closed.size();i++)
        delete(closed[i]);

    return path_found;
   
}

int main(int args,char **argv)
{
    t = clock();
    initializegrid(argv[1]);
    construct_joint_ts();
    generate_product_graph();
    
    t1 = clock() - t;
    double time_taken = ((double)t1)/CLOCKS_PER_SEC; // in seconds
    printf("\nprogram took %f seconds to till computing product graph \n", time_taken);

    int cntstate=0;
    //calsystates();

    pair<int,int> dest_node;
    compute_suffix_cycles();
    // loopf all suffix cycle starting nodes
    // cout << "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| starting with prefix " << new_loopf.size() << endl;
    while(!djk_loopf.empty())
    {
         M_dkj_node* finstate = djk_loopf.top();
         string ini = generate_init_string();
         string init = new_key(ini , 0); 
         djk_loopf.pop();

         int iter,cnt=0;
         // cout << "Finding Prefix -------------------- " << init  << "suffix_cost is " << finstate->f << " " << finstate->coord << endl;
         iter = find_path_in_prod_graph_dkj(init,finstate->coord,1);

         if(iter < 0)  // means if path is found
         {
             new_prefix_path = new_cur_path;
             new_suffix_path = djk_stored_suffix_paths[finstate->coord];
             suf_len = finstate->f;
             break;
         }
     }
    
     cout<<"length_prefix="<<prefix_length<<"\n";
     cout<<"Suffix Cost: "<<suf_len<<"\n";
     cout<<"\nprefix\n";
     for(int i=0;i<new_prefix_path.size();i++)
         cout<<"**"<<new_prefix_path[i]<<"\n";

    cout<<"\nsuffix\n";
    for(int i=0;i<new_suffix_path.size();i++)
         cout<<"**"<<new_suffix_path[i]<<"\n";

    t = clock() - t;
    time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds
 
    printf("\nprogram took %f seconds to execute \n", time_taken);
    cout << "Time: " << time_taken << endl;
    cout<<".............................................."<<endl;

    return 0;
}

void compute_suffix_cycles()
{
    /*******for every final state of Product automaton ***************/

    for(unordered_set<string>::iterator i=Final_states.begin(); i!=Final_states.end() ; i++)
    {    
        // cout << "suffix============================================================== starting find path for " << *i << endl;       
        find_path_in_prod_graph_dkj(*i,*i,0);
    }

    return;
}
