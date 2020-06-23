#include "module.h"
#include "globalRouter.h"

struct Route{
    set<Pos3d> _grids;
    Pos3d _wavefront;
    int _cost;

    Route(){}
    Route(Pos3d source, Pos3d target);
    Route(Route const *route, Pos3d wavefront, Pos3d target);

    void print() const;
};

struct SortRouteByCost{
    bool operator() (const Route *r1, const Route *r2) const{
        return r1->_cost > r2->_cost;
    }
};

class Router{
    Chip &_chip;
    vector<Net*> _net_list;
    vector<RoutingGraph*> _steiner_trees;
public:
    Router(Chip &chip, vector<Net*> &net_list);

    int get_total_wl() const;
    int get_total_hpwl() const;

    void routing_flow();
    void ripup_all_routes();
    void add_net_on_grids();
    void global_route();
    void detail_route();
    set<Pos3d> collect_steiner_points(Net const *net);
    Route* Astar_search(int net_id, Pos3d source, Pos3d target);
    bool check_new_route(int net_id, Route *route, Pos3d wavefront);

    void print_global_route_result() const;
    void print_detail_route_result() const;
};
