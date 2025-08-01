#pragma once
#include "types.h"

class file_io
{
	static string getCurTimeFileName();
public:
	static string getCurTime();
	static void saveOutputs(vector<Query> &queries, vector<pair<string, Output>> &outputs, string mapName);
};

