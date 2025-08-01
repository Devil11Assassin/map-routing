#include "visualizer.h"
#include "map_routing.h"
#include <unordered_set>
#include <iostream>
#include <algorithm>

using namespace sf;

enum Action {
    None,
    Exit,
    OutputNext,
    OutputPrev,
    OutputPrediction,
    OutputActual
};

Action pollEvents(RenderWindow &window)
{
    Action action = Action::None;
    
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            action = Action::Exit;

        else if (event.type == sf::Event::KeyPressed)
        {
            switch (event.key.code)
            {
                case Keyboard::Escape:
                    action = Action::Exit;
                    break;
                case Keyboard::Up:
                case Keyboard::W:
                    action = Action::OutputPrediction;
                    break;
                case Keyboard::Down:
                case Keyboard::S:
                    action = Action::OutputActual;
                    break;
                case Keyboard::Right:
                case Keyboard::D:
                    action = Action::OutputNext;
                    break;
                case Keyboard::Left:
                case Keyboard::A:
                    action = Action::OutputPrev;
                    break;
            }
        }
    }

    return action;
}

void visualizer::visualize(vector<Output>& outputs, vector<Output>& outputsActual, vector<Query>& queries, string curQuery)
{
    // Creating the window
    const int WINDOW_WIDTH = VideoMode::getDesktopMode().width, WINDOW_HEIGHT = VideoMode::getDesktopMode().height;
    const int MAP_WIDTH = static_cast<int>(WINDOW_WIDTH*0.78125), MAP_HEIGHT = static_cast<int>(WINDOW_HEIGHT*0.972);
    
    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Map Routing", Style::Fullscreen);
    View view(Vector2f(MAP_WIDTH/2.f, MAP_HEIGHT/2.f), Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    window.setView(view);

    Font font;
    try {
        font.loadFromFile("CONSOLA.TTF");
    }
    catch (exception& e) {
        cout << "ERROR: Can't read font! Font file doesn't exist or access denied!";
    }

    Text curQueryText;
    curQueryText.setFont(font);
    curQueryText.setString(curQuery);
    curQueryText.setFillColor(Color(255, 0, 0, 150));
    curQueryText.setPosition(Vector2f((MAP_WIDTH - WINDOW_WIDTH)/2.f, (MAP_HEIGHT - WINDOW_HEIGHT)/2.f));
    
    Text curOutputText;
    curOutputText.setFont(font);
    curOutputText.setFillColor(Color(255, 0, 0, 150));
    curOutputText.setPosition(Vector2f((MAP_WIDTH - WINDOW_WIDTH)/2.f, (MAP_HEIGHT - WINDOW_HEIGHT)/2.f + 40));

    // Getting map's boundaries
    double xMin = FLT_MAX, yMin = FLT_MAX;
    double xMax = -FLT_MAX, yMax = -FLT_MAX;

    for (const auto& node : map_routing::map)
    {
        if (node.x < xMin)
            xMin = node.x;
        if (node.x > xMax)
            xMax = node.x;

        if (node.y < yMin)
            yMin = node.y;
        if (node.y > yMax)
            yMax = node.y;
    }

    double width = xMax - xMin;
    double height = yMax - yMin;

    CircleShape start, startCircle, dest, destCircle;
    start.setFillColor(Color::Green);
    startCircle.setFillColor(Color::Transparent);
    startCircle.setOutlineColor(Color::Green);
    startCircle.setOutlineThickness(3.f);
    dest.setFillColor(Color::Red);
    destCircle.setFillColor(Color::Transparent);
    destCircle.setOutlineColor(Color::Red);
    destCircle.setOutlineThickness(3.f);
    

    // Creating the normalized lines
	unordered_map<int, int> edgesDrawn;
    int vertexCount = map_routing::edges.size() * 2;
    Vertex* edgesV = new Vertex[vertexCount];
    
    int index = 0;
    for (const Node& node : map_routing::map)
    {
        for (const auto& adj : node.adjs)
        {
            if (!edgesDrawn.count(adj.second))
            {
                edgesV[index++] = Vertex(Vector2f((node.x - xMin) / width, (node.y - yMin) / height), Color::White);
                edgesV[index++] = Vertex(Vector2f((map_routing::map[adj.first].x - xMin) / width, (map_routing::map[adj.first].y - yMin) / height), Color::White);
                edgesDrawn.insert(make_pair(adj.second, index-2));
            }
        }
    }

    // Denormalizing to screen coordinates
    for (int i = 0; i < vertexCount; i++)
    {
        edgesV[i].position.x *= MAP_WIDTH;
        edgesV[i].position.y = MAP_HEIGHT - (edgesV[i].position.y * MAP_HEIGHT);
    }

    int outputIndex = 0;
    bool showActual = false;
    Action action = Action::None;

    // Draw loop
    while (true)
    {
        vector<int> nodeIDs;
        
        if (!showActual && !outputs.empty())
        {
            nodeIDs = outputs[outputIndex].nodeIDs;
            curOutputText.setString("Output " + to_string(outputIndex));
        }
        else if (showActual && !outputsActual.empty())
        {
            nodeIDs = outputsActual[outputIndex].nodeIDs;
            curOutputText.setString("Output " + to_string(outputIndex) + " (ACTUAL)");
        }

        startCircle.setRadius((queries[outputIndex].radius*MAP_WIDTH) / (1000.f*width));
        startCircle.setScale(Vector2f(1.0, (MAP_HEIGHT/height)/(MAP_WIDTH/width)));
        startCircle.setOrigin(Vector2f(startCircle.getRadius(), startCircle.getRadius()));
        startCircle.setPosition(Vector2f((queries[outputIndex].startX - xMin) * MAP_WIDTH / width,
            MAP_HEIGHT - (queries[outputIndex].startY - yMin)*MAP_HEIGHT/height));
        start.setRadius(startCircle.getRadius() / 10.f);
        start.setOrigin(Vector2f(start.getRadius(), start.getRadius()));
        start.setPosition(startCircle.getPosition());

        destCircle.setRadius(startCircle.getRadius());
        destCircle.setScale(Vector2f(1.0, (MAP_HEIGHT/height)/(MAP_WIDTH/width)));
        destCircle.setOrigin(Vector2f(destCircle.getRadius(), destCircle.getRadius()));
        destCircle.setPosition(Vector2f((queries[outputIndex].destX - xMin) * MAP_WIDTH / width,
            MAP_HEIGHT - ((queries[outputIndex].destY - yMin) * MAP_HEIGHT / height)));
        dest.setRadius(destCircle.getRadius() / 10.f);
        dest.setOrigin(Vector2f(dest.getRadius(), dest.getRadius()));
        dest.setPosition(destCircle.getPosition());

        int nodeCount = nodeIDs.size();
        for (int i = 1; i < nodeCount; i++)
        {
            int vertexIndex = edgesDrawn[map_routing::map[nodeIDs[i - 1]].getEdgeID(nodeIDs[i])];
            edgesV[vertexIndex].color = Color::Red;
            edgesV[vertexIndex + 1].color = Color::Red;
        }

        while (window.isOpen())
        {
            action = pollEvents(window);

            if (action != Action::None)
                break;

            window.clear();
            window.draw(edgesV, vertexCount, Lines);
            window.draw(startCircle);
            window.draw(start);
            window.draw(destCircle);
            window.draw(dest);
            window.draw(curQueryText);
            window.draw(curOutputText);
            window.display();
        }

        if (action == Action::Exit)
        {
            window.close();
            return;
        }
        else
        {
            for (int i = 1; i < nodeCount; i++)
            {
                auto adjs = map_routing::map[nodeIDs[i - 1]].adjs;

                int vertexIndex = edgesDrawn[map_routing::map[nodeIDs[i - 1]].getEdgeID(nodeIDs[i])];
                edgesV[vertexIndex].color = Color::White;
                edgesV[vertexIndex + 1].color = Color::White;
            }

            switch (action) 
            {
                case Action::OutputPrediction:
                    showActual = false;
                    break;
                case Action::OutputActual:
                    showActual = true;
                    break;
                case Action::OutputNext:
                    if (outputIndex != outputs.size() - 1)
                        outputIndex++;
                    break;
                case Action::OutputPrev:
                    if (outputIndex != 0)
                        outputIndex--;
                    break;
            }
        }
    }
}