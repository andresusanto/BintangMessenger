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
#include <vector>
#include <time.h> 

#define MAXHOSTNAME 256

#include "Helper.h"

using namespace std;
char clientState;
string activeUser;
int buatSocket();
void prosesChat(int socketHandle);

vector<string> sckVector;

void printLogo(){
	system("clear");
	
	cout << "           ______  _____ __   _ _______ _______ __   _  ______\n";
	cout << "           |_____]   |   | \\  |    |    |_____| | \\  | |  ____\n";
	cout << "           |_____] __|__ |  \\_|    |    |     | |  \\_| |_____|\n";
	cout << " _______ _______ _______ _______ _______ __   _  ______ _______  ______\n";
	cout << " |  |  | |______ |______ |______ |______ | \\  | |  ____ |______ |_____/\n";
	cout << " |  |  | |______ ______| ______| |______ |  \\_| |_____| |______ |    \\_\n\n";
}

main()
{
	clientState = 'A';	// state inisiasi -> cuma bisa login
	
	int rc = 0;
	char buf[512];
	string tmp;
	
	printLogo();
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
										activeUser = l_user;
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
								Pesan pesan;
								rc = recv(socketHandle, buf, 512, 0);
								vector<string> query = Helper::split((string)buf, ' ');
								
								if (query[0].compare("OK") == 0){
									clientState = 'M';
									thread baru (prosesChat, socketHandle);
									baru.detach();
									offlineMsgHabis = true;
									printLogo();
									cout << "Welcome \e[1m" << activeUser << "\e[0m! Use \e[1mhelp\e[0m to show command list.\n";
								}else if(query[0].compare("MSG") == 0){
									replace(query[3].begin(), query[3].end(), '~', ' ');
									
									pesan.dari = query[1];
									pesan.gid = "";
									pesan.tipe = 'P';
									pesan.waktu = stol(query[2]);
									pesan.pesan = query[3];
									
									Helper::simpanpesanUser(activeUser, pesan);
								}else if(query[0].compare("MSGGROUP") == 0){
									replace(query[4].begin(), query[4].end(), '~', ' ');
									
									pesan.dari = query[2];
									pesan.gid = query[1];
									pesan.tipe = 'G';
									pesan.waktu = stol(query[3]);
									pesan.pesan = query[4];
									
									Helper::simpanpesanUser(activeUser, pesan);
								}
							}
						 } break;
						 
			case 'M'	:{
							
							cout << "[" << activeUser << "]# ";
							getline (cin,tmp);
							
							vector<string> query = Helper::split(tmp, ' ');
							
							if (query[0].compare("message") == 0){
								string msg_send = "MSGTO andre cucu~cucu";
								strcpy(buf, msg_send.c_str());
								send(socketHandle, buf, strlen(buf)+1, 0);
								
							}else if (query[0].compare("list") == 0){
								vector<Pesan> listPesan = Helper::loadChat(activeUser);
								struct tm * timeinfo;
								
								
								for ( int i = 0; i < listPesan.size(); i++ ){
									timeinfo = localtime (&listPesan[i].waktu);
									
									if ( listPesan[i].read ){
										cout << "\e[100m";
									}else{
										cout << "\e[104m";
									}
									
									cout << "\e[1;93m" << listPesan[i].dari << "\e[0m";
									
									if ( listPesan[i].read ){
										cout << "\e[100m";
									}else{
										cout << "\e[104m";
									}
									
									int ukuranDari = listPesan[i].dari.length();
									for (int j = ukuranDari ; j <= 46; j ++)
										cout << " ";
										
									if (listPesan[i].pesan.length() > 46)
										listPesan[i].pesan = listPesan[i].pesan.substr(0,42) + " ...";
										
									cout << asctime(timeinfo) << listPesan[i].pesan;
									
									ukuranDari = listPesan[i].pesan.length();
									for (int j = ukuranDari ; j <= 70; j ++)
										cout << " ";
									
									cout << "\n                                                                       \n\e[0m";
								}
							}else if (query[0].compare("read") == 0){
								if (query.size() == 2){
									vector<Pesan> listPesan = Helper::loadpesanUser(activeUser, query[1], 'P');
									struct tm * timeinfo;
								
									for ( int i = 0; i < listPesan.size(); i++ ){
										timeinfo = localtime (&listPesan[i].waktu);
										cout << listPesan[i].dari << "\t" << listPesan[i].pesan << "\t" << asctime(timeinfo) << "\n";
									}
								}else{
									cout << "Read only accept one parameter!\n";
								}
							}else{
								cout << "Invalid command!\n";
							}
							
						} break;
			default		:	break;
		}
	}

	//strcpy(buf,"Message to send");
	//send(socketHandle, buf, strlen(buf)+1, 0);
}

void prosesChat(int socketHandle){
	int rc;
	char buf[512];
	
	rc = recv(socketHandle, buf, 512, 0);
	while(rc > 0){
		vector<string> query = Helper::split((string)buf, ' ');
								
		if (query[0].compare("MSG") == 0){
			Pesan pesan;
			
			replace(query[3].begin(), query[3].end(), '~', ' ');
								
			pesan.dari = query[1];
			pesan.gid = "";
			pesan.tipe = 'P';
			pesan.waktu = stol(query[2]);
			pesan.pesan = query[3];
			
			Helper::simpanpesanUser(activeUser, pesan);
		}else if(query[0].compare("MSGGROUP") == 0){
			Pesan pesan;
			
			replace(query[4].begin(), query[4].end(), '~', ' ');
								
			pesan.dari = query[2];
			pesan.gid = query[1];
			pesan.tipe = 'G';
			pesan.waktu = stol(query[3]);
			pesan.pesan = query[4];
			
			Helper::simpanpesanUser(activeUser, pesan);
		}else if(query[0].compare("READ") == 0){
			time_t timer; time(&timer);
			long waktu = (long) timer;
			Helper::savereadACK(activeUser, query[1], query[2][0], waktu);
		}
		rc = recv(socketHandle, buf, 512, 0);
	}
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