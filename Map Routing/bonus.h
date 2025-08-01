#pragma once
#include "types.h"

struct EdgeVar
{
	double length;
	vector<double> durations;
};

class bonus
{
	static unordered_map<int, double> getFilteredNodes(double x, double y, double r, Graph &map);
	static Output dijkstra(Graph &map, vector<EdgeVar> &edgesVar);
	static Output getShortestPath(Query query, unordered_map<int, double> startNodes, unordered_map<int, double> destNodes,
		Graph &map, vector<EdgeVar> &edgesVar);

public:
	static double interval;
	static int numOfSpeeds;
	static vector<EdgeVar> edgesVar;

	static Output solveQuery(Query query, Graph &map, vector<EdgeVar> &edgesVar);
	static void readMap(string type, string fileName);
};
