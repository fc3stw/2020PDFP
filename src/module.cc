#include "module.h"
#include <iostream>

// class gGrid

string get_pos_str(Pos3d pos)
{
    string pos_str;
	int z, x ,y;
    tie(z, x, y) = pos;
    pos_str = "(" + to_string(z) + "," + to_string(x) + "," + to_string(y) + ")";
	return pos_str;
}

gGrid::gGrid(int layer, int row, int col, int supply):
	_pos(layer, row, col),
	_supply(supply),
	_demand(0)
{}

Pos3d gGrid::get_pos() const {return _pos;}

int gGrid::get_demand() const {return _demand;}

void gGrid::clear_demand() {_demand = 0;}

int gGrid::get_remain_supply() const {return _supply - _demand;}

set<int>& gGrid::get_nets() {return _nets;}

void gGrid::add_supply(int val) {_supply += val;}

bool gGrid::add_demand(int val)
{
    int new_demand = _demand + val;
    if(new_demand > _supply || new_demand < 0) return false;
    _demand += val;
    return true;
}

void gGrid::add_net(int net_id) {_nets.insert(net_id);}


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

string Layer::get_name() const {return _name;}

Direction Layer::get_dir() const {return _dir;}

int Layer::get_default_supply() const {return _supply;}

gGrid& Layer::get_grid(int row, int col) {return _gGrid2d[row][col];}


// class Chip

Chip::Chip():
	_row_begin_idx(0),
	_col_begin_idx(0),
	_num_rows(0),
	_num_cols(0)
{}

int Chip::get_row_offset() const {return _row_begin_idx;}

int Chip::get_col_offset() const {return _col_begin_idx;}

int Chip::get_num_rows() const {return _num_rows;}

int Chip::get_num_cols() const {return _num_cols;}

int Chip::get_num_layers() const {return _layer_list.size();}

gGrid& Chip::get_grid(Pos3d pos) {return _layer_list.at(get<0>(pos)).get_grid(get<1>(pos), get<2>(pos));}

bool Chip::has_grid(Pos3d pos)
{
    int layer, row, col;
    tie(layer, row, col) = pos;
    if(layer < 0 || layer > _layer_list.size()-1) return false;
    if(row < 0 || row > _num_rows-1) return false;
    if(col < 0 || col > _num_cols-1) return false;
    return true;
}

Direction Chip::get_layer_dir(int layer_id)
{
    Direction dir;
    switch(layer_id % 2){
        case 0 :
        dir = H;
        break;
        case 1 :
        dir = V;
        break;
        default:
        dir = NoDir;
        break;
    }
    return dir;
}

int Chip::get_layer_by_name(string name) const
{
    if(_layer_name2id.find(name)==_layer_name2id.end()) return -1;
    return _layer_name2id.at(name);
}

void Chip::set_layer_info(int row_begin, int col_begin, int row_end, int col_end)
{
    _row_begin_idx = row_begin;
    _col_begin_idx = col_begin;
    _num_rows = row_end - row_begin + 1;
    _num_cols = col_end - col_begin + 1;
}

void Chip::add_layer(string name, int supply)
{
    int layer_id = _layer_list.size();
    _layer_list.push_back(
        Layer(name, layer_id, get_layer_dir(layer_id), supply, _num_rows, _num_cols));
    _layer_name2id[name] = layer_id;
}

void Chip::print_summary()
{
    cout<<"\nChip info:\n";
    cout<<"#rows: "<<get_num_rows()<<", start from "<<get_row_offset()<<"\n";
    cout<<"#cols: "<<get_num_cols()<<", start from "<<get_col_offset()<<"\n";
    cout<<"#layers: "<<get_num_layers()<<"\n";
    for(Layer &layer : _layer_list){
        cout<<"  "<<layer.get_name()<<" dir: "<<layer.get_dir()<<" default supply: "<<layer.get_default_supply()<<"\n";
    }
}


// class Pin

Pin::Pin(string name, int id, CellInstance *cell, int layer_id):
	_name(name),
	_id(id),
	_cell(cell),
	_layer_id(layer_id)
{}

string Pin::get_name() const {return _name;}

int Pin::get_id() const {return _id;}

Pos3d Pin::get_pos() const {return Pos3d(_layer_id, _cell->get_row(), _cell->get_col());}

CellInstance* Pin::get_cell() const {return _cell;}

int Pin::get_num_nets() const {return _net_list.size();}

Net* Pin::get_net(int idx) {return _net_list.at(idx);}

void Pin::add_net(Net *net) {_net_list.push_back(net);}


// class Blockage

Blockage::Blockage(string name, int id, CellInstance *cell, int layer_id, int demand):
	_name(name),
	_id(id),
	_cell(cell),
	_layer_id(layer_id),
	_demand(demand)
{}

string Blockage::get_name() const {return _name;}

int Blockage::get_id() const {return _id;}

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

vector<Pin*>& MasterCell::get_pins() {return _pin_list;}

vector<Blockage*>& MasterCell::get_blkgs() {return _blkg_list;}

int MasterCell::get_pin_by_name(string name) const {return _pin_name2id.at(name);}

void MasterCell::set_id(int val) {_id = val;}

void MasterCell::add_pin(string name, int id, int layer_id)
{
    _pin_list.push_back(
        new Pin(name, id, nullptr, layer_id));
    _pin_name2id[name] = id;
}

void MasterCell::add_blkg(string name, int id, int layer_id, int demand)
{
    _blkg_list.push_back(
        new Blockage(name, id, nullptr, layer_id, demand));
}


