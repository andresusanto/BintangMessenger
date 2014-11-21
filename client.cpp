#include <iostream>
#include <thread>  
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <algorithm>

#define MAXHOSTNAME 256

#include "Helper.h"

using namespace std;
char clientState;
int buatSocket();

main()
{
	clientState = 'A';	// state inisiasi -> cuma bisa login
	
	system("clear");
	
	int rc = 0;
	char buf[512];
	string tmp;
	cout << "           ______  _____ __   _ _______ _______ __   _  ______\n";
	cout << "           |_____]   |   | \\  |    |    |_____| | \\  | |  ____\n";
	cout << "           |_____] __|__ |  \\_|    |    |     | |  \\_| |_____|\n";
	cout << " _______ _______ _______ _______ _______ __   _  ______ _______  ______\n";
	cout << " |  |  | |______ |______ |______ |______ | \\  | |  ____ |______ |_____/\n";
	cout << " |  |  | |______ ______| ______| |______ |  \\_| |_____| |______ |    \\_\n\n";
	cout << "Welcome to Bintang Messenger!\nYou can \e[1mlogin\e[0m or \e[1msignup\e[0m\n\n"; 
	int socketHandle = buatSocket();
	
	while (clientState != 'X') {
		switch(clientState){
			case 'A'	:{	// state A adalah state saat pertama kali program dijalankan (run as guest)
			
							cout << "[guest]$ ";
							getline (cin,tmp);
							
							if (tmp.compare("login") == 0){
								string l_user, l_password;
								cout << "Username : ";
								getline (cin,l_user);
								cout << "Password : \e[30;40m";
								getline (cin,l_password);
								cout << "\e[0m";
								
								string msg_send = "LOGIN " + l_user + " " + l_password;
								strcpy(buf, msg_send.c_str());
								cout << "Logging in ...\n";
								send(socketHandle, buf, strlen(buf)+1, 0);
								rc = recv(socketHandle, buf, 512, 0);
								
								if (rc > 0){
									if(((string)"LOGINOK").compare(buf) == 0){
										
										clientState = 'P';
									}else if(((string)"LOGINNO").compare(buf) == 0){
										cout << "\e[31mAccess Denied!\e[39m\n";
									}
								}else{
									cerr << "Disconected!" << endl;
									exit(EXIT_FAILURE);
								}
								
							}else if(tmp.compare("signup") == 0){
								string l_user, l_password;
								cout << "Pick a Username : \e[1;33m";
								getline (cin,l_user);
								cout << "\e[0mPick a Password : \e[30;40m";
								getline (cin,l_password);
								cout << "\e[0m";
								
								string msg_send = "SIGNUP " + l_user + " " + l_password;
								strcpy(buf, msg_send.c_str());
								cout << "Registering ...\n";
								send(socketHandle, buf, strlen(buf)+1, 0);
								rc = recv(socketHandle, buf, 512, 0);
								
								if (rc > 0){
									if(((string)"SIGNUPOK").compare(buf) == 0){
										cout << "\e[32mRegistration \e[1mSuccess!\e[0m\nYou may now login with your new account!\n";
									}else if(((string)"SIGNUPNO").compare(buf) == 0){
										cout << "\e[31mRegistration Failed!\e[39m\nPlease try another username!\n";
									}
								}else{
									cerr << "Disconected!" << endl;
									exit(EXIT_FAILURE);
								}
								
							}else{
								cout << "Invalid command!\n";
							}
						}
						break;
						
			case 'P'	:{	// state P adalah state yang digunakan untuk memproses pesan offline yang disimpan di server
							bool offlineMsgHabis = false;
							while ( !offlineMsgHabis ) {
								cout << "masup lup" << "\n";
								rc = recv(socketHandle, buf, 512, 0);
								
								cout << buf << "\n";
								vector<string> query = Helper::split((string)buf, ' ');
								if (query[0].compare("OK") == 0){
									clientState = 'A';
									offlineMsgHabis = true;
								}else if(query[0].compare("MSG") == 0){
									cout << " MSG " << query[1] << "\n";
								}else if(query[0].compare("MSGGROUP") == 0){
									cout << " MSG GROUP " << query[1] << "\n";
								}
							}
						 } break;
			default		:	break;
		}
	}

	//strcpy(buf,"Message to send");
	//send(socketHandle, buf, strlen(buf)+1, 0);
}
          
		  
int buatSocket(){
	struct sockaddr_in remoteSocketInfo;
	struct hostent *hPtr;
	int socketHandle;
	const char *remoteHost="localhost";
	int portNumber = 8080;
	
	bzero(&remoteSocketInfo, sizeof(sockaddr_in));  // Clear structure memory
	
	// dapetin system information
	if((hPtr = gethostbyname(remoteHost)) == NULL)
	{
	  cerr << "System DNS name resolution not configured properly." << endl;
	  cerr << "Error number: " << ECONNREFUSED << endl;
	  exit(EXIT_FAILURE);
	}

	// buat socket handle
	if((socketHandle = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
	  close(socketHandle);
	  exit(EXIT_FAILURE);
	}

	// Load system information into socket data structures
	memcpy((char *)&remoteSocketInfo.sin_addr, hPtr->h_addr, hPtr->h_length);
	remoteSocketInfo.sin_family = AF_INET;
	remoteSocketInfo.sin_port = htons((u_short)portNumber);      // Set port number

	if(connect(socketHandle, (struct sockaddr *)&remoteSocketInfo, sizeof(sockaddr_in)) < 0)	// buka koneksi
	{
		// terjadi error, keluar
		cout << "\nCant connect to server!\n";
		close(socketHandle);
		exit(EXIT_FAILURE);
	}
	
	return socketHandle;
}