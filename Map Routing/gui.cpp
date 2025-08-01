#include "gui.h"
#include "map_routing.h"
#include "file_io.h"

using namespace sf;

RenderWindow gui::window;
Event gui::event;

vector<Query> gui::queries;
vector<Output> gui::outputs;

Cursor gui::cursorArrow, gui::cursorHand, gui::cursorCross;

vector<string> gui::mapNames = { "Oldenburg", "San Joaquin", "San Francisco", "North America" };

vector<string> gui::mapPaths = {
    "TEST CASES\\[2] Medium Cases\\Input\\OLMap.txt",
    "TEST CASES\\[2] Medium Cases\\Input\\TGMap.txt",
    "TEST CASES\\[3] Large Cases\\Input\\SFMap.txt",
    "TEST CASES\\[3] Large Cases\\Input\\NAMap.txt"
};

vector<Graph> gui::maps;
vector<vector<Edge>> gui::mapsEdges;
vector<gui::MenuInfo> gui::menuInfos;

gui::Menu gui::menuSelected = Selection;
int gui::mapSelected = -1;

const int gui::WINDOW_WIDTH = VideoMode::getDesktopMode().width;
const int gui::WINDOW_HEIGHT = VideoMode::getDesktopMode().height;

const int gui::MAP_WIDTH = static_cast<int>(WINDOW_WIDTH * 0.78125);
const int gui::MAP_HEIGHT = static_cast<int>(WINDOW_HEIGHT * 0.972);

const int gui::LEFT_BOUNDARY = (MAP_WIDTH - WINDOW_WIDTH)/2;
const int gui::RIGHT_BOUNDARY = LEFT_BOUNDARY + WINDOW_WIDTH;
const int gui::UP_BOUNDARY = (MAP_HEIGHT - WINDOW_HEIGHT)/2;
const int gui::DOWN_BOUNDARY = UP_BOUNDARY + WINDOW_HEIGHT;

Font Buttons::font;

//buttons

Buttons::Buttons(vector<string> strings, RenderWindow& window) 
{
    font.loadFromFile("CONSOLA.TTF");
    Vector2f windowSize(window.getSize());

    backgroundSize = Vector2f(window.getSize().x * 0.25, window.getSize().y * 0.1);
    int characterSize = window.getSize().x * 0.025;
    size = strings.size();
    index = 0;

    texts.resize(size);
    backgrounds.resize(size);

    for (int i = 0; i < size; i++)
    {
        texts[i].setFont(font);
        texts[i].setString(strings[i]);
        texts[i].setCharacterSize(characterSize);
        texts[i].setFillColor(textColor);
        backgrounds[i].setSize(backgroundSize);
        backgrounds[i].setFillColor(backgroundColor);
        backgrounds[i].setOutlineColor(Color::Black);
        backgrounds[i].setOutlineThickness(backgrounds[i].getSize().y / 25);

        texts[i].setOrigin(Vector2f(texts[i].getLocalBounds().width * 0.5, texts[i].getLocalBounds().height * 0.8));
        backgrounds[i].setOrigin(Vector2f(backgroundSize.x / 2, backgroundSize.y / 2));

        texts[i].setPosition(Vector2f(windowSize.x * 0.5, windowSize.y * 0.3));
        backgrounds[i].setPosition(Vector2f(windowSize.x * 0.5, windowSize.y * 0.3));
    }

    for (int i = 1, move = backgroundSize.y + backgroundSize.y / 2; i < size; i++, move += backgroundSize.y + backgroundSize.y/2)
    {
        texts[i].setPosition(Vector2f(texts[i].getPosition().x, texts[i].getPosition().y + move));
        backgrounds[i].setPosition(Vector2f(backgrounds[i].getPosition().x, backgrounds[i].getPosition().y + move));
    }
}

void Buttons::hoverDetection(RenderWindow& window)
{
    Vector2f mousePos = Vector2f(Mouse::getPosition().x, Mouse::getPosition().y);
    for (int i = 0; i < size; i++)
    {
        if (backgrounds[i].getGlobalBounds().contains(mousePos))
        {
            index = i;
            texts[i].setFillColor(selectedTextColor);
            backgrounds[i].setFillColor(selectedBackgroundColor);
           
            window.setMouseCursor(gui::cursorHand);
            break;
        }
        
        index = -1;
        texts[i].setFillColor(textColor);
        backgrounds[i].setFillColor(backgroundColor);
    }

    if (index == -1)
        window.setMouseCursor(gui::cursorArrow);
}

