#include<bits/stdc++.h>
#include <stdexcept>
#include <sstream> 

#include"MTplanner_variables.h"
using namespace std;
using namespace planner_info;

/***djkistra with qtot system states with loops,until,next,few corrections,integer hash improvements,consider node in productgraph for book-keeping***/
//correct the adj updations

clock_t t, t1, t2;

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
    // cout << "Prining table" << endl;
    // for(int k = 0; k<table.size();k++ )
    //     cout << "---" << table[k] << endl;

    // //###################################################### 

    // cout << "Printing state_list" << endl;
    // for(int k=0; k<state_list.size();k++ )
    //     cout << "--->>>>>>" << state_list[k] << endl;

    //#####################################################

    // stores indices for each state to mp and stores final states to dest(vector<int>)
    for(i=0;i<state_list.size();i++)
    {
        mp[state_list[i]]=i;
        if(state_list[i][0]=='a' && state_list[i][1]=='c' && state_list[i][2]=='c')
        {
            dest.insert(i);
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
    // cout<<"Number of automata_transitions="<<no_of_trans<<"\n";
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
                neg_trans_to_neighbour[automata_states[0]]=1;  //stores node for which there is a -ve trans to some other node(all literals are -ve)
            negtrans[automata_states[0]][automata_states[1]].push_back(transition_condn);
        }

    }

    ////////////// printing start /////////////////

    // cout << "Printing Trans" << endl;

    // for(int i=0; i<trans.size() ; i++)
    //     for(int j=0; j<trans[i].size() ; j++)
    //         for(int k=0; k<trans[i][j].size() ; k++)
    //         {
    //         	cout << "---" << i << " " << j << " == ";
    //             for(int l=0; l<trans[i][j][k].size() ; l++)
    //                 cout << trans[i][j][k][l] << " ";
    //         	cout << endl;
    //         }


    // cout << "Printing negtrans" << endl;

    // for(int i=0; i<negtrans.size() ; i++)
    //     for(int j=0; j<negtrans[i].size() ; j++)
    //         for(int k=0; k<negtrans[i][j].size() ; k++)
    //             for(int l=0; l<negtrans[i][j][k].size() ; l++)
    //                 cout << "---" << i << " " << j << " " << negtrans[i][j][k][l] << endl;

    // cout << "Printing neg_trans_to_neighbour" << endl;

    // for(int i=0; i<neg_trans_to_neighbour.size(); i++)
    //     cout << "---" << i << "--->" << neg_trans_to_neighbour[i] << endl;

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
        INI_LOC.push_back(y * sz + x); 
    }    
    int num_pos_system_states;
    vector<int> grid_state;
    
    /** reading coordinates of states and proposition true at it **/
    for(int k = 0; k<No_Robots ; k++)
    {
        vector<vector<int> > temp_pos_system_state;   // just stores states where some prop is true
        unordered_map<int,vector<int> > temp_prop; // int(coord) with prop true at it     
        vector<vector< vector<int> > > temp_prop_sys_states(10000); //## remember to update this as per TS space 
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
                /**pushing the proposition true at state s**/
                int s = grid_state[1] * sz + grid_state[0];
                temp_prop[ s ].push_back(literal);
                /**storing cells associated with a proposition**/
                temp_prop_sys_states[literal].push_back(grid_state);
                /**storing the list of states with a proposition true at it**/
                if(temp_prop[s].size()==1)
                    temp_pos_system_state.push_back(grid_state);
                lit.insert(literal);
            }
        }

        rob_prop.push_back(temp_prop);
        rob_pos_system_state.push_back(temp_pos_system_state);
        rob_prop_sys_states.push_back(temp_prop_sys_states);
        rob_literal.push_back(lit);

    }
    grid_file.close();

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
    //////////////////////////////////////

    return;
}

