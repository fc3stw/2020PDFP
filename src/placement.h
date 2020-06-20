#include "module.h"

using namespace std;

class Placement{
Design& _design;
public:
	/*=====construct=====*/
	Placement(Design& p_Design): _design(p_Design){}
	/*=====function=====*/
	void set_HPWL_for_nets();
	void set_HPWL_for_cells();
};
