#include "module.h"

class Router{
    Chip &_chip;
    vector<Net*> &_net_list;
    vector<Net*> _sorted_net_list;
    bool sort_net_by_hpwl(Net *n1, Net *n2);
public:
    Router(Chip &chip, vector<Net*> &net_list);

    void routing_flow();
    set<Pos3d> collect_steiner_points(Net *net);
};