// Generates final states and adds these nodes to Redc_N
void generate_final_states()
{
	create_permutations(); // generate permutations for generating reighbours in reduced graph
	// create_neighbour_list();
	generate_n_c_r_list();
	generate_tasks();
	generate_master_table();
	generate_prop_power_set_mapping();
	memo = vector<unordered_map<vector<long long int>, long long int, VectorHasher > >(No_Robots);
	pseudo_suffix_cycle = vector<vector<vector<long long int> > >(No_Robots);

	for(unordered_set<int>::iterator f = dest.begin(); f!=dest.end();f++)
	{
		int final_B_state = *f;
		for(int j=0;j<qtot;j++)
	    {
	        for(int z=0;z<trans[j][final_B_state].size();z++)
	        {
	            vector<int> transition_req = trans[j][final_B_state][z];
	            if(transition_req.size()==0)
	                continue;	            
	            vector<int> pos_lit, neg_lit;

	            for(int i=0; i<transition_req.size(); i++)
	            {
	            	if(transition_req[i] > 0)
	            		pos_lit.push_back(transition_req[i]);
	            	else if (transition_req[i]==0)
	            	{
	            		// cout << "Thats is like accept everything !!!!!!!";
	            	}
	            	else
	            		neg_lit.push_back(transition_req[i]);
	            }
	            int r = pos_lit.size();
	            if(r > 0) // this is positive transition so there has to be a positive transtion so r is always greater than 1
	            {
	            	for(int nbh=0; nbh < master_table[r-1].size() ; nbh++)
	            	{
	            		vector<int> v_prop_state_count(No_Robots, 1);
	            		int valid_task = 1;
	            		for(int i=0; i<No_Robots;i++)
	            		{
	            			vector<bool> nbh_bitmap(max_no_of_prop_def, false);	
	            			vector<int> task = master_table[r-1][nbh][i];
	            			if(task[0]==-1)
	            				continue;
	            			for(int e=0; e<task.size();e++)
	            				nbh_bitmap[ pos_lit[task[e]] ] = true;

	            			if( prop_bit_map[i].find( nbh_bitmap ) == prop_bit_map[i].end() )
	            			{
	            				valid_task=0;
	            				break;
	            			}        			
	            			v_prop_state_count[i] = prop_bit_map[i][nbh_bitmap].size();
	            		}
	            		if(!valid_task)  // there is a robot which cannot satisfy assigned task
	            			continue;

	            		string v_prop_state_count_string = vec_to_string(v_prop_state_count);

	            		if(prop_state_count_list.find( v_prop_state_count_string) == prop_state_count_list.end())
    	        			generate_prop_state_count(v_prop_state_count , v_prop_state_count_string);

	            		vector<vector<int>> per = prop_state_count_list[v_prop_state_count_string];
	            		for(int z=0; z < per.size(); z++ )
	            		{	
	            			vector<vector<int>> state(No_Robots , vector<int> (2, -1));
	            			int neg_valid = 1;
		            		for(int i=0; i<No_Robots; i++)
		            		{
		            			vector<bool> nbh_bitmap(max_no_of_prop_def, false);
		            			for(int e=0; e<master_table[r-1][nbh][i].size();e++)
	            					nbh_bitmap[ pos_lit[master_table[r-1][nbh][i][e]] ] = true;
		            			if( master_table[r-1][nbh][i][0] != -1 )
		            			{	
		            				int coord = prop_bit_map [i][nbh_bitmap] [ per[z][i] ];
		            				state[i][0] =   coord % sz  ; 
		            				state[i][1] =   coord / sz	;

		            				for(int neg = 0; neg_valid &&  neg < neg_lit.size(); neg++)
		            				{
		            					int lit = -1 * neg_lit[neg];
		            					for(int e=0; e < rob_prop[i][coord].size(); e++)
		            					{
		            						if(rob_prop[i][coord][e] == lit )
		            						{
		            							neg_valid = 0;
		            							break;
		            						}
		            					}
		            				}
		            			}

		            			if(!neg_valid)
		            				break;
		            		}

		            		if(!neg_valid)
		            			continue;

		            		string state_string = joint_state_to_string(state);
		            		int state_no;
		            		if(N_Reverse_Joint_N.find( state_string) == N_Reverse_Joint_N.end())
		            		{
		            			N_Joint_mapping.push_back(state);
		            			state_no = cur_joint_size++;
		            			N_Reverse_Joint_N[state_string] = state_no ;
		            			N_Joint_N[state_no] = 1;
		            		}
		            		else
		            			state_no = N_Reverse_Joint_N[state_string];
		            		Final_states.insert(state_no * redc_node_multiplier  + final_B_state);	
	            		}   	
	            	}
	            }       
	        }
	    }
	}

	//######## printing Final States ##########

	// for(int i=0; i<N_Joint_mapping.size(); i++)
	// {
	// 	for(int j=0; j<N_Joint_mapping[i].size(); j++)
	// 	{
	// 		cout << N_Joint_mapping[i][j][0] << " " << N_Joint_mapping[i][j][1] <<" | "; 
	// 	}
	// 	cout << endl;
	// }

	// cout << "Printing Final States " << endl;

	// for(unordered_set<long long int>::iterator i = Final_states.begin(); i!=Final_states.end();i++)
	// {
	// 	long long int joint_state = *i/1000;
	// 	int buchi_state = *i%1000;

	// 	for(int i=0; i<No_Robots;i++)
	// 	{
	// 		cout << N_Joint_mapping[joint_state][i][0] << " " << N_Joint_mapping[joint_state][i][1] << " | ";  
	// 	}
	// 	cout << buchi_state << endl;

	// }

	// cout << "Size of N_Joint_mapping is " << N_Joint_mapping.size() << endl;
	// cout << "Size of N_Joint_N is " << N_Joint_N.size() << endl;
	// cout << "Size of Final_states is " << Final_states.size() << endl;

	//#########################################
}


