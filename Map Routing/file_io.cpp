#include "file_io.h"

void file_io::saveOutputs(vector<Query> &queries, vector<Output> &outputs, string mapName)
{
	ofstream file;
	file.open("queries/" + mapName + " Queries.txt");

	int numOfOutputs = outputs.size();
	for (int i = 0; i < numOfOutputs; i++)
	{
		file << std::fixed << std::setprecision(2);
		
		file << "Start: (" << queries[i].startX << ", " << queries[i].startY << ")\n";
		file << "Destination: (" << queries[i].destX << ", " << queries[i].destY << ")\n";
		file << "Radius: " << queries[i].radius << " meters\n";
		
		int numOfNodes = outputs[i].nodeIDs.size();
		file << "Path: " << outputs[i].nodeIDs[0];
		for (int j = 1; j < numOfNodes; j++)
			file << " " << outputs[i].nodeIDs[j];

		file << "Time: " << outputs[i].time << " mins\n";
		file << "Distance: " << outputs[i].distTotal << " km\n";
		file << "Distance (Walking): " << outputs[i].distWalk << " km\n";
		file << "Distance (Driving): " << outputs[i].distDrive << " km\n\n";
	}

	file.close();
}