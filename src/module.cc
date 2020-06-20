#include "module.h"


// class gGrid

gGrid::gGrid(int layer, int row, int col, int supply):
	_pos(layer, row, col),
	_supply(supply),
	_demand(0)
{}

Pos3d gGrid::get_pos() const {return _pos;}

int gGrid::get_demand() const {return _demand;}

int gGrid::get_remain_demand() const {return _supply - _demand;}

bool gGrid::add_demand(int val)
{
    int new_demand = _demand + val;
    if(new_demand > _supply || new_demand < 0) return false;
    _demand += val;
    return true;
}


// class Layer

Layer::Layer(string name, int id, Direction dir, int supply, int num_rows, int num_cols):
    _name(name),
    _id(id),
    _dir(dir),
    _supply(supply)
{
    for(int r = 0; r<num_rows; r++){
        vector<gGrid> grid_row;
        for(int c = 0; c<num_cols; c++){
            grid_row.push_back(gGrid(id, r, c, supply));
        }
        _gGrid2d.push_back(grid_row);
    }
}

Direction Layer::get_dir() const {return _dir;}

gGrid& Layer::get_grid(int row, int col) {return _gGrid2d[row][col];}


// class Chip

Chip::Chip(int row_begin, int col_begin, int row_end, int col_end):
	_row_begin_idx(row_begin),
	_col_begin_idx(col_begin),
	_num_rows(row_end - row_begin + 1),
	_num_cols(col_end - col_begin + 1)
{}

int Chip::get_num_rows() const {return _num_rows;}

int Chip::get_num_cols() const {return _num_cols;}

void Chip::add_layer(string name, int supply)
{
    int layer_id = _layer_list.size();
    _layer_list.push_back(
        Layer(name, layer_id, get_layer_dir(layer_id), supply, _num_rows, _num_cols));
}


// class Pin

Pin::Pin(string name, int id, CellInstance *cell, int layer_id):
	_name(name),
	_id(id),
	_cell(cell),
	_layer_id(layer_id)
{}

Pos3d Pin::get_pos() const {return Pos3d(_layer_id, _cell->get_row(), _cell->get_col());}

void Pin::add_net(Net *net) {_net_list.push_back(net);}


// class Blockage

Blockage::Blockage(string name, int id, CellInstance *cell, int layer_id, int demand):
	_name(name),
	_id(id),
	_cell(cell),
	_layer_id(layer_id),
	_demand(demand)
{}

Pos3d Blockage::get_pos() const {return Pos3d(_layer_id, _cell->get_row(), _cell->get_col());}

int Blockage::get_demand() const {return _demand;}


// class MasterCell

MasterCell::MasterCell(string name, int id):
	_name(name),
	_id(id)
{
    _pin_list.clear();
    _blkg_list.clear();
}

string MasterCell::get_name() const {return _name;}

int MasterCell::get_id() const {return _id;}

vector<Pin>& MasterCell::get_pins() {return _pin_list;}

vector<Blockage>& MasterCell::get_blkgs() {return _blkg_list;}

void MasterCell::set_id(int val) {_id = val;}

void MasterCell::add_pin(string name, int id, int layer_id)
{
    _pin_list.push_back(
        Pin(name, id, nullptr, layer_id));
}

void MasterCell::add_blkg(string name, int id, int layer_id, int demand)
{
    _blkg_list.push_back(
        Blockage(name, id, nullptr, layer_id, demand));
}


// class CellInstance

CellInstance::CellInstance(string name, int id, MasterCell &master_cell, bool fixed):
	_name(name),
	_id(id),
	_master_cell_id(master_cell.get_id()),
	_fixed(fixed)
{
    _pin_list.clear();
    vector<Pin>& pins = master_cell.get_pins();
    for(Pin &pin : pins){
        Pin *new_pin = new Pin(pin);
        _pin_list.push_back(new_pin);
    }

    _blkg_list.clear();
    vector<Blockage>& blkgs = master_cell.get_blkgs();
    for(Blockage &blkg : blkgs){
        Blockage *new_blkg = new Blockage(blkg);
        _blkg_list.push_back(new_blkg);
    }
}

