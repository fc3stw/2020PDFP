#include <tuple>
#include <vector>

using namespace std;

typedef tuple<int, int, int> Pos; // layer, row, col

class Vertex{
    Pos _pos;
public:
    Vertex(const Pos &pos);

    Pos get_pos() const;
};

class Edge{
    Vertex *_v1;
    Vertex *_v2;
public:
    Edge(Vertex *v1, Vertex *v2);

    Vertex* get_v1() const;
    Vertex* get_v2() const;
    int get_wl() const;
};

class GlobalRouter{
    vector<Vertex*> _vertex_list;
    vector<Edge*> _edge_list;
    vector<Edge*> _min_spanning_tree;
public:
    GlobalRouter(const vector<Vertex*> &vertex_list);

    vector<Edge*>& global_route();
};