void generate_abstract_reduced_graph()
{
	vector<vector<int>> ini_T_state(No_Robots);
	int ini_B_state  = 0;
	int ini_T_state_no;
	for(int i=0; i<INI_LOC.size(); i++)
	{
		ini_T_state[i].push_back(INI_LOC[i] % sz);
		ini_T_state[i].push_back(INI_LOC[i] / sz);
	}
	string ini_T_state_string = joint_state_to_string(ini_T_state);

	if(N_Reverse_Joint_N.find( ini_T_state_string) == N_Reverse_Joint_N.end())
	{
		N_Joint_mapping.push_back(ini_T_state);
		ini_T_state_no = cur_joint_size++;
		N_Reverse_Joint_N[ini_T_state_string] = ini_T_state_no ;
		N_Joint_N[ini_T_state_no] = 1;
	}

	long long int ini_redc_no = ini_T_state_no * redc_node_multiplier + ini_B_state;   // this is our start state in the Reduced graph

	Redc_N[ini_redc_no] = 0;
    queue<long long int> Q;   // TS state and Buchi state
    Q.push(ini_redc_no);
    unordered_set<long long int> visited;
    visited.insert(ini_redc_no);

    while(!Q.empty())
    {
    	long long int  cur_no = Q.front();
        long long int cur_T_no = cur_no / redc_node_multiplier ;
        int cur_B_no = cur_no % redc_node_multiplier;
        Q.pop();       

        int negloop=0;  // I am not considering the SKIP transition  which may come in some Buchi Autometas
        //check whether exists a self transition onconjunction of neg literals of cur_B_no//
        if(negtrans[cur_B_no][cur_B_no].size()>0) 
            negloop=1;  

        for(int j=0;j<qtot;j++)
        {
        	for(int z=0;z<trans[cur_B_no][j].size();z++) // considering outgoing Buchi Trans from curr Buchi state
	        {
	        	vector<int> transition_req = trans[cur_B_no][j][z];
	            if(transition_req.size()==0)
	                continue;
	            vector<int> pos_lit, neg_lit;

	            for(int i=0; i<transition_req.size(); i++)
	            {
	            	if(transition_req[i] > 0)
	            		pos_lit.push_back(transition_req[i]);
	            	else if (transition_req[i]==0) // This is SKIP Transition
	            	{
	            		// cout << "Thats is like accept everything !!!!!!!"<<endl;
	            		// cout << "Error::: " << endl;
	            	}
	            	else
	            		neg_lit.push_back(transition_req[i]);
	            }

	            if(transition_req.size()==neg_lit.size())
	            	continue;
 	            int r = pos_lit.size(); // No of +ve literals in trans
	            for(int nbh=0; nbh < master_table[r-1].size() ; nbh++)
            	{
            		vector<int> v_prop_state_count(No_Robots, 1);
            		int valid_task = 1;
            		vector<vector<int>> trans_prop;
            		for(int i=0; valid_task && i<No_Robots;i++)
            		{
            			vector<bool> nbh_bitmap(max_no_of_prop_def, false);
            			vector<int> task = master_table[r-1][nbh][i];  // task assigned to a robot
            			if(task[0]==-1)
            			{
            				trans_prop.push_back(neg_lit);
            				continue;
            			}	
            			for(int e=0; e<task.size();e++) // creates a bitmap for the task
            				nbh_bitmap[ pos_lit[task[e]] ] = true;

            			if( prop_bit_map[i].find( nbh_bitmap ) == prop_bit_map[i].end() )
            			{
            				valid_task=0;
            				break;
            			}        			
            			v_prop_state_count[i] = prop_bit_map[i][nbh_bitmap].size();
            			vector<int> final_robot_prop;
            			
            			for(int u=0; u < task.size(); u++ )
            				final_robot_prop.push_back(pos_lit[task[u]]);
            			for(int u=0; u < neg_lit.size(); u++ )
            				final_robot_prop.push_back(neg_lit[u]);

            			trans_prop.push_back(final_robot_prop);
            		}
            		if(!valid_task)  // there is a robot which cannot satisfy assigned task
            			continue;
            		string v_prop_state_count_string = vec_to_string(v_prop_state_count);

            		if(prop_state_count_list.find( v_prop_state_count_string) == prop_state_count_list.end())
	        			generate_prop_state_count(v_prop_state_count , v_prop_state_count_string);

            		vector<vector<int>> per = prop_state_count_list[v_prop_state_count_string];
            		for(int z=0; z < per.size(); z++ )
            		{	
            			vector<vector<int>> state(No_Robots , vector<int> (2, -1));
            			int neg_valid = 1;
	            		for(int i=0; i<No_Robots; i++)
	            		{
	            			vector<bool> nbh_bitmap(max_no_of_prop_def, false);
	            			for(int e=0; e<master_table[r-1][nbh][i].size();e++)
            					nbh_bitmap[ pos_lit[master_table[r-1][nbh][i][e]] ] = true;

            					
	            			if( master_table[r-1][nbh][i][0] != -1 )
	            			{	
	            				int coord = prop_bit_map [i][nbh_bitmap] [ per[z][i] ];
	            				state[i][0] =   coord % sz  ; 
	            				state[i][1] =   coord / sz	;

	            				//check if nbh satisfies -ne propositions

	            				for(int neg = 0; neg_valid &&  neg < neg_lit.size(); neg++)
	            				{
	            					int lit = -1 * neg_lit[neg];
	            					for(int e=0; e < rob_prop[i][coord].size(); e++)
	            					{
	            						if(rob_prop[i][coord][e] == lit )
	            						{
	            							neg_valid = 0;
	            							break;
	            						}
	            					}
	            				}
	            			}

	            			if(!neg_valid)
	            				break;
	            		}

	            		if(!neg_valid)
	            			continue;
	                    if(!negloop &&  !Are_neighbour( cur_T_no , state  ))  // Adding only neighbours(for 2 axis system) // this is wrong 
                        	continue;
	            		string state_string = joint_state_to_string(state);
	            		int state_no;
	            		if(N_Reverse_Joint_N.find( state_string) == N_Reverse_Joint_N.end())
	            		{
	            			N_Joint_mapping.push_back(state);
	            			state_no = cur_joint_size++;
	            			N_Reverse_Joint_N[state_string] = state_no ;
	            			N_Joint_N[state_no] = 1;
	            		}
	            		else
	            			state_no = N_Reverse_Joint_N[state_string];

	            		int redc_state_no = state_no * redc_node_multiplier  + j;

	            		Redc_N[state_no * redc_node_multiplier  + j] = 1;
	            		Redc_E[cur_no][redc_state_no] = -1 ;
	            		Redc_E_Trans[cur_no][redc_state_no].push_back(trans_prop);

	            		if(visited.find( redc_state_no ) == visited.end())
	            		{
	            			visited.insert(redc_state_no);
	            			Q.push(redc_state_no);
	            		}

            		}   	
            	}
	        }	     
        }

        int flag = 0;
        for(int j=0;j<qtot;j++)
            if(j != cur_B_no && neg_trans_to_neighbour[cur_B_no] )
                flag = 1;
        if(flag==1)
        {
        	for(int j=0;j<qtot;j++)
        	{
        		for(int z=0;z<negtrans[cur_B_no][j].size();z++)
            	{
        			vector<vector<int>> state(No_Robots , vector<int> (2, -1));
        			string state_string = joint_state_to_string(state);
            		int state_no;
            		if(N_Reverse_Joint_N.find( state_string) == N_Reverse_Joint_N.end())
            		{
            			N_Joint_mapping.push_back(state);
            			state_no = cur_joint_size++;
            			N_Reverse_Joint_N[state_string] = state_no ;
            			N_Joint_N[state_no] = 1;
            		}
            		else
            			state_no = N_Reverse_Joint_N[state_string];
            		int redc_state_no = state_no * redc_node_multiplier  + j;
            		Redc_N[state_no * redc_node_multiplier  + j] = 1;
            		Redc_E[cur_no][redc_state_no] = -1 ;
            		vector<vector<int>> trans_prop;
            		for(int i=0; i<No_Robots; i++)
            			trans_prop.push_back(negtrans[cur_B_no][j][z]);

            		Redc_E_Trans[cur_no][redc_state_no].push_back(trans_prop);

            		if(visited.find( redc_state_no ) == visited.end())
            		{
            			visited.insert(redc_state_no);
            			Q.push(redc_state_no);
            		}
            	}
        	}
        }
        // Add neighbours to this node
    }

    /// ###############################################
    // cout << endl <<  "Prining Abstract Reduced Graph " << endl;
    // for( unordered_map<long long int,unordered_map<long long int,long long int>>::iterator i = Redc_E.begin() ; i!=Redc_E.end();i++)
    // {
    // 	cout << "----------Outgoing edges from ";
    // 	for(int r=0; r<No_Robots;r++)
    // 		cout <<   N_Joint_mapping[i->first / redc_node_multiplier ][r][0] << " " << N_Joint_mapping[i->first / redc_node_multiplier ][r][1] << " | " ;
    // 	cout << "(" << i->first % redc_node_multiplier << ")" << endl;

    // 	for( unordered_map<long long int,long long int>::iterator j = Redc_E[i->first].begin() ; j != Redc_E[i->first].end(); j++ )
    // 	{
	   //  	for(int r=0; r<No_Robots;r++)
	   //  		cout <<   N_Joint_mapping[j->first / redc_node_multiplier ][r][0] << " " << N_Joint_mapping[j->first / redc_node_multiplier ][r][1] << " | " ;
    // 		cout << "(" << j->first % redc_node_multiplier << ")" << endl;


    // 		for(int l=0; l< Redc_E_Trans[i->first][j->first].size() ; l++)
    // 		{
    // 			cout << "Trans No: " << l+1 << "   ===>  ";
    // 			for(int r=0; r<Redc_E_Trans[i->first][j->first][l].size() ;r++)
    // 			{
    // 				if(Redc_E_Trans[i->first][j->first][l][r].size() == 0)
    // 				{
    // 					cout << "NULL | ";
    // 					continue;
    // 				}
    // 				for(int h=0; h<Redc_E_Trans[i->first][j->first][l][r].size();h++)
    // 				{
    // 					cout << Redc_E_Trans[i->first][j->first][l][r][h] << " " ;
    // 				}
    // 				cout << " | ";
    // 			}
    // 			cout << endl;
    // 		}

    // 	}

    // }

    cout << "No of Nodes in abstract reduced graph: " << Redc_N.size() << endl;
    // cout << "No of Nodes in abstract reduced graph: " << N_Joint_mapping.size() << endl;
    long long int count = 0;
    unordered_map<long long int, unordered_map <long long int , long long int> >::iterator i = Redc_E.begin();
    for( ; i != Redc_E.end(); i++ )
    {
        for( unordered_map <long long int , long long int>::iterator j = Redc_E[i->first].begin(); j!= Redc_E[i->first].end(); j++)
        {
            count++;
        }
    }

    cout << "No of Edges in abstract reduced graph: " << count << endl;


}


