#pragma once
#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

// length: km
// time: minutes
struct Edge {
	double length;
	double time;
	
	Edge() {
		length = 0;
		time = 0;
	};
	
	Edge(double length, double time)
	{
		this->length = length;
		this->time = time;
	}
};

// x: km
// y: km
// adjs: <nodeID, edgeID>
struct Node {
	double x, y;
	vector<pair<int, int>> adjs;
	
	Node()
	{
		x = 0;
		y = 0;
	};

	Node(int x, int y)
	{
		this->x = x;
		this->y = y;
	};

	int getEdgeID(int nodeAdj)
	{
		for (const auto& i : adjs)
		{
			if (i.first == nodeAdj)
				return i.second;
		}

		return -1;
	}
};

// (x,y): km
// radius: meters
struct Query {
	double startX, startY;
	double destX, destY;
	double radius;
};

// nodeIDs: Nodes of the shortest path
// time: mins
// dist: km
struct Output {
	vector<int> nodeIDs;
	double time;
	double distTotal, distWalk, distDrive;
};

struct Test {
	string type, mapFile, queryFile, outputFile;
	Test(string type, string mapFile, string queryFile, string outputFile)
	{
		this->type = type;
		this->mapFile = mapFile;
		this->queryFile = queryFile;
		this->outputFile = outputFile;
	}
};

typedef vector<Node> Graph;
