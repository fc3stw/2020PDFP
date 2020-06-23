#include <iostream>
#include <fstream>
#include <vector>
#include "placement.h"


using namespace std;

int main(int argc, char** argv){
	fstream input, output;
	if (argc == 3) {
        input.open(argv[1], ios::in);
        output.open(argv[2], ios::out);
        if (!input) {
            cerr << "Cannot open the input file \"" << argv[1]
                 << "\". The program will be terminated..." << endl;
            exit(1);
        }
        if (!output) {
            cerr << "Cannot open the output file \"" << argv[2]
                 << "\". The program will be terminated..." << endl;
            exit(1);
        }
    }
    else {
        cerr << "Usage: ./cell_move_router <input.txt> <output.txt> " << endl;
        exit(1);
    }

    
    CellLibrary celllibrary;
    Design design;
    Chip chip;

    string str;  
 	int max_cell_move;
 	int row_begin, col_begin, row_end, col_end; 
    while(input >> str){
    	if (str == "MaxCellMove"){
	    	// cout<<"MaxCellMove"<<endl;
			input >> str;
	    	max_cell_move = atoi(str.c_str());
	    	design.set_max_cell_move(max_cell_move);
	    }

	    if (str == "GGridBoundaryIdx")
	    {
	    	// cout<<"GGridBoundaryIdx"<<endl;
			input >> str;
	    	row_begin = atoi(str.c_str());
	    	input >> str;
	    	col_begin = atoi(str.c_str());
	    	input >> str;
	    	row_end = atoi(str.c_str());
	    	input >> str;
	    	col_end = atoi(str.c_str());
 			chip.set_layer_info(row_begin, col_begin, row_end, col_end);
	    }

	    if (str == "NumLayer")
	    {
	    	// cout<<"NumLayer"<<endl;
			input >> str;
	    	int layer_cnt = atoi(str.c_str());
	    	for (int i = 0; i < layer_cnt; ++i)
	    	{
	    		input >> str;
	    		input >> str;
	    		string layer_name = str;
	    		input >> str;
	    		input >> str;
	    		input >> str;
	    		int default_demand = atoi(str.c_str());
	    		chip.add_layer(layer_name, default_demand);
	    	}
	    }

	    if (str == "NumNonDefaultSupplyGGrid")
	    {
	    	// cout<<"NumNonDefaultSupplyGGrid"<<endl;
			input >> str;
	    	int non_default_cnt = atoi(str.c_str());
	    	for (int i = 0; i < non_default_cnt; ++i)
	    	{
	    		input >> str;
	    		int row_idx = atoi(str.c_str()) - chip.get_row_offset();
	    		input >> str;
	    		int col_idx = atoi(str.c_str()) - chip.get_col_offset();
	    		input >> str;
	    		int lay_idx = atoi(str.c_str()) - 1;
	    		input >> str;
	    		int value = atoi(str.c_str());
	    		gGrid &g = chip.get_grid(Pos3d(lay_idx, row_idx, col_idx));
	    		g.add_supply(value);
	    	}
	    }

	    if (str == "NumMasterCell")
	    {
	    	// cout<<"NumMasterCell"<<endl;
			input >> str;
	    	int master_cell_cnt = atoi(str.c_str());
	    	for (int i = 0; i < master_cell_cnt; ++i)
	    	{
	    		input >> str;
	    		input >> str;
	    		string mc_name = str;
	    		int pin_cnt, blk_cnt;
	    		input >> str;
	    		pin_cnt = atoi(str.c_str());
	    		input >> str;
	    		blk_cnt = atoi(str.c_str());
	    		MasterCell master_cell(mc_name, i);
	    		for (int j = 0; j < pin_cnt; ++j)
	    		{
	    			input >> str;
	    			input >> str;
	    			string pin_name = str;
	    			input >> str;
	    			int layer_num = chip.get_layer_by_name(str);
	    			master_cell.add_pin(pin_name, j, layer_num);
	    		}
	    		for (int k = 0; k < blk_cnt; ++k)
	    		{
	    			input >> str;
	    			input >> str;
	    			string blk_name = str;
	    			input >> str;
	    			int layer_num = chip.get_layer_by_name(str);
	    			input >> str;
	    			int dem = atoi(str.c_str());
	    			master_cell.add_blkg(blk_name, k, layer_num, dem);
	    		}
	    		celllibrary.add_master_cell(master_cell);
	    	}

	    }

	    if (str == "NumCellInst")
	    {
	    	// cout<<"NumCellInst"<<endl;
			input >> str;
	    	int cell_num = atoi(str.c_str());
	    	for (int j = 0; j < cell_num; ++j)
	    	{
	    		input >> str;
	    		input >> str;
	    		string cell_name = str;
	    		input >> str;
	    		bool success = true;
	    		MasterCell ms = celllibrary.get_master_cell_by_name(str, success);
	    		if(success == false) cerr<<"SOMETHING WRONG"<<endl;
	    		input >> str;
	    		int row_idx = atoi(str.c_str()) - chip.get_row_offset();
	    		input >> str;
	    		int col_idx = atoi(str.c_str()) - chip.get_col_offset();
	    		input >> str;
	    		bool fixed;
	    		if (str == "Fixed") fixed = true;
	    		else fixed = false;
	    		CellInstance* cell = new CellInstance(cell_name, j, ms, fixed);
	    		cell->set_pos(row_idx, col_idx);
	    		design.add_cell(cell);
	    	}
	    }

	    if (str == "NumNets")
	    {
	    	// cout<<"NumNets"<<endl;
			input >> str;
	    	int net_num = atoi(str.c_str());
	    	for (int i = 0; i < net_num; ++i)
	    	{
	    		input >> str;
	    		input >> str;
	    		string net_name = str;
	    		input >> str;
	    		int pin_num = atoi(str.c_str());
	    		input >> str;
	    		int layer_num = chip.get_layer_by_name(str);
	    		Net* net = new Net(net_name, i, layer_num);
	    		for (int j = 0; j < pin_num; ++j)
	    		{
	    			input >> str;
		    		input >> str;
		    		int pos = str.find('/');
		    		if (pos == string::npos) cerr<<"ERROR IN FINDIND /"<<endl;
		    		else{
		    			string cellinst =  str.substr(0, pos);
		    			string master_pin = str.substr(pos+1, str.size());
		    			CellInstance* pin_cell =  design.get_cell_by_name(cellinst);
		    			int mc_id = pin_cell->get_master_cell_id();
		    			MasterCell mc =  celllibrary.get_master_cell_by_id(mc_id);
		    			int pin_id = mc.get_pin_by_name(master_pin);
		    			Pin* pin = pin_cell->get_pin(pin_id);
		    			net->add_pin(pin);
		    		}
	
	    		}
	    		design.add_net(net);
	    	}
	    }
	    // if (str == "NumRoutes")
	    // {
	    // 	input >> str;
	    // 	int route_num = atoi(str.c_str());
	    // 	for (int i = 0; i < route_num; ++i)
	    // 	{
	    // 		input >> str;
	    // 		int srow_idx =  atoi(str.c_str());
	    // 		input >> str;
	    // 		int scol_idx = atoi(str.c_str());
	    // 		input >> str;
	    // 		int slay_idx = atoi(str.c_str());
	    // 		input >> str;
	    // 		int erow_idx = atoi(str.c_str());
	    // 		input >> str;
	    // 		int ecol_idx = atoi(str.c_str());
	    // 		input >> str;
	    // 		int elay_idx = atoi(str.c_str());
	    // 		input >> str;
	    // 		string name = str;
	    // 		Net* net = design.get_net_by_name(name);
	    // 	}
	    	
	    // }
    }
	design.print_summary(false);
	chip.print_summary();
	// exit(1);

	// initialize placer and router
	Placement placer(design, chip);
	vector<Net*> net_list;
	int num_nets = design.get_num_nets();
	for(int net_id = 0; net_id < num_nets; net_id++){
		net_list.push_back(
			design.get_net_by_id(net_id));
	}
	Router router(chip, net_list);

	placer.reset_demand();
	placer.set_HPWL_for_nets();
	int hpwl;
	int wl = -1;
	int iter = 0;
	while(iter < 10){
		placer.reset_demand();
		placer.set_HPWL_for_nets();
		
		hpwl = router.get_total_hpwl();
		router.routing_flow();
		int new_wl = router.get_total_wl();
		if(wl==-1){
			wl = new_wl;
			cout<<"Initial wire length = "<<wl<<endl;
			// break;
		}
		else if(new_wl < wl){
			wl = new_wl;
			cout<<"wire length decreases"<<endl;
		}
		cout<<"Total HPWL = "<<hpwl<<endl;
		cout<<"wire length = "<<new_wl<<endl;

		placer.set_HPWL_for_cells();
		bool move_success = placer.move_cell();
		if(!move_success){
			break;
		}

		iter++;
	}
	cout<<"Final wire length = "<<wl<<endl;

    return 0;
}