long long int find_single_robot_path_cost( long long int src, long long int dest, int robot_no, vector<long long int>& cycle, int s_index , int e_index , vector<vector<vector<long long int>>> &actual_path  )
{
	long long int cost = 0;
	
	

    unordered_map<long long int, node*> init_vertex;
    unordered_map<long long int,int > vis;
    int d_x,d_y;
    d_x = N_Joint_mapping[dest/1000][robot_no][0];
    d_y = N_Joint_mapping[dest/1000][robot_no][1];
    long long int src_no = ( N_Joint_mapping[src/1000][robot_no][1] * sz + N_Joint_mapping[src/1000][robot_no][0] ) * 1000 +  (s_index % 1000);  // ((y*sx+x)*1000) + B_state
    long long int dest_no = ( N_Joint_mapping[dest/1000][robot_no][1] * sz + N_Joint_mapping[dest/1000][robot_no][0] ) * 1000 + (e_index % 1000);
    node * src_nd = newnode( src_no );  
    priority_queue<node*,vector<node*>,comp> qopen;
    
    vector<node*> closed;  // updates the nodes to be deleted
    qopen.push(src_nd);
    bool path_valid = false;

    //cout << "-----------(" << N_Joint_mapping[src/1000][robot_no][0] << N_Joint_mapping[src/1000][robot_no][1] << ")" << src%1000 << "------>";
	//cout << "(" << N_Joint_mapping[dest/1000][robot_no][0] << N_Joint_mapping[dest/1000][robot_no][1] << ")" << dest%1000;	

    if( N_Joint_mapping[src/1000][robot_no][0] == N_Joint_mapping[dest/1000][robot_no][0] && N_Joint_mapping[src/1000][robot_no][1] == N_Joint_mapping[dest/1000][robot_no][1] && src%redc_node_multiplier == dest % redc_node_multiplier )
	{	
		//cout << " ****************";
				
	}
	else
	{
		init_vertex[src_no] = src_nd;
		path_valid = true;
	}
	//cout << endl;
    
    while(!qopen.empty())
    {
    	node* cur_nd = qopen.top();
        qopen.pop();
        long long int cur_no = cur_nd->coord;
        long long int cur_T_no = cur_no / redc_node_multiplier; // single robot coord y*sz+z
        int cur_index = cur_no % redc_node_multiplier; // index of this node in the cycle
        int cur_B_no = cycle[cur_index] % 1000;

        //cout << "cur_node " << (cur_no/1000)%sz << " " << (cur_no/1000)/sz << " (" << cur_B_no << ") " << endl; 

        if(path_valid && cur_no == dest_no) // target found
        {
        	vector<long long int> path;
        	node * c = cur_nd -> par;
			while( c != NULL)
			{
				path.push_back(c->coord);
				long long int t = c->coord/1000;
				//cout << t%sz << " " << t/sz  << "|"; 
				c = c->par;
			}
			//cout << endl;	
			reverse(path.begin() , path.end());


			actual_path[robot_no].push_back(path);        	
        	for(int x=0; x<closed.size(); x++)
        		delete(closed[x]);
          	return cur_nd->f;
        }

       
        
        if(vis.find(cur_no)!=vis.end())
          continue;
        else
          vis[cur_no]=1;

        // Add neighbours here for the cur node
      	int x = cur_T_no % sz;
      	int y = cur_T_no / sz;

      	int nx,ny;

      	int n_b = cycle[ (cur_index+1)%cycle.size()] %1000;
      	
      	for(int j=0;   ( !(path_valid) || ((cur_index)%cycle.size()) != e_index   ) && j < Redc_E_Trans[ cycle[cur_index] ] [  cycle[ (cur_index+1)%cycle.size()  ]].size() ; j++)  // transition between transitionsa
      	{
      		vector<int> req = Redc_E_Trans[ cycle[cur_index] ] [ cycle[(cur_index+1)%cycle.size()] ][j][robot_no];
      		// neighbours that satisfy this transition
      		

      		for(int d=0; d < dir; d++ ) // iterating over each neighbour
      		{
      			nx = x + nb[d][0];
      			ny = y + nb[d][1];

      			if(!single_robot_valid( nx, ny ))
      				continue;

                vector<int> prop_cur;  // propositions true at curr node for curr robot
                if(rob_prop[robot_no].find( ny *sz + nx )==rob_prop[robot_no].end())
                    prop_cur.push_back(0);
                else
                {
                    prop_cur = rob_prop[robot_no][ny*sz+nx];
                    prop_cur.push_back(0);
                }

                int cnt=0;
                for(int k=0;k<req.size();k++)
                {
                    if(req[k]>=0)
                        for(int l=0;l<prop_cur.size();l++)
                        {
                            if(req[k]==prop_cur[l])
                            {
                                cnt++; break;
                            }
                        }
                        else
                        {
                            int tmp=1;
                            for(int l=0;l<prop_cur.size();l++)
                            {
                                if(-1*req[k]==prop_cur[l])
                                {
                                    tmp=0; break;
                                }
                            }
                            cnt+=tmp;
                        }
                }
                if(cnt==req.size())
                {
                	long long int n_no = ( ny*sz+nx ) * 1000 + (  (cur_index + 1)%cycle.size() ) ;
                    node* tmp = newnode(n_no);
                    closed.push_back(tmp);
                    tmp->d = cur_nd->d + abs(nx - x) + abs( ny - y);
                    // tmp->h = 0;
                    tmp->h = abs(nx - d_x) + abs( ny - d_y);  // we are using the heuristic cost
                    tmp->f = tmp->d + tmp->h;
                    tmp->par = cur_nd;
                    
                    // this node is a condidate
                    if(init_vertex.find( n_no )==init_vertex.end())  // node is not there
                    {
                    	init_vertex[n_no] = tmp;
                    	qopen.push(tmp);
                    	//cout << " Node Added " << nx << " " << ny << " (" << n_b << ") " << endl; 

                    }
                    else
			        {
			            node* oldtmp = init_vertex[n_no];
			            if(oldtmp->f > tmp->f )
			            {
			                init_vertex[n_no]=tmp;
			                qopen.push(tmp);
	                    	//cout << " Node Added " << nx << " " << ny << " (" << n_b << ") " << endl; 

			            }

			        }

                }
      		}
      	}

      	int negloop = 0;
      	if(negtrans[cur_B_no][cur_B_no].size()>0) 
            negloop=1;
       	if(negloop) // add transitions from neg self loop
       	{
       		for(int j=0; j<trans[ cur_B_no ] [cur_B_no].size() ; j++)  // transition between transitionsa
	      	{
	      		vector<int> req = trans[cur_B_no][cur_B_no][j];
	      		// neighbours that satisfy this transition
	      		bool neg_tran = true;
	      		for(int i=0; i<req.size();i++)
	      			if(req[i]>0)
	      			{
	      				neg_tran = false;
	      				break;
	      			}
	      		if(!neg_tran)
	      			continue;

	      		for(int d=0; d < dir; d++ ) // iterating over each neighbour
	      		{
	      			nx = x + nb[d][0];
	      			ny = y + nb[d][1];

	      			if(!single_robot_valid( nx, ny ))
	      				continue;
	                
	                vector<int> prop_cur;  // propositions true at curr node for curr robot
	                if(rob_prop[robot_no].find( ny *sz + nx )==rob_prop[robot_no].end())
	                {
	                    prop_cur.push_back(0);
	                }
	                else
	                {
	                    prop_cur = rob_prop[robot_no][ny*sz+nx];
	                    prop_cur.push_back(0);
	                }

	                int cnt=0;
	                for(int k=0;k<req.size();k++)
	                {
	                    if(req[k]>=0)
	                        for(int l=0;l<prop_cur.size();l++)
	                        {
	                            if(req[k]==prop_cur[l])
	                            {
	                                cnt++; break;
	                            }
	                        }
	                        else
	                        {
	                            int tmp=1;
	                            for(int l=0;l<prop_cur.size();l++)
	                            {
	                                if(-1*req[k]==prop_cur[l])
	                                {
	                                    tmp=0; break;
	                                }
	                            }
	                            cnt+=tmp;
	                        }
	                }
	                if(cnt==req.size())
	                {
	                    long long int n_no = ( ny*sz+nx ) * 1000 + cur_index ;
	                    node* tmp = newnode(n_no);
                    	closed.push_back(tmp);
	                    tmp->d = cur_nd->d + abs(nx - x) + abs( ny - y);
	                    // tmp->h = 0;
                        tmp->h = abs(nx - d_x) + abs( ny - d_y);  // we are using heuristic
	                    tmp->f = tmp->d + tmp->h;
	                    tmp->par = cur_nd;
	                    
	                    // this node is a condidate
	                    if(init_vertex.find( n_no )==init_vertex.end())  // node is not there
	                    {
	                    	init_vertex[n_no] = tmp;
	                    	qopen.push(tmp);
	                    	//cout << " Node Added " << nx << " " << ny << " (" << n_b << ") " << endl; 
	                    }
	                    else
				        {
				            node* oldtmp = init_vertex[n_no];
				            if(oldtmp->f > tmp->f )
				            {
				                init_vertex[n_no]=tmp;
				                qopen.push(tmp);
	                    		//cout << " Node Added " << nx << " " << ny << " (" << n_b << ") " << endl; 

				            }
				        }
	                }          
	      		}
	      	}
       	}

       	if(!path_valid)
            path_valid=true;
    }
    

    for(int x=0; x<closed.size(); x++)  // deleting the noes to prevent memory leak
        delete(closed[x]);

	return 50000;
}


