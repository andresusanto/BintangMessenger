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
bool notifyNew;
void prosesChat(int socketHandle);

char sendMsgStatus;

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
	sendMsgStatus = 'N';
	
	int rc = 0;
	char buf[512];
	string tmp;
	
	notifyNew = false;
	
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
									notifyNew = true;
									
									replace(query[3].begin(), query[3].end(), '~', ' ');
									
									pesan.dari = query[1];
									pesan.gid = "";
									pesan.tipe = 'P';
									pesan.waktu = stol(query[2]);
									pesan.pesan = query[3];
									
									Helper::simpanpesanUser(activeUser, pesan);
								}else if(query[0].compare("READ") == 0){
									Helper::savereadACK(activeUser, query[1], 'P', stol(query[2]));
								}else if(query[0].compare("MSGGROUP") == 0){
									notifyNew = true;
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
			
							if (notifyNew){
								cout << "\nYou have new message(s)! Use \e[1mlist\e[0m to show them.\n";
							}
							
							cout << "[" << activeUser << "]# ";
							getline (cin,tmp);
							
							if (tmp.length() > 0){
								vector<string> query = Helper::split(tmp, ' ');
								
								if (query[0].compare("message") == 0){
									if (query.size() == 2){
										if (query[1].compare(activeUser) == 0){
											cout << "You can't message yourself!\n";
										}else{
											string msg_send;
											cout << "Enter message: ";
											getline (cin,msg_send);
											
											replace(msg_send.begin(), msg_send.end(), ' ', '~');
											
											string command_send = "MSGTO " + query[1] + " " + msg_send;
											sendMsgStatus = 'W';
											int percobaan = 0;
											
											strcpy(buf, command_send.c_str());
											send(socketHandle, buf, strlen(buf)+1, 0);
									
											while (percobaan <= 30 && sendMsgStatus == 'W'){
												std::chrono::milliseconds dura( 200 );
												std::this_thread::sleep_for( dura );
												percobaan++;
											}
											
											if (sendMsgStatus == 'W'){
												cout << "Failed to send your message!\n";
											}else if(sendMsgStatus == 'E'){
												cout << "Error: User " << query[1] << " doesnt exist!\n";
											}else if(sendMsgStatus == 'S'){
												cout << "Message sent!\n";
												time_t timer; time(&timer);
												long waktu = (long) timer;
												
												replace(msg_send.begin(), msg_send.end(), '~', ' ');
												Pesan pesan;
												pesan.dari = query[1];
												pesan.gid = "";
												pesan.tipe = 'S';
												pesan.waktu = waktu;
												pesan.pesan = msg_send;
												
												Helper::simpanpesanUser(activeUser, pesan);
												Helper::saveviewACK(activeUser, query[1], 'P', waktu);
											}
											
											sendMsgStatus = 'N';
										}
									}else{
										cout << "Invalid message argument. Usage: message (username)\n";
									}
									
									
								}else if (query[0].compare("list") == 0){
									vector<Pesan> listPesan = Helper::loadChat(activeUser);
									struct tm * timeinfo;
									notifyNew = false;
									
									cout << "\n";
									
									for ( int i = 0; i < listPesan.size(); i++ ){
										timeinfo = localtime (&listPesan[i].waktu);
										
										int ukuranDari;
										if (listPesan[i].tipe == 'G'){
											listPesan[i].gid = "[" + listPesan[i].gid + "]";
											cout << "\e[1;32m" << listPesan[i].gid << "\e[0m";
											ukuranDari = listPesan[i].gid.length();
										}else{
											cout << "\e[1;93m" << listPesan[i].dari << "\e[0m";
											ukuranDari = listPesan[i].dari.length();
										}
										
										
										if ( ! listPesan[i].read ){
											cout << "\e[1;5m [NEW]\e[0m";
											ukuranDari += 6;
										}
										
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
										
										bool awalan = true;
										int ukuranPesan = listPesan.size();
										for ( int i = 0; i < ukuranPesan; i++ ){
											timeinfo = localtime (&listPesan[i].waktu);
											
											bool sudahView = Helper::viewACK(activeUser, query[1], 'P', listPesan[i].waktu);
											
											if ( awalan && !sudahView ){
												awalan = false;
												cout << "\n   --------------------- Unread Message Below ----------------------\n";
											}
											
											int ukuranDari;
											
											if (listPesan[i].tipe == 'S'){
												ukuranDari = activeUser.length();
												cout << "\e[1;32m" << activeUser << "\e[0m";
											}else{
												ukuranDari = listPesan[i].dari.length();
												cout << "\e[1;93m" << listPesan[i].dari << "\e[0m";
											}
											
											
											for (int j = ukuranDari ; j <= 46; j ++)
												cout << " ";
												
											if (listPesan[i].pesan.length() > 46)
												listPesan[i].pesan = listPesan[i].pesan.substr(0,42) + " ...";
												
											cout << asctime(timeinfo) << listPesan[i].pesan;
											
											ukuranDari = listPesan[i].pesan.length();
											for (int j = ukuranDari ; j <= 70; j ++)
												cout << " ";
											
											if (listPesan[i].tipe == 'S'){
												if (listPesan[i].read){
													cout << "\n                                                                [READ] \n\n\e[0m";
												}else{
													cout << "\n                                                                       \n\n\e[0m";
												}
											}else{
												cout << "\n                                                                       \n\n\e[0m";
											}
										}
										
										if (ukuranPesan > 0){
											time_t timer; time(&timer);
											long waktu = (long) timer;
											
											
											string msg_send = "CREAD " + query[1];
											strcpy(buf, msg_send.c_str());
											send(socketHandle, buf, strlen(buf)+1, 0);
											
											Helper::saveviewACK(activeUser, query[1], 'P', waktu);
										}
									
									}else{
										cout << "Read only accept one parameter!\n";
									}
								
								}else if (query[0].compare("rg") == 0){
									if (query.size() == 2){
										vector<Pesan> listPesan = Helper::loadpesanUser(activeUser, query[1], 'G');
										struct tm * timeinfo;
										
										bool awalan = true;
										int ukuranPesan = listPesan.size();
										for ( int i = 0; i < ukuranPesan; i++ ){
											timeinfo = localtime (&listPesan[i].waktu);
											
											bool sudahView = Helper::viewACK(activeUser, query[1], 'G', listPesan[i].waktu);
											
											if ( awalan && !sudahView ){
												awalan = false;
												cout << "\n   --------------------- Unread Message Below ----------------------\n";
											}
											
											int ukuranDari;
											
											if (listPesan[i].dari.compare(activeUser) == 0){
												ukuranDari = activeUser.length();
												cout << "\e[1;32m" << activeUser << "\e[0m";
											}else{
												ukuranDari = listPesan[i].dari.length();
												cout << "\e[1;93m" << listPesan[i].dari << "\e[0m";
											}
											
											
											for (int j = ukuranDari ; j <= 46; j ++)
												cout << " ";
												
											if (listPesan[i].pesan.length() > 46)
												listPesan[i].pesan = listPesan[i].pesan.substr(0,42) + " ...";
												
											cout << asctime(timeinfo) << listPesan[i].pesan;
											
											ukuranDari = listPesan[i].pesan.length();
											for (int j = ukuranDari ; j <= 70; j ++)
												cout << " ";
											
											
											cout << "\n                                                                       \n\n\e[0m";
											
										}
										
										if (ukuranPesan > 0){
											time_t timer; time(&timer);
											long waktu = (long) timer;
											
											
											Helper::saveviewACK(activeUser, query[1], 'G', waktu);
										}
									
									}else{
										cout << "Read only accept one parameter!\n";
									}								
								
								}else if (query[0].compare("create") == 0){
									if (query.size() == 2){
										
										string command_send = "CGROUP " + query[1];
										sendMsgStatus = 'C';
										int percobaan = 0;
										
										strcpy(buf, command_send.c_str());
										send(socketHandle, buf, strlen(buf)+1, 0);
								
										while (percobaan <= 30 && sendMsgStatus == 'C'){
											std::chrono::milliseconds dura( 200 );
											std::this_thread::sleep_for( dura );
											percobaan++;
										}
										
										if (sendMsgStatus == 'C'){
											cout << "Request to create group failed!\n";
										}else if(sendMsgStatus == 'E'){
											cout << "Error: Group " << query[1] << " already exist!\n";
										}else if(sendMsgStatus == 'S'){
											cout << "Group created! Now you are a member of " + query[1] + "!\n";
										}
										
										sendMsgStatus = 'N';
									}else{
										cout << "Group name can't contains space!\n";
									}
								}else if (query[0].compare("join") == 0){
									if (query.size() == 2){
										
										string command_send = "JGROUP " + query[1];
										sendMsgStatus = 'J';
										int percobaan = 0;
										
										strcpy(buf, command_send.c_str());
										send(socketHandle, buf, strlen(buf)+1, 0);
								
										while (percobaan <= 30 && sendMsgStatus == 'J'){
											std::chrono::milliseconds dura( 200 );
											std::this_thread::sleep_for( dura );
											percobaan++;
										}
										
										if (sendMsgStatus == 'J'){
											cout << "Request to join group failed!\n";
										}else if(sendMsgStatus == '1'){
											cout << "Error: Group " << query[1] << " doesn't exist!\n";
										}else if(sendMsgStatus == '2'){
											cout << "Error: You already a member of " << query[1] << "!\n";
										}else if(sendMsgStatus == 'S'){
											cout << "Join success! Now you are a member of " + query[1] + "!\n";
										}
										
										sendMsgStatus = 'N';
									}else{
										cout << "Group name can't contains space!\n";
									}
								}else if (query[0].compare("leave") == 0){
									if (query.size() == 2){
										
										string command_send = "LGROUP " + query[1];
										sendMsgStatus = 'L';
										int percobaan = 0;
										
										strcpy(buf, command_send.c_str());
										send(socketHandle, buf, strlen(buf)+1, 0);
								
										while (percobaan <= 30 && sendMsgStatus == 'L'){
											std::chrono::milliseconds dura( 200 );
											std::this_thread::sleep_for( dura );
											percobaan++;
										}
										
										if (sendMsgStatus == 'L'){
											cout << "Request to leave group failed!\n";
										}else if(sendMsgStatus == 'E'){
											cout << "Error: You are not a member of " << query[1] << "!\n";
										}else if(sendMsgStatus == 'S'){
											cout << "Leave success! Now you are not a member of " + query[1] + " anymore!\n";
										}
										
										sendMsgStatus = 'N';
									}else{
										cout << "Group name can't contains space!\n";
									}
								
								}else if(query[0].compare("mg") == 0){
									if (query.size() == 2){
									
										string msg_send;
										cout << "Enter message: ";
										getline (cin,msg_send);
										
										replace(msg_send.begin(), msg_send.end(), ' ', '~');
										
										string command_send = "MSGGROUPTO " + query[1] + " " + msg_send;
										sendMsgStatus = 'W';
										int percobaan = 0;
										
										strcpy(buf, command_send.c_str());
										send(socketHandle, buf, strlen(buf)+1, 0);
								
										while (percobaan <= 30 && sendMsgStatus == 'W'){
											std::chrono::milliseconds dura( 200 );
											std::this_thread::sleep_for( dura );
											percobaan++;
										}
										
										if (sendMsgStatus == 'W'){
											cout << "Failed to send your message!\n";
										}else if(sendMsgStatus == 'E'){
											cout << "Error: You are not a member of " << query[1] << "!\n";
										}else if(sendMsgStatus == 'S'){
											cout << "Message sent!\n";
											time_t timer; time(&timer);
											long waktu = (long) timer;
											
											replace(msg_send.begin(), msg_send.end(), '~', ' ');
											Pesan pesan;
											pesan.dari = activeUser;
											pesan.gid = "";
											pesan.tipe = 'G';
											pesan.waktu = waktu;
											pesan.pesan = msg_send;
											
											Helper::simpanpesanUser(activeUser, pesan);
											Helper::saveviewACK(activeUser, query[1], 'G', waktu);
										}
										
										sendMsgStatus = 'N';
										
									}else{
										cout << "Invalid message argument. Usage: message (username)\n";
									}
									
								
								}else{
									cout << "Invalid command!\n";
								}
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
			notifyNew = true;
			
			Pesan pesan;
			
			replace(query[3].begin(), query[3].end(), '~', ' ');
								
			pesan.dari = query[1];
			pesan.gid = "";
			pesan.tipe = 'P';
			pesan.waktu = stol(query[2]);
			pesan.pesan = query[3];
			
			Helper::simpanpesanUser(activeUser, pesan);
		}else if(query[0].compare("MSGGROUP") == 0){
			notifyNew = true;
			
			Pesan pesan;
			
			replace(query[4].begin(), query[4].end(), '~', ' ');
								
			pesan.dari = query[2];
			pesan.gid = query[1];
			pesan.tipe = 'G';
			pesan.waktu = stol(query[3]);
			pesan.pesan = query[4];
			
			Helper::simpanpesanUser(activeUser, pesan);
		}else if(query[0].compare("READ") == 0){
			Helper::savereadACK(activeUser, query[1], 'P', stol(query[2]));
		}else if(query[0].compare("MSGOK") == 0){
			sendMsgStatus = 'S';
		}else if(query[0].compare("MSGNO") == 0){
			sendMsgStatus = 'E';
		}else if(query[0].compare("CGOK") == 0 && sendMsgStatus == 'C'){
			sendMsgStatus = 'S';
		}else if(query[0].compare("CGNO") == 0 && sendMsgStatus == 'C'){
			sendMsgStatus = 'E';
		}else if(query[0].compare("JGOK") == 0 && sendMsgStatus == 'J'){
			sendMsgStatus = 'S';
		}else if(query[0].compare("JGNO1") == 0 && sendMsgStatus == 'J'){
			sendMsgStatus = '1';
		}else if(query[0].compare("JGNO2") == 0 && sendMsgStatus == 'J'){
			sendMsgStatus = '2';
		}else if(query[0].compare("LGOK") == 0 && sendMsgStatus == 'L'){
			sendMsgStatus = 'S';
		}else if(query[0].compare("LGNO") == 0 && sendMsgStatus == 'L'){
			sendMsgStatus = 'E';
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