#include "globalRouter.h"
#include <cmath>
#include <algorithm>
#include <map>

GlobalRouter::GlobalRouter(const RoutingGraph &original_graph)
{
	_original_graph = RoutingGraph(original_graph, false);
	_mst = RoutingGraph(original_graph, true);
}

RoutingGraph GlobalRouter::get_mst() const {return _mst;}

bool compare(const Edge *e1, const Edge *e2)
{
	return e1->get_weight() < e2->get_weight();
}

void GlobalRouter::route()
{
	map<int, int> v_to_cluster_id;
	vector<Edge*> edge_list;
	int cluster = 0;

	int num_edges = _original_graph.get_num_edges();
	for(int i = 0; i < num_edges; i++){
		Edge *edge;
		edge = _original_graph.get_edge(i);
		edge_list.push_back(edge);
	}

	sort(edge_list.begin(), edge_list.end(), compare);

	for(int i = 0; i < num_edges; i++){
		int v1 = edge_list[i]->get_v1()->get_id();
		int v2 = edge_list[i]->get_v2()->get_id();
		bool v1_find = (v_to_cluster_id.find(v1) != v_to_cluster_id.end()) ? true : false;
		bool v2_find = (v_to_cluster_id.find(v2) != v_to_cluster_id.end()) ? true : false;

		if(v1_find == true && v2_find == true && (v_to_cluster_id[v1] == v_to_cluster_id[v2]))
			continue;

		if(v1_find == false && v2_find == false){
			v_to_cluster_id[v1] = cluster;
			v_to_cluster_id[v2] = cluster;
		}
		else if(v1_find == true && v2_find == false){
			v_to_cluster_id[v2] = v_to_cluster_id[v1];
		}
		else if(v1_find == false && v2_find == true){
			v_to_cluster_id[v1] = v_to_cluster_id[v2];
		}
		else{
			for(auto c : v_to_cluster_id){
				if(c.second == v_to_cluster_id[v1] || c.second == v_to_cluster_id[v2]){
					v_to_cluster_id[v1] = cluster;
					v_to_cluster_id[v2] = cluster;
				}
			}
		}
		cluster++;
		_mst.add_edge(v1, v2);
	}
}