void Buttons::clickDetection(int &selection)
{
    if (index == -1)
        return;

    if (Mouse::isButtonPressed(Mouse::Left))
    {
        selection = index;
    }
}

void Buttons::draw(RenderWindow& window)
{
    for (int i = 0; i < size; i++)
    {
        window.draw(backgrounds[i]);
        window.draw(texts[i]);
    }
}

void gui::loadMaps()
{
    maps.resize(4);
    mapsEdges.resize(4);

    for (int mapIndex = 0; mapIndex < 4; mapIndex++)
    {
        ifstream file;
        file.open(mapPaths[mapIndex]);

        int numOfNodes;
        file >> numOfNodes;
        maps[mapIndex].reserve(numOfNodes + 2);

        while (numOfNodes--)
        {
            int index;
            Node node{};

            file >> index >> node.x >> node.y;
            maps[mapIndex].push_back(node);
        }

        int numOfEdges;
        file >> numOfEdges;
        mapsEdges[mapIndex].reserve(numOfEdges);

        for (int i = 0; i < numOfEdges; i++)
        {
            int node1, node2;
            double speed;
            Edge edge{};

            file >> node1 >> node2 >> edge.length >> speed;

            edge.time = edge.length * 60.0 / speed;
            mapsEdges[mapIndex].push_back(edge);
            maps[mapIndex][node1].adjs.push_back(make_pair(node2, i));
            maps[mapIndex][node2].adjs.push_back(make_pair(node1, i));
        }

        file.close();
    }
}

void gui::calculateMenuVerticesThread(int mapIndex)
{
    menuInfos[mapIndex].nodesDrawn.assign(maps[mapIndex].size(), -1);
    menuInfos[mapIndex].edgesDrawn.assign(mapsEdges[mapIndex].size(), -1);

    MenuInfo& info = menuInfos[mapIndex];
    Graph& map = maps[mapIndex];
    vector<Edge>& edges = mapsEdges[mapIndex];

    for (const auto& node : map)
    {
        if (node.x < info.xMin)
            info.xMin = node.x;
        if (node.x > info.xMax)
            info.xMax = node.x;

        if (node.y < info.yMin)
            info.yMin = node.y;
        if (node.y > info.yMax)
            info.yMax = node.y;
    }

    info.width = info.xMax - info.xMin;
    info.height = info.yMax - info.yMin;

    // Creating the normalized lines
    info.vertexCount = edges.size() * 2;
    info.edgeVertices = new Vertex[info.vertexCount];

    int index = 0;
    int nodeIndex = 0;
    for (const Node& node : map)
    {
        for (const auto& adj : node.adjs)
        {
            if (info.edgesDrawn[adj.second] == -1)
            {
                info.edgeVertices[index++] = Vertex(Vector2f((node.x - info.xMin) / info.width, (node.y - info.yMin) / info.height), Color(200, 200, 200, 255));
                info.edgeVertices[index++] = Vertex(Vector2f((map[adj.first].x - info.xMin) / info.width, (map[adj.first].y - info.yMin) / info.height), Color::White);

                info.nodesDrawn[nodeIndex] = index - 2;
                info.nodesDrawn[adj.first] = index - 1;
                info.edgesDrawn[adj.second] = index - 2;
            }
        }

        nodeIndex++;
    }

    // Denormalizing to screen coordinates
    for (int i = 0; i < info.vertexCount; i++)
    {
        info.edgeVertices[i].position.x *= MAP_WIDTH;
        info.edgeVertices[i].position.y = MAP_HEIGHT - (info.edgeVertices[i].position.y * MAP_HEIGHT);
    }
}

void gui::calculateMenuVertices()
{
    menuInfos.resize(4);

    vector<thread> threads;

    for (int mapIndex = 0; mapIndex < 4; mapIndex++)
        threads.emplace_back(calculateMenuVerticesThread, mapIndex);

    for (auto& t : threads)
        t.join();
}

void gui::uninitialize()
{
    for (int i = 0; i < 4; i++)
    {
        delete[] menuInfos[i].edgeVertices;
        menuInfos[i].edgeVertices = nullptr;
    }
}

