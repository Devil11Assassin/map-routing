#pragma once
#include "types.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <iostream>
#include <thread>

class gui
{
	enum Menu {
		Selection,
		Routing,
		Exit
	};

	struct MenuInfo
	{
		double xMin = DBL_MAX, yMin = DBL_MAX;
		double xMax = DBL_MIN, yMax = DBL_MIN;
		double width, height;

		int vertexCount = 0;
		vector<sf::Vertex> edgeVertices;
		vector<int> nodesDrawn;
		vector<int> edgesDrawn;
	};

	static sf::RenderWindow window;
	static sf::Event event;

	static vector<Query> queries;
	static vector<pair<string, Output>> outputs;

	static vector<string> mapNames;
	static vector<string> mapPaths;
	static vector<Graph> maps;
	static vector<vector<Edge>> mapsEdges;
	static vector<MenuInfo> menuInfos;

	static Menu menuSelected;
	static int mapSelected;

	static const int WINDOW_WIDTH, WINDOW_HEIGHT;
	static const int MAP_WIDTH, MAP_HEIGHT;
	static const int LEFT_BOUNDARY, RIGHT_BOUNDARY;
	static const int UP_BOUNDARY, DOWN_BOUNDARY;

	static void loadMaps();
	static void calculateMenuVerticesThread(int mapIndex);
	static void calculateMenuVertices();
	static void mapSelectionMenu();
	static void mapRoutingMenu();

public:
	static sf::Cursor cursorArrow, cursorHand, cursorCross;

	static void initialization();
	static void testing();
};

class Buttons
{
public:
	std::vector<sf::Text> texts;
	std::vector<sf::RectangleShape> backgrounds;
	static sf::Font font;
	int index;
	int size;

	sf::Color textColor = sf::Color::Black;
	sf::Color selectedTextColor = sf::Color::Red;
	sf::Color backgroundColor = sf::Color(200, 200, 200, 255);
	sf::Color selectedBackgroundColor = sf::Color(150, 150, 150, 255);

	sf::Vector2f backgroundSize;

	Buttons(std::vector<string> strings, sf::RenderWindow& window);

	void hoverDetection(sf::RenderWindow& window);
	void clickDetection(int &selection);
	void draw(sf::RenderWindow& window);
};