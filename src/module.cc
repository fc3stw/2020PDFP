#include "module.h"

gGrid::gGrid(int layer, int row, int col, int supply):
	_pos(layer, row, col),
	_supply(supply),
	_demand(0)
{}

Pos gGrid::get_pos() const {return _pos;}
int gGrid::get_demand() const {return _demand;}
void gGrid::add_supply(int val){_supply += val;}
bool gGrid::add_demand(int val){
    int new_demand = _demand + val;
    if(new_demand > _supply || new_demand < 0) return false;
    _demand += val;
    return true;
}