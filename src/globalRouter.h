#include "routingGraph.h"
#include <tuple>
#include <vector>

using namespace std;

class GlobalRouter{
    RoutingGraph *_original_graph;
    RoutingGraph *_mst;
public:
    GlobalRouter(){}
    GlobalRouter(RoutingGraph const *original_graph);
    ~GlobalRouter();

    RoutingGraph *get_mst() const;

    void set_graph(RoutingGraph const *original_graph);
    void clear_graph();

    void route();
};