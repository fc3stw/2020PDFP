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
    RoutingGraph(){}
    RoutingGraph(const RoutingGraph &input_graph, bool vertex_only);
    ~RoutingGraph();

    int get_num_vertices() const;
    int get_num_edges() const;

    Vertex* get_vertex(int id) const;
    Edge* get_edge(int id) const;
    int get_total_edge_weights() const;

    void add_vertex(Pos3d pos);
    void add_edge(int v1, int v2);
    void remove_vertex(int id);
    void remove_edge(int id);
};

class Vertex{
    int _id;
    Pos3d _pos;
    set<Edge*> _edges;
public:
    Vertex(int id, const Pos3d &pos);

    int get_id() const;
    Pos3d get_pos() const;
    set<Edge*>& get_edges();

    void set_id(int val);
    void add_edge(Edge *e);
};

class Edge{
    int _id;
    Vertex *_v1;
    Vertex *_v2;
    int _weight;
public:
    Edge(int id, Vertex* v1, Vertex* v2, int weight);

    int get_id() const;
    Vertex* get_v1() const;
    Vertex* get_v2() const;
    int get_weight() const;

    void set_id(int val);
};