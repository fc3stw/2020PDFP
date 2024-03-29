#include "placement.h"
#include <climits>
#include <iostream>
#include <algorithm>
#include <cmath>

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
    //cout<<"set_HPWL_for_cells"<<endl;
    cell_cost_list.clear();

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
            // cellinstance_HPWL += net -> get_route().size(); 
        }
        cell -> set_hpwl_cost(cellinstance_HPWL);
        cell_cost_list.push_back(pair<int, int>(cellinstance_HPWL, cell_id));
    }
    sort(cell_cost_list.begin(), cell_cost_list.end());
    int totalcost = 0;
    for (auto pos: cell_cost_list)
    {
      totalcost += pos.first;
      //cout<<"Cost: "<<pos.first<<", Cell ID: "<<pos.second<<endl;
    }
    // cout<<"TOTAL: "<<totalcost<<endl;
}
bool Placement::move_cell()
{
    // random_shuffle(cell_cost_list.begin(), cell_cost_list.end());
    vector<pair<int, int> >::reverse_iterator iter;
    for (iter = cell_cost_list.rbegin(); iter != cell_cost_list.rend(); iter++)
    // vector<pair<int, int> >::iterator iter;
    // for (iter = cell_cost_list.begin(); iter != cell_cost_list.end(); iter++)
    {
       int id = iter->second;//cell_id
       CellInstance* current_cell = _design.get_cell_by_id(id);
       //cout<<"CELL ID: "<<current_cell->get_id()<<endl;
       if(current_cell->is_fixed()) continue;
       if(_design.cell_is_moved(current_cell->get_id())) continue;
       minus_demand(current_cell);
       //Process one cell new position use zero force
       int layer_id = 0;
       int row  = 0;
       int col = 0; 
       //int layer_id_sum = 0;
       double row_sum = 0;
       double col_sum = 0;
       int pin_count = 0;
       for(int pin_id = 0 ; pin_id < current_cell -> get_num_pins(); pin_id++){//The pin cell connect  
           Pin* pin = current_cell -> get_pin(pin_id);//get pin for current cell
           for(int net_id = 0; net_id < pin -> get_num_nets(); net_id++){//get net this pin connect
                Net *net = pin -> get_net(net_id);//get net
                int num_pins = net -> get_num_pins();//This net connect which pins
                int num_pins_minus1 = num_pins - 1;
                for(int pin_id_other = 0; pin_id_other < num_pins; pin_id_other++){
                    Pin* pin_connected =  net -> get_pin(pin_id_other);
                    if(pin_connected != pin){ //Don't calculate current cell  
                        tie(layer_id, row, col) = pin_connected -> get_pos();
                        //layer_id_sum += num_pins_minus1*layer_id;
                        //cout<<"ROW: "<<row<<", COL: "<<col<<endl;
                        row_sum += row;
                        col_sum += col; 
                    }
                }
                //cout<<"num_pins_minus1: "<<num_pins_minus1<<", row_sum: "<<row_sum<<", col_sum: "<<col_sum<<endl;
                pin_count += num_pins_minus1;
           }              
       }
       //layer = layer_id_sum / pin_count;
       row = round(row_sum / pin_count);
       col = round(col_sum / pin_count);
       //cout<<"ROW: "<<row<<", COL: "<<col<<endl;
       if(update_demand(current_cell, row, col)){
           cout<<"SUCCESSFULLY MOVE CURRENT"<<endl;
           _design.move_cell(current_cell->get_id());
           return true;
       } 
       if(another_move(current_cell, row, col)){
           cout<<"SUCCESSFULLY MOVE EIGHT"<<endl;
           _design.move_cell(current_cell->get_id());
           return true;
       } 
       else{
          restore_demand(current_cell);
          continue;
       } 
    }
    return false;
}
void Placement::minus_demand(CellInstance* cell)
{
  //cout<<"minus_demand"<<endl;
    int blkgs_num =  cell->get_num_blkgs();
    //cout<<"blkgs_num: "<<blkgs_num<<endl;
    for (int i = 0; i < blkgs_num; ++i)
    {
        Blockage* blk = cell->get_blkg(i);
        //cout<<"BLK NAME: "<<blk->get_name()<<endl;
        Pos3d position = blk->get_pos();
        // int layer_id, row, col;
        // tie(layer_id, row, col) = blk->get_pos();
        // cout<<"Layer: "<<layer_id<<", row: "<<row<<", col: "<<col<<endl;
        gGrid &grid = _chip.get_grid(position);
        if (!grid.add_demand(-(blk->get_demand())))
        {
            cerr<<"ERROR IN GRID DEMAND"<<endl;
        }
        // tie(layer_id, row, col) = blk->get_pos();
        // cout<<"Layer: "<<layer_id<<", row: "<<row<<", col: "<<col<<endl;
    }
    return;
}

void Placement::restore_demand(CellInstance* cell)
{
    int blkgs_num =  cell->get_num_blkgs();
    for (int i = 0; i < blkgs_num; ++i)
    {
        Blockage* blk = cell->get_blkg(i);
        Pos3d position = blk->get_pos();
        gGrid grid = _chip.get_grid(position);
        if (!grid.add_demand((blk->get_demand())))
        {
            cerr<<"ERROR IN RESTORE GRID DEMAND"<<endl;
        }
    }
    return;
}