long long int compute_cycle_cost(vector<long long int> &cycle , vector<vector<vector<long long int>>> &actual_path )
{
	long long int S_cost = 0;
	for(int i=0; i<No_Robots; i++)
	{
		//finding the first known robot location
		int j=0;
		while(j<cycle.size() && N_Joint_mapping[ (cycle[j]/redc_node_multiplier) ][i][0] == -1 )
			j++;

		if(j == cycle.size())  // robot is not doing anything in this cycle(task assignment)
		{
		 	// I have to take care of this part  
			continue;
		}

		int first_loc = j; // location for first know node for a given robot
		int src_loc = first_loc;
		int dest_loc;

		if(j==cycle.size()-1)  // can simplify this -->>   j = (first_loc+1)%cycle.size();
			j=0;
		else
			j=first_loc+1;

		for( ; j!=first_loc ; j = (j+1)%cycle.size() )
		{
			if(N_Joint_mapping[ (cycle[j]/redc_node_multiplier) ][i][0] == -1)
				continue;
			dest_loc = j;

			vector<long long int> tmp;
			for(int x=src_loc; x!=dest_loc; x = (x+1)%cycle.size())
			{
				tmp.push_back(cycle[x]);
			}
			tmp.push_back(cycle[dest_loc]);
			if(memo[i].find(tmp) != memo[i].end())
			{
				S_cost += memo[i][tmp];
			}
			else 
			{
				long long int  cost = find_single_robot_path_cost( cycle[src_loc] , cycle[dest_loc] , i ,cycle , src_loc , dest_loc , actual_path);
				S_cost += cost;
				memo[i][tmp] = cost;
			}
			if(S_cost > min_suffix_cost)
				return 50000;
			src_loc = dest_loc;
		}
		vector<long long int> tmp;
		for(int x=src_loc; x!=first_loc; x = (x+1)%cycle.size())
			tmp.push_back(cycle[x]);
		tmp.push_back(cycle[first_loc]);
		if(memo[i].find(tmp)!=memo[i].end())
				S_cost += memo[i][tmp];
		else
		{
			long long int cost = find_single_robot_path_cost(cycle[src_loc] , cycle[first_loc] , i ,  cycle , src_loc , first_loc , actual_path);
			S_cost += cost;
			memo[i][tmp] = cost;		
		}

		if(S_cost > min_suffix_cost)
			return 50000;

	}

    //cout << "cycle cost:" << S_cost << "min_cycle_cost " << min_suffix_cost  << endl;
	return S_cost;

}