void gui::initialization()
{
    loadMaps();
    calculateMenuVertices();

    ContextSettings settings;
    settings.antialiasingLevel = 8;

    window.create(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Map Routing", Style::Fullscreen, settings);
    //window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);
    cursorArrow.loadFromSystem(Cursor::Arrow);
    cursorHand.loadFromSystem(Cursor::Hand);
    cursorCross.loadFromSystem(Cursor::Cross);
    
    //testing();
    while (true)
    {
        switch (menuSelected)
        {
            case Selection: 
            {
                mapSelectionMenu();
                break;
            }
            case Routing:
            {
                mapRoutingMenu();
                file_io::saveOutputs(queries, outputs, mapNames[mapSelected]);
                queries.clear();
                outputs.clear();
                mapSelected = -1;

                break;
            }
        }

        if (menuSelected == Exit)
            break;
    }

    window.close();
    uninitialize();
}

//gui
void gui::mapSelectionMenu() 
{
    View view(Vector2f(0, 0), Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    window.setView(window.getDefaultView());

    Texture bgTxt;
    bgTxt.loadFromFile("textures/bg.jpg");
    Sprite bg(bgTxt);

    Buttons buttons(mapNames, window);
 
    while (window.isOpen())
    {
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
            {
                menuSelected = Exit;
                break;
            }

            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)
            {
                menuSelected = Exit;
                break;
            }
        }

        if (menuSelected != Selection)
            break;

        buttons.hoverDetection(window);
        buttons.clickDetection(mapSelected);

        if (mapSelected != -1)
        {
            menuSelected = Routing;
            window.setMouseCursor(cursorArrow);
            break;
        }

        window.clear();
        window.draw(bg);
        buttons.draw(window);
        window.display();
    }
}

