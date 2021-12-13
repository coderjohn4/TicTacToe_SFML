//-lsfml-graphics -lsfml-window -lsfml-system -Wfatal-errors -lsfml-network -Wfatal-errors
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <list>
using namespace std;

enum Player{
    X,
    O
};

enum PacketType{
    PlayerType,
    PlayerTurnStart,
    SetPieceLocationClient,
    WinnerFound,
    SetPieceLocation
};

int main(){
    
    int tttboard[3][3];
    for(int x = 0; x < 3; x++){
        for(int y = 0; y<3; y++){
            tttboard[x][y] = 0;
        }
    } 


    sf::TcpListener listener;
   
    if(listener.listen(55120) != sf::Socket::Done){
        return -1;
        cout << "[ERROR CANT START SERVER]" << endl;
    };

    cout << "Server listening on port 55120" << endl;
    
    sf::SocketSelector selector;
    
    selector.add(listener);
    
    list<sf::TcpSocket*> clients;
    
    sf::TcpSocket* socketPlayerX;
    sf::TcpSocket* socketPlayerO;

    bool running = true;
    bool bStartGame = false; 
    bool bInitClients = false;

    Player whoIsPlaying = Player::X;

    
    while(running){
    
        if(selector.wait()){

            cout << "Activity.." << endl;
            
            if(selector.isReady(listener)){
                
                sf::TcpSocket* newClient = new sf::TcpSocket;
                cout << "Listener Activity.." << endl;
               
                if(listener.accept(*newClient) == sf::Socket::Done){
                    cout << "New Client" << endl;
                    selector.add(*newClient);
                    clients.push_back(newClient);
                    cout << "Clients size: " << clients.size() << endl; 
                    if(clients.size() == 2){
                        bInitClients = true;
                        list<sf::TcpSocket*>::iterator it = clients.begin();
                        socketPlayerX = (sf::TcpSocket*)*it;
                        advance(it, 1);
                        socketPlayerO = (sf::TcpSocket*)*it;

                    };
                }

            }  
            else{

                cout << "Other Network Activity.." << endl;
                
                for(list<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it){
                    sf::TcpSocket* currentClient = *it;
                    
                    if(selector.isReady(*currentClient)){
                        
                        sf::Packet packet;
                        sf::Socket::Status receiveStatus;
        
                        receiveStatus = currentClient->receive(packet);
                    
                        if(receiveStatus == sf::Socket::Disconnected){
                            cout << "Client disconnected!" << endl;
                            selector.remove(*currentClient);
                            currentClient->disconnect();
                            delete(currentClient); 
                            it = clients.erase(it);
                            cout << "Clients size: " << clients.size() << endl; 
                        }

                        if(receiveStatus == sf::Socket::Done){

                            int packetType;
                            cout << "Client Data Received" << endl;
                            packet >> packetType;
                            cout << "Packettype: " << packetType << endl;
                            
                            if(packetType == PacketType::SetPieceLocation){
                                int idxX,idxY;
                                packet >> idxX >> idxY;
                                cout << "SetPiecetLocation, idxX: " << idxX << ", idxY: " << idxY << ", player: " << whoIsPlaying << endl;
                                int PieceTypeNo;
                                if(whoIsPlaying == Player::X){
                                   PieceTypeNo = 1;
                                }
                                else if(whoIsPlaying == Player::O){
                                    PieceTypeNo = 2;
                                };    
                                tttboard[idxX][idxY] = PieceTypeNo;
                                //Send piece location update to both Clients
                                packet.clear();
                                packet << PacketType::SetPieceLocationClient << idxX << idxY << PieceTypeNo;

                                socketPlayerO->send(packet);
                                socketPlayerX->send(packet);

                                //Switch user
                                if(whoIsPlaying == Player::X){
                                   whoIsPlaying = Player::O;
                                }
                                else if(whoIsPlaying == Player::O){
                                   whoIsPlaying = Player::X;
                                }; 

                                //Send you are    
                                packet.clear();
                
                                packet << PacketType::PlayerTurnStart;
                
                                if(whoIsPlaying == Player::X) 
                                    socketPlayerX->send(packet);
                                
                                if(whoIsPlaying == Player::O) 
                                    socketPlayerO->send(packet);

    

                            }

                        }

                    };

                };
            };

            if(bInitClients){
                sf::Packet packet;
                
                packet << PacketType::PlayerType << Player::X;
                socketPlayerX->send(packet);
                
                packet.clear();
                
                packet << PacketType::PlayerType << Player::O;
                socketPlayerO->send(packet);
                
                bInitClients = false;
                bStartGame = true;

                packet.clear();
                
                packet << PacketType::PlayerTurnStart;
                
                if(whoIsPlaying == Player::X) 
                    socketPlayerX->send(packet);
                
                if(whoIsPlaying == Player::O) 
                    socketPlayerO->send(packet);

            }


            if(bStartGame == true) {



            }

        };

    };
    
    return 0;
    
}