string CellInstance::get_name() const {return _name;}

int CellInstance::get_id() const {return _id;}

bool CellInstance::is_fixed() const {return _fixed;}

int CellInstance::get_row() const {return _row;}

int CellInstance::get_col() const {return _col;}

Pos2d CellInstance::get_pos() const {return Pos2d(_row, _col);}

void CellInstance::set_id(int val) {_id = val;}

void CellInstance::set_pos(Pos2d pos)
{
    if(is_fixed()) return;
    _row = get<0>(pos);
    _col = get<1>(pos);
}

void CellInstance::set_pos(int row, int col)
{
    if(is_fixed()) return;
    _row = row;
    _col = col;
}


// class Net

Net::Net(string name, int id, int min_layer):
	_name(name),
	_id(id),
	_min_layer(min_layer)
{}

string Net::get_name() const {return _name;}

int Net::get_id() const {return _id;}

int Net::get_min_layer() const {return _min_layer;}

void Net::set_id(int val) {_id = val;}

void Net::add_pin(Pin *pin)
{
    _pins.push_back(pin);
    pin->add_net(this);
}

void Net::add_route(Pos3d pos) {_route.push_back(pos);}


// class CellLibrary

CellLibrary::CellLibrary(){}

MasterCell CellLibrary::get_master_cell_by_name(string name, bool &success) const
{
    int id = get_id_by_name(name);
    if(id==-1){
        success = false;
        return MasterCell(0,0);
    }
    return _master_cell_list.at(id);
}

void CellLibrary::add_master_cell(MasterCell ms)
{
    int id = _master_cell_list.size();
    ms.set_id(id);
    _master_cell_list.push_back(ms);
    _name2id[ms.get_name()] = id;
}


// class ExtraDemand

ExtraDemand::ExtraDemand(){}

int ExtraDemand::get_same_demand(int ms1, int ms2, int layer) const
{
    ExtraDemandCondition cond = get_cond(ms1, ms2, layer);
    if(same_grid.find(cond)==same_grid.end()) return 0;
    return same_grid.at(cond);
}

int ExtraDemand::get_adj_demand(int ms1, int ms2, int layer) const
{
    ExtraDemandCondition cond = get_cond(ms1, ms2, layer);
    if(adj_hor_grid.find(cond)==adj_hor_grid.end()) return 0;
    return adj_hor_grid.at(cond);
}

void ExtraDemand::add_same_demand(int ms1, int ms2, int layer, int demand)
{
    ExtraDemandCondition cond = get_cond(ms1, ms2, layer);
    same_grid[cond] = demand;
}

void ExtraDemand::add_adj_demand(int ms1, int ms2, int layer, int demand)
{
    ExtraDemandCondition cond = get_cond(ms1, ms2, layer);
    adj_hor_grid[cond] = demand;
}


// class Design

Design::Design(int max_cell_move, int num_cells, int num_nets) :
	_max_cell_move(max_cell_move)
{
    _cell_list.clear();
    _net_list.clear();
    _moved_cell_id.clear();
    _cell_name2id.clear();
    _net_name2id.clear();
}

CellInstance* Design::get_cell_by_id(int id) const {return _cell_list.at(id);}

Net* Design::get_net_by_id(int id) const {return _net_list.at(id);}

CellInstance* Design::get_cell_by_name(string name) const {return _cell_list.at(_cell_name2id.at(name));}

Net* Design::get_net_by_name(string name) const {return _net_list.at(_net_name2id.at(name));}

void Design::add_cell(CellInstance *cell){
    int id = _cell_list.size();
    cell->set_id(id);
    _cell_list.push_back(cell);
    _cell_name2id[cell->get_name()] = id;
}

void Design::add_net(Net *net){
    int id = _net_list.size();
    net->set_id(id);
    _net_list.push_back(net);
    _net_name2id[net->get_name()] = id;
}