bool Placement::another_move(CellInstance* cell, int row, int column)
{
    int max_cell_supply_sum = 0;
    int row_new = -1;
    int col_new = -1;
    int layer_id = 0;
    int row_old = 0;
    int col_old = 0;
    for(int c = column -1; c <= column + 1; c++ ){
        for(int r = row - 1;r <= row + 1; r++){
            int blksgsNum = cell -> get_num_blkgs();
            int cell_supply_sum = 0;
            for(int blk_id = 0; blk_id < blksgsNum ; blk_id++){
                Blockage* blk = cell->get_blkg(blk_id);
                tie(layer_id, row_old, col_old) = blk-> get_pos();//old row and old column, I dont care
                int current_blk_demand = blk -> get_demand();
                gGrid grid = _chip.get_grid(Pos3d(layer_id, r, c));
                if(!grid.add_demand(current_blk_demand)){//Add demand, if the grid is full, we should restore all operate
                    cell_supply_sum = INT_MIN;//The row,col is eliminate
                    for(int i = 0; i < blk_id; i++){
                        Blockage* blk = cell->get_blkg(i);
                        tie(layer_id, row_old, col_old) = blk-> get_pos();//old row and old column, I dont care
                        gGrid grid = _chip.get_grid(Pos3d(layer_id, r, c));
                        if (!grid.add_demand(-(blk->get_demand()))){cerr<<"ERROR IN RESTORE DEMAND"<<endl;}                        
                    }
                    break; 
                }
                cell_supply_sum += grid.get_remain_supply();
                if(blk_id == blksgsNum - 1){ //finish a grid should restore, too
                        for(int blk_id = 0; blk_id < blksgsNum ; blk_id++){
                            Blockage* blk = cell->get_blkg(blk_id);
                            tie(layer_id, row_old, col_old) = blk-> get_pos();//old row and old column, I dont care
                            gGrid grid = _chip.get_grid(Pos3d(layer_id, r, c));
                        if (!grid.add_demand(-(blk->get_demand()))){cerr<<"ERROR IN RESTORE DEMAND"<<endl;}                        
                        }
                }
            }
            if(cell_supply_sum > max_cell_supply_sum){
                max_cell_supply_sum = cell_supply_sum;
                row_new = r;
                col_new = c;
            }                               
        }
    }
    if(row_new == -1)
        return false;
    else{
        if(update_demand(cell, row_new, col_new))
            return true;
        else{
            cerr<<"There is something wrong!!!"<<endl;
            return false;
        }
    } 
}


bool Placement::update_demand(CellInstance* cell, int row, int column)
{
  vector<gGrid*> history_grid;
  vector<int> history_gain;
  history_grid.clear();
  history_gain.clear();
  for (int blk_id = 0; blk_id < cell -> get_num_blkgs(); blk_id++){ //All blk cell connected
    Blockage* blk = cell -> get_blkg(blk_id);//get block
    //cout<<"BLK NAME: "<<blk->get_name()<<endl;
    Pos3d position = blk->get_pos();//find where is the blk
    // int layer_id, row, col;
        // tie(layer_id, row, col) = blk->get_pos();
        // cout<<"Layer: "<<layer_id<<", row: "<<row<<", col: "<<col<<endl;
    gGrid &grid = _chip.get_grid(position);//go to this grid 
    if(!grid.add_demand(blk -> get_demand())){
      for (int i = 0; i < history_gain.size(); ++i)
      {
        history_grid[i]->add_demand(-history_gain[i]);
      }
      return false;
    }//update demand in this grid
    history_grid.push_back(&grid);
    history_gain.push_back(blk -> get_demand());
    // cout<<"Layer: "<<layer_id<<", row: "<<row<<", col: "<<col<<endl;
  }
  // cout<<"BEFORE: "<<endl;
  // cell->print();
  cell -> set_pos(row, column);//update cell location
  // cout<<"AFTER: "<<endl;
  // cell->print();
  return true;
} 

void Placement::reset_demand()
{
  int num_layers = _chip.get_num_layers();
  int num_rows = _chip.get_num_rows();
  int num_cols = _chip.get_num_cols();
  for(int layer = 0; layer < num_layers; layer++){
      for(int row = 0; row < num_rows; row++){
          for(int col = 0; col < num_cols; col++){
              _chip.get_grid(Pos3d(layer, row, col)).clear_demand();
          }
      }
  }

  for(int cell_id = 0; cell_id < _design.get_num_cells(); cell_id++){
    CellInstance* cell = _design.get_cell_by_id(cell_id);
    for (int blk_id = 0; blk_id < cell -> get_num_blkgs(); blk_id++){
      Blockage* blk = cell -> get_blkg(blk_id);//get block
      Pos3d position = blk->get_pos();//find where is the blk
      gGrid &grid = _chip.get_grid(position);//go to this grid 
      if(!grid.add_demand(blk -> get_demand())) cerr<<"ERROR IN GRID DEMAND"<<endl;
    }
  }
  return;
}