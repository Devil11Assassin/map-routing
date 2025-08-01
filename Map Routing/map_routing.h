#pragma once
#include "types.h"

class map_routing
{
	static vector<pair<int, double>> getFilteredNodes(double x, double y, double r, Graph& map);
	static Output dijkstra(Graph& map, vector<Edge>& edges);
	static Output getShortestPath(Query query, vector<pair<int, double>> startNodes, vector<pair<int, double>> destNodes,
		Graph& map, vector<Edge>& edges);

public:
	static Graph map;
	static vector<Edge> edges;

	static Output solveQuery(Query query, Graph& map, vector<Edge>& edges);
};