// class CellInstance

CellInstance::CellInstance(string name, int id, MasterCell &master_cell, bool fixed):
	_name(name),
	_id(id),
	_master_cell_id(master_cell.get_id()),
	_fixed(fixed),
    _row(0),
    _col(0),
    _hpwl_cost(0)
{
    _pin_list.clear();
    vector<Pin*>& pins = master_cell.get_pins();
    for(Pin *pin : pins){
        Pin *new_pin = new Pin(pin->get_name(), pin->get_id(), this, get<0>(pin->get_pos()));
        _pin_list.push_back(new_pin);
    }

    _blkg_list.clear();
    vector<Blockage*>& blkgs = master_cell.get_blkgs();
    for(Blockage *blkg : blkgs){
        Blockage *new_blkg = new Blockage(blkg->get_name(), blkg->get_id(), this, get<0>(blkg->get_pos()), blkg->get_demand());
        _blkg_list.push_back(new_blkg);
    }
}

string CellInstance::get_name() const {return _name;}

int CellInstance::get_id() const {return _id;}

int CellInstance::get_master_cell_id() const {return _master_cell_id;}

bool CellInstance::is_fixed() const {return _fixed;}

int CellInstance::get_row() const {return _row;}

int CellInstance::get_col() const {return _col;}

Pos2d CellInstance::get_pos() const {return Pos2d(_row, _col);}

int CellInstance::get_hpwl_cost() const {return _hpwl_cost;}

int CellInstance::get_num_pins() const {return _pin_list.size();}

int CellInstance::get_num_blkgs() const {return _blkg_list.size();}

Pin* CellInstance::get_pin(int idx) {return _pin_list.at(idx);}

Blockage* CellInstance::get_blkg(int idx) {return _blkg_list.at(idx);}

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

void CellInstance::set_hpwl_cost(int val) {_hpwl_cost = val;}

void CellInstance::print()
{
    cout<<"Cell #"<<get_id()<<" "<<get_name()<<" ("<<get_row()<<","<<get_col()<<") fixed: "<<is_fixed()<<"\n";
    for(Pin *pin : _pin_list){
        cout<<"  Pin "<<pin->get_name()<<" layer: "<<get<0>(pin->get_pos())<<"\n";
    }
    for(Blockage *blkg : _blkg_list){
        cout<<"  Blockage "<<blkg->get_name()<<" layer: "<<get<0>(blkg->get_pos())<<" demand: "<<blkg->get_demand()<<"\n";
    }
}


// class Net

Net::Net(string name, int id, int min_layer):
	_name(name),
	_id(id),
	_min_layer(min_layer),
    _hpwl(0)
{}

string Net::get_name() const {return _name;}

int Net::get_id() const {return _id;}

int Net::get_min_layer() const {return _min_layer;}

int Net::get_hpwl() const {return _hpwl;}

int Net::get_num_pins() const {return _pins.size();}

Pin* Net::get_pin(int idx) const {return _pins.at(idx);}

vector<Pos3d>& Net::get_route() {return _route;}

void Net::set_id(int val) {_id = val;}

void Net::set_hpwl(int val) {_hpwl = val;}

void Net::add_pin(Pin *pin)
{
    _pins.push_back(pin);
    pin->add_net(this);
}

void Net::add_route(Pos3d pos) {_route.push_back(pos);}

void Net::print() const
{
    cout<<"Net #"<<get_id()<<" "<<get_name()<<", #pins="<<get_num_pins()<<"\n";
    for(Pin *pin : _pins){
        int layer, row, col;
        tie(layer, row, col) = pin->get_pos();
        cout<<"  "<<pin->get_cell()->get_name()<<" "<<pin->get_name()<<" ("<<layer<<", "<<row<<", "<<col<<")\n";
    }
}


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

MasterCell CellLibrary::get_master_cell_by_id(int id) const {return _master_cell_list.at(id);}

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
Design::Design():
    _max_cell_move(0)
{}

int Design::get_num_cells() const {return _cell_list.size();}

int Design::get_num_nets() const {return _net_list.size();}

CellInstance* Design::get_cell_by_id(int id) const {return _cell_list.at(id);}

Net* Design::get_net_by_id(int id) const {return _net_list.at(id);}

CellInstance* Design::get_cell_by_name(string name) const {return _cell_list.at(_cell_name2id.at(name));}

Net* Design::get_net_by_name(string name) const {return _net_list.at(_net_name2id.at(name));}

int Design::get_max_cell_move() const {return _max_cell_move;}

int Design::get_num_moved_cells() const {return _moved_cell_id.size();}

bool Design::cell_is_moved(int cell_id) const {return _moved_cell_id.find(cell_id)!=_moved_cell_id.end();}

void Design::set_max_cell_move(int val) {_max_cell_move = val;}

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

void Design::move_cell(int cell_id) {_moved_cell_id.insert(cell_id);}

void Design::print_summary(bool detail)
{
    cout<<"\nDesign info:\n";
    cout<<"max #cell moves: "<<get_max_cell_move()<<"\n";
    cout<<"#cells: "<<get_num_cells()<<"\n";
    if(detail){
        for(CellInstance *cell : _cell_list){
            cell->print();
        }
    }

    cout<<"#moved cells: "<<get_num_moved_cells()<<"\n  id: ";
    for(int cell_id : _moved_cell_id){
        cout<<cell_id<<" ";
    }
    cout<<"\n";
    
    cout<<"#nets:"<<get_num_nets()<<"\n";
    if(detail){
        for(Net *net : _net_list){
            net->print();
        }
    }
}
