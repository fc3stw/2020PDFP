#include "routingGraph.h"
#include <tuple>
#include <vector>

using namespace std;

class GlobalRouter{
    RoutingGraph _original_graph;
    RoutingGraph _mst;
public:
    GlobalRouter(RoutingGraph &original_graph);

    RoutingGraph get_mst() const;

    void global_route();
};