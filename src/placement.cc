#include "placement.h"
#include <climits>

void Placement::set_HPWL_for_nets()
{
	/*==set_HPWL_for_nets==*/
    for(int net_id = 0; net_id < _design.get_num_nets(); net_id++){
        Net* net = _design.get_net_by_id(net_id);
        int maxLayer = 0;
	    int minLayer = INT_MAX;
	    int maxRow = 0;
	    int minRow = INT_MAX;
	    int maxCol = 0;
	    int minCol = INT_MAX;
        int layer_id = 0;
        int row = 0;
        int col = 0;
        for(int pin_id = 0 ; pin_id < net -> get_num_pins(); pin_id++){
            Pin* pin = net -> get_pin(pin_id); 
            tie(layer_id, row, col) = pin -> get_pos();
            maxLayer = max(layer_id, maxLayer);
			minLayer = min(layer_id, minLayer);
			maxRow   = max(row, maxRow);
			minRow   = min(row, minRow);
			maxCol   = max(col, maxCol);
			minCol   = min(col, minCol); 
        }
        net -> set_hpwl(maxLayer-minLayer+maxRow-minRow+maxCol-minCol);
    }
}
void Placement::set_HPWL_for_cells()
{
    cell_cost_list.erase(cell_cost_list.begin(), cell_cost_list.end());

	/*==set_HPWL_for_cells==*/
    for(int cell_id = 0; cell_id < _design.get_num_cells(); cell_id++){
        CellInstance* cell = _design.get_cell_by_id(cell_id);
        set<Net*> nets_on_cell;
        for(int pin_id = 0 ; pin_id < cell -> get_num_pins(); pin_id++){
           Pin* pin = cell -> get_pin(pin_id);
           for(int net_id = 0; net_id < pin -> get_num_nets(); net_id++){
                nets_on_cell.insert(pin -> get_net(net_id));
           }  
        }
        int cellinstance_HPWL = 0;
        for(Net *net : nets_on_cell){
           cellinstance_HPWL += net -> get_hpwl(); 
        }
        cell -> set_hpwl_cost(cellinstance_HPWL);
        cell_cost_list.insert(cellinstance_HPWL, cell_id);
    }
}
void Placement::move_cell()
{
    map<int,int>::iterator iter;
    for (iter = cell_cost_list.begin(); iter != cell_cost_list.end(); iter++)
    {
       int id = iter->second;
       CellInstance* current_cell = _design->get_cell_by_id(id);
       minus_demand(current_cell);
       
    }

}
void Placement::minus_demand(CellInstance* cell)
{
    int blkgs_num =  cell->get_num_blkgs();
    for (int i = 0; i < blkgs_num; ++i)
    {
        Blockage* blk = cell->get_blkg(i);
        Pos3d position = blk->get_pos();
        gGrid grid = _chip.get_grid(position);
        if (!grid.add_demand(-(blk->get_demand())))
        {
            cerr<<"ERROR IN GRID DEMAND"<<endl;
        }
    }
    return;
}

void Placement::updata_demand(CellInstance* cell, int row, int column)
{

} 

// int main(){
//     Placement* pPlacement = new Placement(design);
// }