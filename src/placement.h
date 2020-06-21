#include "module.h"

using namespace std;

class Placement{
Design& _design;
map<int,int> cell_cost_list; //cost, ID
public:
	/*=====construct=====*/
	Placement(Design& p_Design): _design(p_Design){}
	/*=====function=====*/
	void set_HPWL_for_nets();
	void set_HPWL_for_cells();
	void move_cell();
};
