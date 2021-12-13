//-lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -Wfatal-errors 
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <unistd.h>
using namespace std;

enum Player{
    X,
    O
};

enum PacketType{
    PlayerType,     // PacketType, PlayerType
    PlayerTurnStart,
    SetPieceLocationClient,
    WinnerFound,
    SetPieceLocation,
};

void showString(sf::RenderWindow &window, sf::Font& font, std::string text, sf::Vector2f position, sf::Color color)
{ 
    sf::Text myText(text, font);
    myText.setCharacterSize(20);
    myText.setFillColor(color);
    myText.setPosition(position);
    window.draw(myText);
}

int main(){

    bool playerXWon = false;
    bool playerOWon = false;

    sf::TcpSocket socket;
    sf::Socket::Status status;
    int packetType;
    status = socket.connect("127.0.0.1", 55120);

    sf::Packet packet;
    
    socket.receive(packet);
    int iPlayerType;    
    packet >> packetType >> iPlayerType;
    cout << "Player type no.: " << iPlayerType << endl;
    if(iPlayerType == Player::X){
        cout << "PlayerX" << endl;
    }
    else if(iPlayerType == Player::O){
        cout << "PlayerO" << endl;
    }

    socket.setBlocking(false);   

    sf::RenderWindow window(sf::VideoMode(400, 400), "Tic Tac Toe");
    
    sf::Font myFont;
    if (!myFont.loadFromFile("arial.ttf"))
    {
        cout << "Error loading file \"arial.ttf\"";
        return 0;
        
    }
    
    sf::RectangleShape sqOutline;
    sqOutline.setFillColor(sf::Color::Transparent);
    sqOutline.setOutlineColor(sf::Color::Black);
    sqOutline.setOutlineThickness(1.5);
    sqOutline.setSize(sf::Vector2f(100,100));
    
    sf::Texture xotexure;
    xotexure.setSmooth(true);
    if (!xotexure.loadFromFile("xo.png")) std::cout << "[ERROR]" << endl;    

    sf::Sprite spriteX;
    sf::Sprite spriteO;
    spriteX.setTexture(xotexure);
    spriteX.setTextureRect(sf::IntRect(10,0,440,486));            
    spriteX.setScale(0.2f, 0.2f);
    spriteO.setTexture(xotexure);
    spriteO.setTextureRect(sf::IntRect(450,0,440,486));            
    spriteO.setScale(0.2f, 0.2f);

    int tttboard[3][3];
    for(int x = 0; x < 3; x++){
        for(int y = 0; y<3; y++){
            tttboard[x][y] = 0;
        }
    } 

    bool isMyTurn = false;
    
    while(window.isOpen())  
    {
        sf::Event ev;
        while(window.pollEvent(ev))
        {
            if(ev.type == sf::Event::Closed) window.close();
            
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::N))
            {   
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
            {
                window.close();
            }
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {   
                if(isMyTurn == true) {
                    sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
                    if(mousePosition.x > 50 && mousePosition.x < 350 && mousePosition.y > 50 && mousePosition.y < 350){
                        
                        int idxX = (mousePosition.x -50) /100;
                        int idxY = (mousePosition.y -50) /100;

                        if(idxX >= 0 && idxX < 3 && idxY >= 0 && idxY < 3){
                            if(tttboard[idxX][idxY] == 0){
                                if(iPlayerType == Player::X){
                                    tttboard[idxX][idxY] = 1;
                                }
                                else if(iPlayerType == Player::O){
                                    tttboard[idxX][idxY] = 2;
                                };

                                //Send pice information to server
                                packet.clear(); 
                                packet << PacketType::SetPieceLocation << idxX << idxY;
                                socket.send(packet);

                                isMyTurn = false;
                                
                            }
                        }
                    }
                }
            }

        }

        if(socket.receive(packet) == sf::Socket::Done){
            cout << "Packet recieved" << endl;
            packet >> packetType;
            cout << "Packettype: " << packetType << endl;
            
            //The player can place a piece 
            if(packetType == PacketType::PlayerTurnStart){
                cout << "It's my turn: " << iPlayerType << endl;
                isMyTurn = true;
            }

            //Set piece SetPieceLocation
            if(packetType == PacketType::SetPieceLocationClient){
                cout << "Place piece: " << iPlayerType << endl;
                int pieceTypeNo, idxX, idxY;
                packet >> idxX >> idxY >> pieceTypeNo;
                cout << "SetPiecetLocation, idxX: " << idxX << ", idxY: " << idxY << ", player: " << pieceTypeNo << endl;
                tttboard[idxX][idxY] = pieceTypeNo;
            }

        }

        window.clear(sf::Color::White);
        //cout << "------" <<endl;
        for(int x = 0; x < 3; x++){
            for(int y = 0; y<3; y++){
                sqOutline.setPosition(x*100+50, y*100+50);
                window.draw(sqOutline);
                if(tttboard[x][y] == 1){
                    spriteX.setPosition(x*100 +60, y*100 +52);
                    window.draw(spriteX);
                };
                if(tttboard[x][y] == 2){
                    spriteO.setPosition(x*100 +60, y*100 +52);
                    window.draw(spriteO);
                };
                //cout << tttboard[x][y] << " ";
            }
            //cout << endl;
        } 
        if(isMyTurn) {
            sf::RectangleShape shape;
            shape.setPosition(90,0);
            shape.setSize(sf::Vector2f(300,50));
            shape.setFillColor(sf::Color::Black);
            window.draw(shape);
            showString(window, myFont, "It's your turn :-)", sf::Vector2f(100,10), sf::Color::Red);
        }
        if(playerXWon){
            sf::RectangleShape shape;
            shape.setPosition(90,90);
            shape.setSize(sf::Vector2f(300,50));
            shape.setFillColor(sf::Color::Black);
            window.draw(shape);
            showString(window, myFont, "Congratulations Winner 'X'!", sf::Vector2f(100,100), sf::Color::Red);
            isMyTurn = false;
        }
        if(playerOWon){
            sf::RectangleShape shape;
            shape.setPosition(90,90);
            shape.setSize(sf::Vector2f(300,50));
            shape.setFillColor(sf::Color::Black);
            window.draw(shape);
            showString(window, myFont, "Congratulations Winner 'O'!", sf::Vector2f(100,100), sf::Color::Blue);
            isMyTurn = false;
        }

        // spriteO.setPosition(160+100,152);
        // window.draw(spriteO);
        window.display();


    };

    return 0;
}