#include <tuple>
#include <vector>
#include <set>

using namespace std;

typedef tuple<int, int, int> Pos3d; // layer, row, col

class Vertex;
class Edge;

class RoutingGraph{
    vector<Vertex*> _vertex_list;
    vector<Edge*> _edge_list;
public:
    RoutingGraph(const RoutingGraph &input_graph, bool vertex_only);

    int get_num_vertices() const;
    int get_num_edges() const;

    Vertex* get_vertex(int id) const;
    Edge* get_edge(int id) const;

    void add_vertex(Pos3d pos);
    void add_edge(int v1, int v2);
    void remove_edge(int id);
};

class Vertex{
    int _id;
    Pos3d _pos;
    set<int> _edge_ids;
public:
    Vertex(int id, const Pos3d &pos);

    Pos3d get_pos() const;
    set<int>& get_edges();

    void add_edge(int edge_id);
    void remove_edge(int edge_id);
};

class Edge{
    int _id;
    int _v1;
    int _v2;
    int _weight;
public:
    Edge(int id, int v1, int v2, int weight);

    int get_v1() const;
    int get_v2() const;
    int get_weight() const;
};