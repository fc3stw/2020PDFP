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

typedef tuple<int, int, int> Pos3d; // layer, row, col
typedef tuple<int, int> Pos2d; // row, col
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
	Pos3d _pos;
	int _supply;
	int _demand; // including extra demand
public:
	gGrid(int layer, int row, int col, int supply);

	Pos3d get_pos() const;
	int get_demand() const;
	int get_remain_demand() const;

	bool add_demand(int val);
};

class Layer{
	string _name;
	int _id;
	Direction _dir;
	int _supply;
	vector<vector<gGrid>> _gGrid2d;
public:
	Layer(string name, int id, Direction dir, int supply, int num_rows, int num_cols);

	Direction get_dir() const;
	gGrid& get_grid(int row, int col);
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
	Chip(int row_begin, int col_begin, int row_end, int col_end);

	int get_num_rows() const;
	int get_num_cols() const;

	void add_layer(string name, int supply);
};

class Pin{
	string _name;
	int _id;
	CellInstance *_cell;
	int _layer_id;
	vector<Net*> _net_list;
public:
	Pin(string name, int id, CellInstance *cell, int layer_id);

	Pos3d get_pos() const;
	void add_net(Net *net);
};

class Blockage{
	string _name;
	int _id;
	CellInstance *_cell;
	int _layer_id;
	int _demand; // only default demand
public:
	Blockage(string name, int id, CellInstance *cell, int layer_id, int demand);

	Pos3d get_pos() const;
	int get_demand() const;
};

class MasterCell{
	// This is the cell type
	string _name;
	int _id;
	vector<Pin> _pin_list;
	vector<Blockage> _blkg_list;
public:
	MasterCell(string name, int id);

	string get_name() const;
	int get_id() const;
	vector<Pin>& get_pins();
	vector<Blockage>& get_blkgs();

	void set_id(int val);
	void add_pin(string name, int id, int layer_id);
	void add_blkg(string name, int id, int layer_id, int demand);
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
	CellInstance(string name, int id, MasterCell master_cell, bool fixed);

	string get_name() const;
	int get_id() const;
	bool is_fixed() const;
	int get_row() const;
	int get_col() const;
	Pos2d get_pos() const;

	void set_id(int val);
	void set_pos(Pos2d pos);
	void set_pos(int row, int col);
};

class Net{
	string _name;
	int _id;
	int _min_layer;
	vector<Pin*> _pins;
	vector<Pos3d> _route;
public:
	Net(string name, int id, int min_layer);

	string get_name() const;
	int get_id() const;
	int get_min_layer() const;

	void set_id(int val);
	void add_pin(Pin *pin);
	void add_route(Pos3d pos);
};

class CellLibrary{
	vector<MasterCell> _master_cell_list;
	map<string, int> _name2id;

	int get_id_by_name(string name) const {
		if(_name2id.find(name)==_name2id.end()) return -1;
		return _name2id.at(name);
	}
public:
	CellLibrary();

	MasterCell get_master_cell_by_name(string name, bool &success) const;

	void add_master_cell(MasterCell ms);
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
	ExtraDemand();

	int get_same_demand(int ms1, int ms2, int layer) const;
	int get_adj_demand(int ms1, int ms2, int layer) const;

	void add_same_demand(int ms1, int ms2, int layer, int demand);
	void add_adj_demand(int ms1, int ms2, int layer, int demand);
};

class Design{
	int _max_cell_move;
	vector<CellInstance*> _cell_list;
	vector<Net*> _net_list;
	set<int> _moved_cell_id;
	map<string, int> _cell_name2id;
	map<string, int> _net_name2id;
public:
	Design(int max_cell_move, int num_cells, int num_nets);

	CellInstance* get_cell_by_id(int id) const;
	Net* get_net_by_id(int id) const;

	CellInstance* get_cell_by_name(string name) const;
	Net* get_net_by_name(string name) const;

	void add_cell(CellInstance *cell);
	void add_net(Net *net);
};