void gui::mapRoutingMenu()
{
    View viewMap(Vector2f(MAP_WIDTH / 2.f, MAP_HEIGHT / 2.f), Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    View viewUI(Vector2f(MAP_WIDTH / 2.f, MAP_HEIGHT / 2.f), Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    window.setView(viewMap);

    MenuInfo& info = menuInfos[mapSelected];
    Graph map(maps[mapSelected]);
    vector<Edge> edges(mapsEdges[mapSelected]);

    CircleShape startCircle, destCircle;
    startCircle.setFillColor(Color::Transparent);
    startCircle.setOutlineColor(Color::Green);
    startCircle.setOutlineThickness(2.f);
    destCircle.setFillColor(Color::Transparent);
    destCircle.setOutlineColor(Color::Red);
    destCircle.setOutlineThickness(2.f);

    startCircle.setRadius((50000.f * MAP_WIDTH) / (1000.f * info.width));
    startCircle.setScale(Vector2f(0.f, 0.f));
    startCircle.setOrigin(Vector2f(startCircle.getRadius(), startCircle.getRadius()));

    destCircle.setRadius(startCircle.getRadius());
    destCircle.setScale(startCircle.getScale());
    destCircle.setOrigin(Vector2f(destCircle.getRadius(), destCircle.getRadius()));

    float zoom = 1.f;
    bool getPath = false;
    bool isDrawing = false;
    bool isDragging = false;

    Output output;
    Query query;
    query.radius = 50000.f;

    Vector2f oldPos;
    Vertex* walkPath = new Vertex[4];

    window.setView(viewUI);
    unordered_set<char> numbers = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
    bool isTyping = false;
    float charSize = static_cast<float>(window.getSize().x * 0.025);

    Font font;
    font.loadFromFile("CONSOLA.TTF");

    Text textRadius("Radius: ", font, charSize), textTime("", font, charSize*0.8), 
        textDist("", font, charSize*0.8), textDistWalk("", font, charSize*0.8), 
        textDistDrive("", font, charSize*0.8);

    textRadius.setFillColor(Color::Red);
    textTime.setFillColor(Color::Red);
    textDist.setFillColor(Color::Red);
    textDistWalk.setFillColor(Color::Red);
    textDistDrive.setFillColor(Color::Red);
    
    textRadius.setOutlineThickness(2);
    textTime.setOutlineThickness(2);
    textDist.setOutlineThickness(2);
    textDistWalk.setOutlineThickness(2);
    textDistDrive.setOutlineThickness(2);


    textRadius.setPosition(Vector2f((MAP_WIDTH - WINDOW_WIDTH) / 2.f, (MAP_HEIGHT - WINDOW_HEIGHT) / 2.f));
    textTime.setPosition(textRadius.getPosition().x, textRadius.getPosition().y + charSize*1.5);
    textDist.setPosition(textTime.getPosition().x, textTime.getPosition().y + charSize*1.5);
    textDistWalk.setPosition(textDist.getPosition().x, textDist.getPosition().y + charSize*1.5);
    textDistDrive.setPosition(textDistWalk.getPosition().x, textDistWalk.getPosition().y + charSize*1.5);

    string radius = "50";
    Text textRadiusValue(radius, font, charSize);
    textRadiusValue.setPosition(textRadius.getPosition().x + textRadius.getGlobalBounds().width, textRadius.getPosition().y);
    textRadiusValue.setFillColor(Color::Red);

    FloatRect bgRectRadius = textRadiusValue.getGlobalBounds();
    RectangleShape bgRadius(Vector2f(bgRectRadius.width, bgRectRadius.height));
    bgRadius.setFillColor(Color(50, 0, 0, 200));
    bgRadius.setPosition(textRadiusValue.getPosition());

    while (window.isOpen())
    {
        while (window.pollEvent(event))
        {
            window.setView(viewMap);
            Vector2f mousePosMap = Vector2f(event.mouseButton.x, event.mouseButton.y);
            Vector2f mouseCoordsMap = window.mapPixelToCoords(Vector2i(mousePosMap));

            window.setView(viewUI);
            Vector2f mousePosUI = Vector2f(event.mouseButton.x, event.mouseButton.y);
            Vector2f mouseCoordsUI = window.mapPixelToCoords(Vector2i(mousePosUI));
            
            window.setView(viewMap);

            if (event.type == Event::Closed)
            {
                menuSelected = Exit;
                break;
            }
            else if (event.type == Event::KeyPressed)
            {
                if (event.key.code == Keyboard::Escape)
                {
                    menuSelected = Selection;
                    break;
                }

                if (event.key.code == Keyboard::Enter)
                {
                    isTyping = false;
                    if (radius == "0")
                    {
                        radius = "1";
                        textRadiusValue.setString(radius);
                    }

                    query.radius = stoi(radius) * 1000.f;

                    startCircle.setRadius((query.radius * MAP_WIDTH) / (1000.f * info.width));
                    startCircle.setOrigin(Vector2f(startCircle.getRadius(), startCircle.getRadius()));
                    destCircle.setRadius(startCircle.getRadius());
                    destCircle.setOrigin(Vector2f(destCircle.getRadius(), destCircle.getRadius()));

                    getPath = (startCircle.getScale() == Vector2f(1.0, (MAP_HEIGHT / info.height) / (MAP_WIDTH / info.width)) &&
                        startCircle.getScale() == destCircle.getScale());
                }

                if (isTyping && event.key.code == Keyboard::BackSpace)
                {
                    if (radius.size())
                        radius.pop_back();

                    if (radius.empty())
                        radius = "0";
                }
            }
            else if (event.type == Event::MouseWheelMoved)
            {
                if (!isDragging)
                {
                    float zoomTemp = 1.f;
                    zoomTemp -= static_cast<float>(event.mouseWheel.delta) / 10.f;
                    zoom *= zoomTemp;

                    zoom = min(1.f, zoom);

                    if (zoom == 1.f)
                    {
                        zoomTemp = 1.f;
                        viewMap.setSize(Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
                    }

                    viewMap.zoom(zoomTemp);

                    Vector2f viewPos = viewMap.getCenter();
                    Vector2f viewSize = viewMap.getSize() / 2.f;

                    Vector2f newViewPos = viewPos;

                    if (viewPos.x - viewSize.x < LEFT_BOUNDARY)
                        newViewPos.x = LEFT_BOUNDARY + viewSize.x;
                    else if (viewPos.x + viewSize.x > RIGHT_BOUNDARY)
                        newViewPos.x = RIGHT_BOUNDARY - viewSize.x;

                    if (viewPos.y - viewSize.y < UP_BOUNDARY)
                        newViewPos.y = UP_BOUNDARY + viewSize.y;
                    else if (viewPos.y + viewSize.y > DOWN_BOUNDARY)
                        newViewPos.y = DOWN_BOUNDARY - viewSize.y;

                    viewMap.setCenter(newViewPos);

                    window.setView(viewMap);
                }
            }
            else if (event.type == Event::MouseButtonPressed)
            {
                if (event.key.code == Mouse::Middle)
                {
                    isDragging = true;
                    oldPos = mousePosMap;
                }
                
                if (event.key.code == Mouse::Left)
                {
                    window.setView(viewUI);
                    if (bgRadius.getGlobalBounds().contains(mouseCoordsUI))
                    {
                        isTyping = true;
                    }
                    else if (isTyping)
                    {
                        isTyping = false;
                        if (radius == "0")
                        {
                            radius = "1";
                            textRadiusValue.setString(radius);
                        }
                        
                        query.radius = stoi(radius) * 1000.f;

                        startCircle.setRadius((query.radius * MAP_WIDTH) / (1000.f * info.width));
                        startCircle.setOrigin(Vector2f(startCircle.getRadius(), startCircle.getRadius()));
                        destCircle.setRadius(startCircle.getRadius());
                        destCircle.setOrigin(Vector2f(destCircle.getRadius(), destCircle.getRadius()));

                        getPath = (startCircle.getScale() == Vector2f(1.0, (MAP_HEIGHT / info.height) / (MAP_WIDTH / info.width)) &&
                            startCircle.getScale() == destCircle.getScale());
                    }
                    else
                    {
                        window.setView(viewMap);
                        
                        if (startCircle.getPosition() != mouseCoordsMap)
                        {
                            startCircle.setScale(Vector2f(1.0, (MAP_HEIGHT / info.height) / (MAP_WIDTH / info.width)));
                            startCircle.setPosition(mouseCoordsMap);

                            query.startX = (startCircle.getPosition().x / MAP_WIDTH) * info.width + info.xMin;
                            query.startY = ((MAP_HEIGHT - startCircle.getPosition().y) / MAP_HEIGHT) * info.height + info.yMin;

                            getPath = (startCircle.getScale() == destCircle.getScale());
                        }
                        else
                        {
                            startCircle.setScale(Vector2f(0.f, 0.f));
                            startCircle.setPosition(Vector2f(0.f, 0.f));
                        }
                    }
                    window.setView(viewMap);
                }
                
                if (event.key.code == Mouse::Right)
                {
                    if (destCircle.getPosition() != mouseCoordsMap)
                    {
                        destCircle.setScale(Vector2f(1.0, (MAP_HEIGHT / info.height) / (MAP_WIDTH / info.width)));
                        destCircle.setPosition(mouseCoordsMap);

                        query.destX = (destCircle.getPosition().x / MAP_WIDTH) * info.width + info.xMin;
                        query.destY = ((MAP_HEIGHT - destCircle.getPosition().y) / MAP_HEIGHT) * info.height + info.yMin;

                        getPath = (startCircle.getScale() == destCircle.getScale());
                    }
                    else
                    {
                        destCircle.setScale(Vector2f(0.f, 0.f));
                        destCircle.setPosition(Vector2f(0.f, 0.f));
                    }
                }
            }
            else if (event.type == Event::MouseButtonReleased && event.key.code == Mouse::Middle)
                isDragging = false;
            else if (event.type == Event::MouseMoved)
            {
                if (isDragging)
                {
                    Vector2f newPos = Vector2f(event.mouseMove.x, event.mouseMove.y);
                    viewMap.move((oldPos - newPos) * zoom);

                    Vector2f viewPos = viewMap.getCenter();
                    Vector2f viewSize = viewMap.getSize() / 2.f;

                    Vector2f newViewPos = viewPos;

                    if (viewPos.x - viewSize.x < LEFT_BOUNDARY)
                        newViewPos.x = LEFT_BOUNDARY + viewSize.x;
                    else if (viewPos.x + viewSize.x > RIGHT_BOUNDARY)
                        newViewPos.x = RIGHT_BOUNDARY - viewSize.x;

                    if (viewPos.y - viewSize.y < UP_BOUNDARY)
                        newViewPos.y = UP_BOUNDARY + viewSize.y;
                    else if (viewPos.y + viewSize.y > DOWN_BOUNDARY)
                        newViewPos.y = DOWN_BOUNDARY - viewSize.y;

                    viewMap.setCenter(newViewPos);
                    window.setView(viewMap);
                    oldPos = newPos;
                }
            }
            else if (isTyping && event.type == Event::TextEntered)
            {
                if (numbers.count(event.text.unicode))
                {
                    if (radius != "0")
                    {
                        radius += event.text.unicode;

                        if (stoi(radius) > 9999)
                            radius = to_string(9999);
                    }
                    else
                    {
                        if (event.text.unicode != '0')
                        {
                            radius = "";
                            radius += event.text.unicode;
                        }
                    }
                }
            }
        }

        if (menuSelected != Routing)
        {
            vector<int>& nodeIDs = output.nodeIDs;
            int nodeCount = nodeIDs.size();

            for (int i = 1; i < nodeCount; i++)
            {
                int vertexIndex = info.edgesDrawn[map[nodeIDs[i - 1]].getEdgeID(nodeIDs[i])];
                info.edgeVertices[vertexIndex].color = Color(200, 200, 200, 255);
                info.edgeVertices[vertexIndex + 1].color = Color(200, 200, 200, 255);
            }

            for (int i = 0; i < 4; i++)
                walkPath[i].color = Color(255, 255, 0, 0);

            break;
        }

        if (isDrawing && (startCircle.getScale() != destCircle.getScale()))
        {
            textTime.setString("");
            textDist.setString("");
            textDistWalk.setString("");
            textDistDrive.setString("");

            vector<int>& nodeIDs = output.nodeIDs;
            int nodeCount = nodeIDs.size();

            for (int i = 1; i < nodeCount; i++)
            {
                int vertexIndex = info.edgesDrawn[map[nodeIDs[i - 1]].getEdgeID(nodeIDs[i])];
                info.edgeVertices[vertexIndex].color = Color(200, 200, 200, 255);
                info.edgeVertices[vertexIndex + 1].color = Color(200, 200, 200, 255);
            }

            for (int i = 0; i < 4; i++)
                walkPath[i].color = Color(255, 255, 0, 0);

            isDrawing = false;
        }

        if (getPath)
        {
            for (int i = 0; i < 4; i++)
                walkPath[i].color = Color(255, 255, 0, 0);

            textTime.setString("");
            textDist.setString("");
            textDistWalk.setString("");
            textDistDrive.setString("");

            vector<int>& nodeIDs = output.nodeIDs;
            int nodeCount = nodeIDs.size();
            
            if (nodeCount)
            {
                for (int i = 1; i < nodeCount; i++)
                {
                    int vertexIndex = info.edgesDrawn[map[nodeIDs[i - 1]].getEdgeID(nodeIDs[i])];
                    info.edgeVertices[vertexIndex].color = Color(200, 200, 200, 255);
                    info.edgeVertices[vertexIndex + 1].color = Color(200, 200, 200, 255);
                }
            }

            output = map_routing::solveQuery(query, map, edges);

            nodeIDs = output.nodeIDs;
            nodeCount = nodeIDs.size();

            if (nodeCount)
            {
                for (int i = 1; i < nodeCount; i++)
                {
                    int vertexIndex = info.edgesDrawn[map[nodeIDs[i - 1]].getEdgeID(nodeIDs[i])];
                    info.edgeVertices[vertexIndex].color = Color::Red;
                    info.edgeVertices[vertexIndex + 1].color = Color::Red;
                }

                walkPath[0].color = Color(255, 255, 0, 255);
                walkPath[0].position = startCircle.getPosition();

                walkPath[1].color = Color(255, 255, 0, 255);
                walkPath[1].position = info.edgeVertices[info.nodesDrawn[nodeIDs[0]]].position;

                walkPath[2].color = Color(255, 255, 0, 255);
                walkPath[2].position = info.edgeVertices[info.nodesDrawn[nodeIDs[nodeCount - 1]]].position;

                walkPath[3].color = Color(255, 255, 0, 255);
                walkPath[3].position = destCircle.getPosition();

                textTime.setString("Time (min): " + to_string(lround(output.time)));
                textDist.setString("Distance (km): " + to_string(lround(output.distTotal)));
                textDistWalk.setString("Walking (km): " + to_string(lround(output.distWalk)));
                textDistDrive.setString("Driving (km): " + to_string(lround(output.distDrive)));
                
                isDrawing = true;

                queries.push_back(query);
                outputs.push_back(output);
            }

            getPath = false;
        }

        window.setView(viewUI);

        Vector2f mousePosUI = Vector2f(Mouse::getPosition());
        Vector2f mouseCoordsUI = window.mapPixelToCoords(Vector2i(mousePosUI));
        if (bgRadius.getGlobalBounds().contains(mouseCoordsUI))
            bgRadius.setFillColor(Color(25, 0, 0, 200));
        else
            bgRadius.setFillColor(Color(50, 0, 0, 200));

        if (isTyping)
            bgRadius.setFillColor(Color(25, 0, 0, 200));

        textRadiusValue.setString(radius);
        bgRectRadius = textRadiusValue.getGlobalBounds();
        bgRadius.setSize(Vector2f(bgRectRadius.width + (!radius.empty()) * charSize / 3, bgRectRadius.height + (!radius.empty()) * charSize / 1.5));
        bgRadius.setPosition(Vector2f(textRadiusValue.getPosition().x - (!radius.empty()) * charSize / 10, textRadiusValue.getPosition().y));
        
        
        window.clear();

        window.setView(viewMap);
        window.draw(info.edgeVertices, info.vertexCount, Lines);
        window.draw(walkPath, 4, Lines);
        window.draw(startCircle);
        window.draw(destCircle);

        window.setView(viewUI);
        window.draw(textRadius);
        window.draw(bgRadius);
        window.draw(textRadiusValue);
        window.draw(textTime);
        window.draw(textDist);
        window.draw(textDistWalk);
        window.draw(textDistDrive);

        window.display();
    }
}

void gui::testing()
{
    View view(Vector2f(WINDOW_WIDTH/2, WINDOW_HEIGHT/2), Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));

    unordered_set<char> numbers = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
    bool isTyping = false;

    string radius = "0";
    Font font;
    font.loadFromFile("CONSOLA.TTF");

    Text text(radius, font, window.getSize().x * 0.025);
    text.setPosition(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
    text.setFillColor(Color::Red);

    FloatRect bgRect = text.getGlobalBounds();
    RectangleShape bg(Vector2f(bgRect.width, bgRect.height));
    bg.setFillColor(Color(50, 0, 0, 255));
    bg.setPosition(text.getPosition());
    

    window.setView(view);
    while (window.isOpen())
    {
        while (window.pollEvent(event))
        {
            if (event.type == Event::KeyPressed)
            {
                if (event.key.code == Keyboard::Enter)
                    isTyping = false;
                
                if (event.key.code == Keyboard::BackSpace)
                {
                    if (radius.size())
                        radius.pop_back();

                    if (radius.empty())
                        radius = "0";
                }
            }

            if (event.type == Event::MouseButtonPressed && event.key.code == Mouse::Left)
            {
                if (bg.getGlobalBounds().contains(Vector2f(Mouse::getPosition())))
                    isTyping = true;
                else
                {
                    isTyping = false;
                }
            }

            if (isTyping && event.type == Event::TextEntered)
            {
                if (numbers.count(event.text.unicode))
                {
                    if (radius != "0")
                    {
                        radius += event.text.unicode;

                        if (stoi(radius) > 1000)
                            radius = to_string(1000);
                    }
                    else
                    {
                        if (event.text.unicode != '0')
                        {
                            radius = "";
                            radius += event.text.unicode;
                        }
                    }
                }
            }
        }

        if (bg.getGlobalBounds().contains(Vector2f(Mouse::getPosition())))
            bg.setFillColor(Color(25, 0, 0, 255));
        else
            bg.setFillColor(Color(50, 0, 0, 255));

        if (isTyping)
            bg.setFillColor(Color(25, 0, 0, 255));

        text.setString(radius);
        bgRect = text.getGlobalBounds();
        float charSize = static_cast<float>(text.getCharacterSize());
        bg.setSize(Vector2f(bgRect.width + (!radius.empty()) * charSize / 3, bgRect.height + (!radius.empty()) * charSize/1.5));
        bg.setPosition(Vector2f(text.getPosition().x - (!radius.empty()) * charSize / 10, text.getPosition().y));

        window.clear();
        window.draw(bg);
        window.draw(text);
        window.display();
    }
}