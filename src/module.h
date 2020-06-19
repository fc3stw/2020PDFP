#include <tuple>
#include <string>
#include <vector>
#include <set>
#include <map>

using namespace std;

#define H 0
#define V 1
#define Z 2
typedef short Direction;
// placement related

typedef tuple<int, int, int> Pos; // layer, row, col
typedef tuple<int, int, int> ExtraDemandCondition; // ms1 id, ms2 id, layer id

class gGrid;
class Layer;
class Chip;
class Pin;
class Blockage;
class MasterCell;
class CellInstance;
class Net;
class CellLibrary;
class ExtraDemand;
class Design;

class gGrid{
	// This is the basic P&R resource
	Pos _pos;
	int _supply;
	int _demand; // including extra demand
public:
	gGrid(int layer, int row, int col, int supply);

	Pos get_pos() const {return _pos;}
	int get_demand() const {return _demand;}

	void add_supply(int val);
	bool add_demand(int val);
};

class Layer{
	string _name;
	int _id;
	Direction _dir;
	int _supply;
	vector<vector<gGrid>> _gGrid2d;
public:
	Layer(string name, int id, Direction dir, int supply, int num_rows, int num_cols):
	_name(name),
	_id(id),
	_dir(dir),
	_supply(supply)
	{
		_gGrid2d.clear();
		for(int r = 0; r<num_rows; r++){
			vector<gGrid> grid_row;
			for(int c = 0; c<num_cols; c++){
				grid_row.push_back(gGrid(id, r, c, supply));
			}
			_gGrid2d.push_back(grid_row);
		}
	}

	Direction get_dir() const {return _dir;}
	gGrid& get_grid(int row, int col) {return _gGrid2d[row][col];}
};

class Chip{
	// assume idx starts from 0, need to be handled by parser
	// assume rows, columns and layers are continuous
	int _row_begin_idx;
	int _col_begin_idx;
	int _num_rows;
	int _num_cols;
	vector<Layer> _layer_list;

	Direction get_layer_dir(int layer_id){return layer_id % 2;}
public:
	Chip(int row_begin, int col_begin, int row_end, int col_end):
	_row_begin_idx(row_begin),
	_col_begin_idx(col_begin),
	_num_rows(row_end - row_begin + 1),
	_num_cols(col_end - col_begin + 1)
	{
		_layer_list.clear();
	}

	int get_num_rows() const {return _num_rows;}
	int get_num_cols() const {return _num_cols;}

	void add_layer(string name, int supply){
		int layer_id = _layer_list.size();
		_layer_list.push_back(
			Layer(name, layer_id, get_layer_dir(layer_id), supply, _num_rows, _num_cols));
	}
};

class Pin{
	string _name;
	int _id;
	CellInstance *_cell;
	int _layer_id;
public:
	Pin(string name, int id, CellInstance *cell, int layer_id):
	_name(name),
	_id(id),
	_cell(cell),
	_layer_id(layer_id)
	{}

	Pos get_pos() const {return Pos(_layer_id, _cell->get_row(), _cell->get_col());}
};

class Blockage{
	string _name;
	int _id;
	CellInstance *_cell;
	int _layer_id;
	int _demand; // only default demand
public:
	Blockage(string name, int id, CellInstance *cell, int layer_id, int demand):
	_name(name),
	_id(id),
	_cell(cell),
	_layer_id(layer_id),
	_demand(demand)
	{}

	Pos get_pos() const {return Pos(_layer_id, _cell->get_row(), _cell->get_col());}
	int get_demand() const {return _demand;}
};

class MasterCell{
	// This is the cell type
	string _name;
	int _id;
	vector<Pin> _pin_list;
	vector<Blockage> _blkg_list;
public:
	MasterCell(string name, int id):
	_name(name),
	_id(id)
	{
		_pin_list.clear();
		_blkg_list.clear();
	}

	string get_name() const {return _name;}
	int get_id() const {return _id;}
	vector<Pin>& get_pins() {return _pin_list;}
	vector<Blockage>& get_blkgs() {return _blkg_list;}

	void set_id(int val) {_id = val;}
	void add_pin(string name, int id, int layer_id){_pin_list.push_back(
		Pin(name, id, nullptr, layer_id)
	);}
	void add_blkg(string name, int id, int layer_id, int demand){_blkg_list.push_back(
		Blockage(name, id, nullptr, layer_id, demand)
	);}
};