// Generates reduced graph for a final state and finds shortest cycle 

void DFS(long long int src_no, long long int cur_no , long long int dest_no, unordered_set<long long int> visited, vector<long long int> cycle , int degen)
{
	for(unordered_map<long long int,long long int>::iterator i= Redc_E[cur_no].begin() ; i!= Redc_E[cur_no].end(); i++ )
	{
		if(i->first == dest_no)
		{
			// for(int k=0; k<cycle.size(); k++)
			Total_cycles++;	
			long long int c = 50000;
			vector<vector<vector<long long int>>> actual_path(No_Robots);
			
			//cout << "cycle is " ;
            for(int x=0; x < cycle.size(); x++)
            {
                long long int xxs = cycle[x]/1000;
                int xxb = cycle[x]%1000;

                //for(int y=0; y<No_Robots;y++);
                    //cout << "(" << N_Joint_mapping[xxs][y][0] << "," << N_Joint_mapping[xxs][y][1] << ")";
                //cout << xxb << " --> ";

            }
            //cout << endl;

            c = compute_cycle_cost(cycle , actual_path);
            

			if(c < min_suffix_cost)
			{
				pseudo_suffix_cycle = actual_path;  
				min_suffix_cycles.clear();
				min_suffix_cycles.push_back(cycle);
				min_suffix_cost = c;
			}
			else if(c == min_suffix_cost)
            {
				min_suffix_cycles.push_back(cycle);
                //  cout << "cycle is " ;
                // for(int x=0; x < cycle.size(); x++)
                // {
                //     long long int xxs = cycle[x]/1000;
                //     int xxb = cycle[x]%1000;

                //     for(int y=0; y<No_Robots;y++)
                //         cout << "(" << N_Joint_mapping[xxs][y][0] << "," << N_Joint_mapping[xxs][y][1] << ")";
                //     cout << xxb << " --> ";

                // }
                // cout << endl;

            }
			continue;
		}
        if(visited.find(i->first) == visited.end() && (i->first % 1000) != (dest_no % 1000) )
        {
            bool flag = true;
            for(unordered_set<int>::iterator x = dest.begin(); x != dest.end() ; x++ )
                if(*x == (i->first % 1000))
                    flag = false;
            if(!flag)
                continue;

            unordered_set<long long int> temp = visited;
            temp.insert(i->first);
            cycle.push_back(i->first);
            DFS(src_no , i->first , dest_no, temp ,cycle, 1 );
            cycle.pop_back();
        }
	}

}



