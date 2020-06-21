#include "module.h"

using namespace std;

class Placement{
Design& _design;
Chip& _chip;
map<int,int> cell_cost_list; //cost, ID
public:
	/*=====construct=====*/
	Placement(Design& p_Design, Chip p_Chip): _design(p_Design), _chip(p_Chip){}
	/*=====function=====*/
	void set_HPWL_for_nets();
	void set_HPWL_for_cells();
	void move_cell();
	void minus_demand(CellInstance* cell);
	bool is_position_valid(CellInstance* cell, int row, int column);
	bool another_move(CellInstance* cell, int row, int column);
	bool update_demand(CellInstance* cell, int row, int column);//If yes update demand,else restore original demand 
};
