#include "globalRouter.h"
#include <cmath>
#include <algorithm>
#include <map>
#include <iostream>
#include <cassert>

GlobalRouter::GlobalRouter(RoutingGraph const *original_graph)
{
	_original_graph = new RoutingGraph(original_graph, false);
	_mst = new RoutingGraph(original_graph, true);
}

GlobalRouter::~GlobalRouter()
{
	// if(_original_graph) delete _original_graph;
	// if(_mst) delete _mst;
}

RoutingGraph* GlobalRouter::get_mst() const {return _mst;}

bool compare(const Edge *e1, const Edge *e2)
{
	return e1->get_weight() < e2->get_weight();
}

void GlobalRouter::set_graph(RoutingGraph const *original_graph)
{
	_original_graph = new RoutingGraph(original_graph, false);
	_mst = new RoutingGraph(original_graph, true);
}

void GlobalRouter::clear_graph()
{
	delete _original_graph;
	delete _mst;
}

void GlobalRouter::route()
{
	map<int, int> v_to_cluster_id;
	vector<Edge*> edge_list;
	int cluster = 0;

	int num_edges = _original_graph->get_num_edges();
	for(int i = 0; i < num_edges; i++){
		Edge *edge = _original_graph->get_edge(i);
		edge_list.push_back(edge);
	}

	sort(edge_list.begin(), edge_list.end(), compare);

	for(int i = 0; i < num_edges; i++){
		if(_mst->get_num_edges() == _mst->get_num_vertices()-1) break;

		int v1 = edge_list.at(i)->get_v1()->get_id();
		int v2 = edge_list.at(i)->get_v2()->get_id();
		bool v1_find = (v_to_cluster_id.find(v1) != v_to_cluster_id.end());
		bool v2_find = (v_to_cluster_id.find(v2) != v_to_cluster_id.end());

		if(v1_find == true && v2_find == true){
			if(v_to_cluster_id.at(v1) == v_to_cluster_id.at(v2)){
				continue;
			}
		}

		if(v1_find == false && v2_find == false){
			v_to_cluster_id[v1] = cluster;
			v_to_cluster_id[v2] = cluster;
		}
		else if(v1_find == true && v2_find == false){
			v_to_cluster_id[v2] = v_to_cluster_id.at(v1);
		}
		else if(v1_find == false && v2_find == true){
			v_to_cluster_id[v1] = v_to_cluster_id.at(v2);
		}
		else{
			for(auto c : v_to_cluster_id){
				if(c.second == v_to_cluster_id.at(v1) || c.second == v_to_cluster_id.at(v2)){
					v_to_cluster_id[v1] = cluster;
					v_to_cluster_id[v2] = cluster;
				}
			}
		}
		cluster++;
		_mst->add_edge(v1, v2);
	}
}