int main(int args,char **argv)
{
    
    t = clock();
    initializegrid(argv[1]);
    generate_final_states();
    generate_abstract_reduced_graph();
    int count = 0;
    //cout << "Final States " << Final_states.size() << endl;
    //cout << "min_suffix_cost: " << min_suffix_cost << endl;
    //cout << "dest_size " << dest.size() << endl;
    for(unordered_set<int>::iterator x = dest.begin(); x != dest.end() ; x++ )
        cout << *x << endl;
   	for(unordered_set<long long int>::iterator i=Final_states.begin(); i!=Final_states.end() ; i++)
    {    
        if(Redc_N.find(*i) != Redc_N.end())
        {
            //cout << "===================================================================" << count << endl;
        	long long int src_no = *i;
        	long long int dest_no = *i;
        	unordered_set<long long int> visited;
        	visited.insert(src_no);
        	vector<long long int> cycle;
			cycle.push_back(src_no); 	
        	DFS(src_no , src_no,  dest_no, visited  , cycle , 0 );
        	count++;
        }
    }

    // cout << "Total Final States Explored: " << count << endl;
    // cout << "Total Cycles Found " << Total_cycles << endl;
    cout << "Number of Robots: " << No_Robots << endl;	
    cout << "Suffix Cost: " << min_suffix_cost << endl; 
    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds
 	cout << "Time: " << time_taken << endl;
    
    cout << "Min_Suffix Cycle is: ";
    for(int i=0; i<min_suffix_cycles[0].size() ;i++)
    {	
	cout << "< ";
    	for(int r=0; r<No_Robots;r++)
	{
		int x = N_Joint_mapping[ min_suffix_cycles[0][i] / redc_node_multiplier ][r][0];
		int y = N_Joint_mapping[ min_suffix_cycles[0][i] / redc_node_multiplier ][r][1];
		if(x == -1)
			cout << "(*" << " "<< "*)" << " ";
		else
			cout << "(" << x << " " << y << ") ";	
	}
    	cout << "> (" << min_suffix_cycles[0][i] % redc_node_multiplier << ")"  << " --> ";
    }
    cout << endl << endl;	
    for(int r=0; r<No_Robots;r++)	
    {	
	cout << "Robot " << r+1 << " Trajectory: ";
     	for(int i=0; i<min_suffix_cycles[0].size() ;i++)
    	{	
		int x = N_Joint_mapping[ min_suffix_cycles[0][i] / redc_node_multiplier ][r][0];
		int y = N_Joint_mapping[ min_suffix_cycles[0][i] / redc_node_multiplier ][r][1];
		if(x == -1)
			cout << "(*" << " "<< "*)";
		else
			cout << "(" << x << " " << y << ")";
		cout << " --> ";	
	}
	int x = N_Joint_mapping[ min_suffix_cycles[0][0] / redc_node_multiplier ][r][0];
	int y = N_Joint_mapping[ min_suffix_cycles[0][0] / redc_node_multiplier ][r][1];
	if(x == -1)
		cout << "(*" << " "<< "*)" << " ";
	else
		cout << "(" << x << " " << y << ") ";	

	
	cout << endl;
    	
    }	
    cout << endl;
    cout<<"............................................................................." << endl;

    

    return 0;
}
