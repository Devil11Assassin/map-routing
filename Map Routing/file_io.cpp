#include "file_io.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

string file_io::getCurTimeFileName()
{
	time_t curTime_t = chrono::system_clock::to_time_t(chrono::system_clock::now());
	stringstream ss;
	tm bt{};

	localtime_s(&bt, &curTime_t);
	ss << put_time(&bt, "%Y-%m-%d %H-%M-%S");

	return ss.str();
}

string file_io::getCurTime()
{
	time_t curTime_t = chrono::system_clock::to_time_t(chrono::system_clock::now());
	stringstream ss;
	tm bt{};
	
	localtime_s(&bt, &curTime_t);
	ss << put_time(&bt, "%Y-%m-%d %H:%M:%S");

	return ss.str();
}

void file_io::saveOutputs(vector<Query> &queries, vector<pair<string, Output>> &outputs, string mapName)
{
	ofstream file;
	file.open("queries//" + getCurTimeFileName() + " " + mapName + " Queries.txt");

	int numOfOutputs = outputs.size();
	for (int i = 0; i < numOfOutputs; i++)
	{
		file << std::fixed << std::setprecision(2);
		file << "Timestamp: " << outputs[i].first << "\n";
		file << "Start: (" << queries[i].startX << ", " << queries[i].startY << ")\n";
		file << "Destination: (" << queries[i].destX << ", " << queries[i].destY << ")\n";
		file << "Radius: " << queries[i].radius << " meters\n";
		
		Output &output = outputs[i].second;

		int numOfNodes = output.nodeIDs.size();
		file << "Path: " << output.nodeIDs[0];
		for (int j = 1; j < numOfNodes; j++)
			file << " " << output.nodeIDs[j];

		file << "Time: " << output.time << " mins\n";
		file << "Distance: " << output.distTotal << " km\n";
		file << "Distance (Walking): " << output.distWalk << " km\n";
		file << "Distance (Driving): " << output.distDrive << " km\n\n";
	}

	file.close();
}