class CellInstance{
	string _name;
	int _id;
	int _master_cell_id;
	bool _fixed;
	vector<Pin*> _pin_list;
	vector<Blockage*> _blkg_list;
	int _row;
	int _col;
public:
	CellInstance(string name, int id, MasterCell master_cell, bool fixed):
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

	string get_name() const {return _name;}
	int get_id() const {return _id;}
	bool is_fixed() const {return _fixed;}
	int get_row() const {return _row;}
	int get_col() const {return _col;}

	void set_id(int val) {_id = val;}
	void set_pos(int row, int col){
		if(is_fixed()) return;
		_row = row;
		_col = col;
	}
};

class Net{
	string _name;
	int _id;
	int _min_layer;
	vector<Pin*> _pins;
	vector<Pos> _route;
public:
	Net(string name, int id, int min_layer):
	_name(name),
	_id(id),
	_min_layer(min_layer)
	{}

	string get_name() const {return _name;}
	int get_id() const {return _id;}
	int get_min_layer() const {return _min_layer;}

	void set_id(int val) {_id = val;}
	void add_pin(Pin *pin) {_pins.push_back(pin);}
	void add_route(Pos pos) {_route.push_back(pos);}
};

class CellLibrary{
	vector<MasterCell> _master_cell_list;
	map<string, int> _name2id;

	int get_id_by_name(string name) const {
		if(_name2id.find(name)==_name2id.end()) return -1;
		return _name2id.at(name);
	}
public:
	CellLibrary(){}

	MasterCell get_master_cell_by_name(string name, bool &success) const {
		int id = get_id_by_name(name);
		if(id==-1){
			success = false;
			return MasterCell(0,0);
		}
		return _master_cell_list.at(id);
	}

	void add_master_cell(MasterCell ms){
		int id = _master_cell_list.size();
		ms.set_id(id);
		_master_cell_list.push_back(ms);
		_name2id[ms.get_name()] = id;
	}
};

class ExtraDemand{
	map<ExtraDemandCondition, int> same_grid;
	map<ExtraDemandCondition, int> adj_hor_grid;

	ExtraDemandCondition get_cond(int ms1, int ms2, int layer) const {
		if(ms2 < ms1){
			int tmp = ms2;
			ms2 = ms1;
			ms1 = tmp;
		}
		return ExtraDemandCondition(ms1, ms2, layer);
	}
public:
	ExtraDemand(){}

	int get_same_demand(int ms1, int ms2, int layer) const {
		ExtraDemandCondition cond = get_cond(ms1, ms2, layer);
		if(same_grid.find(cond)==same_grid.end()) return 0;
		return same_grid.at(cond);
	}
	int get_adj_demand(int ms1, int ms2, int layer) const {
		ExtraDemandCondition cond = get_cond(ms1, ms2, layer);
		if(adj_hor_grid.find(cond)==adj_hor_grid.end()) return 0;
		return adj_hor_grid.at(cond);
	}

	void add_same_demand(int ms1, int ms2, int layer, int demand){
		ExtraDemandCondition cond = get_cond(ms1, ms2, layer);
		same_grid[cond] = demand;
	}
	void add_adj_demand(int ms1, int ms2, int layer, int demand){
		ExtraDemandCondition cond = get_cond(ms1, ms2, layer);
		adj_hor_grid[cond] = demand;
	}
};

class Design{
	int _max_cell_move;
	vector<CellInstance*> _cell_list;
	vector<Net*> _net_list;
	set<int> _moved_cell_id;
	map<string, int> _cell_name2id;
	map<string, int> _net_name2id;
public:
	Design(int max_cell_move, int num_cells, int num_nets) :
	_max_cell_move(max_cell_move)
	{
		_cell_list.clear();
		_net_list.clear();
		_moved_cell_id.clear();
		_cell_name2id.clear();
		_net_name2id.clear();
	}

	CellInstance* get_cell_by_id(int id) const {return _cell_list.at(id);}
	Net* get_net_by_id(int id) const {return _net_list.at(id);}

	CellInstance* get_cell_by_name(string name) const {return _cell_list.at(_cell_name2id.at(name));}
	Net* get_net_by_name(string name) const {return _net_list.at(_net_name2id.at(name));}

	void add_cell(CellInstance *cell){
		int id = _cell_list.size();
		cell->set_id(id);
		_cell_list.push_back(cell);
		_cell_name2id[cell->get_name()] = id;
	}
	void add_net(Net *net){
		int id = _net_list.size();
		net->set_id(id);
		_net_list.push_back(net);
		_net_name2id[net->get_name()] = id;
	}
};
