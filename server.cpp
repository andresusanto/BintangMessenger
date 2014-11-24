#include <unistd.h>
#include <thread>  
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h> 
#include <netdb.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <cstring>
#include <algorithm>
#include <fcntl.h>
#include "Pesan.h"
#include "DataKoneksi.h"
#include "Helper.h"

#define MAXHOSTNAME 256
#define SERVERCAPACITY 100

using namespace std;

DataKoneksi *dakon;


int getUserChannel(string username);

void proses(int sock, int channel){

	int rc = 1;  // Actual number of bytes read
	char buf[512];
	
	dakon[channel].state = 1; //state channel dipakai, belum login.
	
	rc = recv(sock, buf, 512, 0);
	while (rc > 0) {
		
		switch(dakon[channel].state){
		
			case 1:		{
							buf[rc]= (char) NULL;
							vector<string> query = Helper::split((string)buf, ' ');
							if (query[0].compare("LOGIN") == 0) {
								if (Helper::login(query[1], query[2])){
									strcpy(buf, ((string)"LOGINOK").c_str());
									send(sock, buf, strlen(buf)+1, 0);
									
									Helper::serverLog(query[1] + " logged in.");
									
									vector<Pesan> offData = Helper::pesanUser(query[1]);
									int banyakOff = offData.size();
									
									Helper::serverLog("Start sending pending message(s) to " + query[1] + ".");
									
									for (int i = 0; i < banyakOff; i++){
									
										std::chrono::milliseconds dura( 20 );	// untuk speed sangat tinggi, untuk mengimbangi kecepatan kernel VM
										std::this_thread::sleep_for( dura );
										
										replace(offData[i].pesan.begin(), offData[i].pesan.end(), ' ', '~');
										if (offData[i].tipe == 'G'){
											strcpy(buf, ((string)"MSGGROUP " + offData[i].gid + " " + offData[i].dari + " " + to_string(offData[i].waktu) + " " + offData[i].pesan).c_str());
										}else{
											strcpy(buf, ((string)"MSG " + offData[i].dari + " " + to_string(offData[i].waktu) + " " + offData[i].pesan).c_str());
										}
										send(sock, buf, strlen(buf)+1, 0);
									
									}
									std::chrono::milliseconds dura( 100 );
									std::this_thread::sleep_for( dura );
									
									
									vector<string> rawData = Helper::loadRaw(query[1]);
									int banyakRaw = rawData.size();
									
									for (int i = 0; i < banyakRaw; i++){
									
										std::chrono::milliseconds dura( 20 );	// untuk speed sangat tinggi, untuk mengimbangi kecepatan kernel VM
										std::this_thread::sleep_for( dura );
										
										strcpy(buf, rawData[i].c_str());
										send(sock, buf, strlen(buf)+1, 0);
									
									}
									
									std::this_thread::sleep_for( dura );
									
									strcpy(buf, ((string)"OK").c_str());
									send(sock, buf, strlen(buf)+1, 0);
									
									dakon[channel].username = query[1];
									dakon[channel].state = 2;	// state udah login
									dakon[channel].sock = sock;
									
									Helper::serverLog("Pending message(s) to " + query[1] + " have been sent. User is now online.");
									
								}else{
									strcpy(buf, ((string)"LOGINNO").c_str());
									send(sock, buf, strlen(buf)+1, 0);
								}
								
							}else if(query[0].compare("SIGNUP") == 0){
								if (Helper::signup(query[1], query[2])) {
									Helper::serverLog("User " + query[1] + " have been registered succesfuly.");
									strcpy(buf, ((string)"SIGNUPOK").c_str());
								}else{
									strcpy(buf, ((string)"SIGNUPNO").c_str());
								}
								send(sock, buf, strlen(buf)+1, 0);
							}
							rc = recv(sock, buf, 512, 0);
							break;
						}
						
			case 2	: {
							
						buf[rc]= (char) NULL;
						vector<string> query = Helper::split((string)buf, ' ');
						
						if (query[0].compare("MSGTO") == 0) {
							int uc = getUserChannel(query[1]);
							time_t timer; time(&timer);
							long waktu = (long) timer;
								
							if (uc != -1){
								string to_send = "MSG " + dakon[channel].username + " " + to_string(waktu) + " " + query[2];
								strcpy(buf, to_send.c_str());
								send(dakon[uc].sock, buf, strlen(buf)+1, 0);
								
								Helper::serverLog(dakon[channel].username + " messaged " + query[1] + ". [Direct]");
								
								to_send = "MSGOK";
								strcpy(buf, to_send.c_str());
								send(dakon[channel].sock, buf, strlen(buf)+1, 0);
							}else{
								if (Helper::userExist(query[1])){
									replace(query[2].begin(), query[2].end(), '~', ' ');
									
									Pesan pesan;
									
									pesan.dari = dakon[channel].username;
									pesan.gid = "";
									pesan.tipe = 'S';
									pesan.pesan = query[2];
									pesan.waktu = waktu;
									
									Helper::storePesan(query[1], pesan);
									
									Helper::serverLog(dakon[channel].username + " messaged " + query[1] + ". [Pending]");
									
									string to_send = "MSGOK";
									strcpy(buf, to_send.c_str());
									send(dakon[channel].sock, buf, strlen(buf)+1, 0);
								}else{
									string to_send = "MSGNO";
									strcpy(buf, to_send.c_str());
									send(dakon[channel].sock, buf, strlen(buf)+1, 0);
								}
							}
						}else if (query[0].compare("MSGGROUPTO") == 0){
							string to_send;
							if (Helper::isMemberGroup(query[1], dakon[channel].username)){
								vector<string> send_list = Helper::memberGroup(query[1]);
								int jumList = send_list.size();
								time_t timer; time(&timer);
								long waktu = (long) timer;
							
								for (int i = 0; i < jumList; i++){
									if (send_list[i].compare(dakon[channel].username) != 0){
										int uc = getUserChannel(send_list[i]);
										if (uc != -1){
											to_send = "MSGGROUP " + query[1] + " " + dakon[channel].username + " " + to_string(waktu) + " " + query[2];
											strcpy(buf, to_send.c_str());
											send(dakon[uc].sock, buf, strlen(buf)+1, 0);
										}else{
											replace(query[2].begin(), query[2].end(), '~', ' ');
											
											Pesan pesan;
											
											pesan.dari = dakon[channel].username;
											pesan.gid = query[1];
											pesan.tipe = 'G';
											pesan.pesan = query[2];
											pesan.waktu = waktu;
											
											Helper::storePesan(send_list[i], pesan);
											
											replace(query[2].begin(), query[2].end(), ' ', '~');
										}
									}
								}
								
								Helper::serverLog(dakon[channel].username + " messaged " + query[1] + ". [GROUP]");
								
								to_send = "MSGOK";
								strcpy(buf, to_send.c_str());
								send(dakon[channel].sock, buf, strlen(buf)+1, 0);
							}else{
								to_send = "MSGNO";
								strcpy(buf, to_send.c_str());
								send(dakon[channel].sock, buf, strlen(buf)+1, 0);
							}
							
							
							
						}else if (query[0].compare("CREAD") == 0){
							int uc = getUserChannel(query[1]);
							time_t timer; time(&timer);
							long waktu = (long) timer;
							string to_send = "READ " + dakon[channel].username + " " + to_string(waktu);
								
							if (uc != -1){
								strcpy(buf, to_send.c_str());
								send(dakon[uc].sock, buf, strlen(buf)+1, 0);
							}else{
								Helper::saveRaw(query[1], to_send);
							}
							
						}else if (query[0].compare("CGROUP") == 0){
							string to_send;
							if (Helper::createGroup(query[1])){
								Helper::joinGroup(query[1], dakon[channel].username);
								Helper::serverLog(dakon[channel].username + " has created " + query[1] + ".");
								to_send = "CGOK";
							}else{
								to_send = "CGNO";
							}
							strcpy(buf, to_send.c_str());
							send(dakon[channel].sock, buf, strlen(buf)+1, 0);
							
						}else if (query[0].compare("JGROUP") == 0){
							string to_send;
							if (Helper::isGroup(query[1])){
								
								if (Helper::joinGroup(query[1], dakon[channel].username)){
									Helper::serverLog(dakon[channel].username + " has joined " + query[1] + ".");
									to_send = "JGOK";
								}else{
									to_send = "JGNO2";
								}
							}else{
								to_send = "JGNO1";
							}
							strcpy(buf, to_send.c_str());
							send(dakon[channel].sock, buf, strlen(buf)+1, 0);
							
						}else if (query[0].compare("LGROUP") == 0){
							string to_send;
							if (Helper::leaveGroup(query[1], dakon[channel].username)){
								Helper::serverLog(dakon[channel].username + " has left " + query[1] + ".");
								to_send = "LGOK";
							}else{
								to_send = "LGNO";
							}
							strcpy(buf, to_send.c_str());
							send(dakon[channel].sock, buf, strlen(buf)+1, 0);
						}else if (query[0].compare("FILETO") == 0){
							string to_send;
							if (Helper::userExist(query[1])){
								int uc = getUserChannel(query[1]);
								if (uc != -1){
									to_send = "OK";
									strcpy(buf, to_send.c_str());
									send(dakon[channel].sock, buf, strlen(buf)+1, 0);
									
									
									int ukuran = atoi(query[3].c_str());
									char tmpFile[ukuran];
									rc = recv(sock, tmpFile, ukuran, 0);
									
									
									to_send = "FILEFROM " + dakon[channel].username + " " + query[2] + " " + query[3];
									strcpy(buf, to_send.c_str());
									send(dakon[uc].sock, buf, strlen(buf)+1, 0);
									
									//rc = recv(dakon[uc].sock, buf, 512, 0);
									
									std::chrono::milliseconds dura( 200 );	
									std::this_thread::sleep_for( dura );
								
									send(dakon[uc].sock, tmpFile, ukuran, 0);
									
									Helper::serverLog(dakon[channel].username + " has sent a " + query[3] + " bytes file to " + query[1] + ".");
									
									to_send = "OK";
									strcpy(buf, to_send.c_str());
									send(dakon[channel].sock, buf, strlen(buf)+1, 0);
									
									
								}else{
									to_send = "NO 2";
									strcpy(buf, to_send.c_str());
									send(dakon[channel].sock, buf, strlen(buf)+1, 0);
								}
							}else{
								to_send = "NO 1";
								strcpy(buf, to_send.c_str());
								send(dakon[channel].sock, buf, strlen(buf)+1, 0);
							}
						}
						
						rc = recv(sock, buf, 512, 0);
			
			}break;
			default:	break;
		
		}
		
	}
	
	if (dakon[channel].state != 0){
		Helper::serverLog(dakon[channel].username + " logged out. User is now offline.");
		dakon[channel].state = 0; //Biar channel bisa dipake yang lain
		dakon[channel].username = "";
	}
	
}


int availChannel(){
	for (int i = 0; i < SERVERCAPACITY; i++){
		if (dakon[i].state == 0){
			return i;
		}
	}
	
	return -1;
}

int getUserChannel(string username){
	for (int i = 0; i < SERVERCAPACITY; i++){
		if (dakon[i].username.compare(username) == 0){
			return i;
		}
	}
	return -1;
}

main()
{
   struct sockaddr_in socketInfo;
   char sysHost[MAXHOSTNAME+1];  // Untuk Hostname komputer yang sedang aktif
   struct hostent *hPtr;
   int socketHandle;
   int portNumber = 8080;
   
   system("clear");
   
   cout << "BINTANG MESSENGER SERVER 0.1b\n";
   cout << "------------------------------\n";
   
   dakon = new DataKoneksi[SERVERCAPACITY];
   for (int i = 0; i < SERVERCAPACITY; i++){
		dakon[i].username = "";
		dakon[i].state = 0;
   }

   bzero(&socketInfo, sizeof(sockaddr_in));  // Clear structure memory

   cout << "Setting up server's hostname ... ";

   gethostname(sysHost, MAXHOSTNAME); 
   if((hPtr = gethostbyname(sysHost)) == NULL)
   {
      cerr << "System hostname misconfigured." << endl;
      exit(EXIT_FAILURE);
   }
   
   cout << "OK\n";
   cout << "Creating master socket ... ";
   // Buat socket utk listen

   if((socketHandle = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      close(socketHandle);
      exit(EXIT_FAILURE);
   }
	
	cout << "OK\n";
   cout << "Configuring master socket ... ";
   
   // Load system info untuk socket

   socketInfo.sin_family = AF_INET;
   socketInfo.sin_addr.s_addr = htonl(INADDR_ANY);
   socketInfo.sin_port = htons(portNumber);   
   
	cout << "OK\n";
   cout << "Binding master socket ... ";
   
   // Bind socket yang sudah d buat

   if( bind(socketHandle, (struct sockaddr *) &socketInfo, sizeof(socketInfo)) < 0)
   {
      close(socketHandle);
      perror("bind");
      exit(EXIT_FAILURE);
   }
   
   
   listen(socketHandle, 1);
   
   cout << "OK\n\n";
   Helper::serverLog("Bintang messenger server started!");

   int socketConnection;
   while( 1 )
   {
    int rcod = (socketConnection = accept(socketHandle, NULL, NULL));
	if (rcod >= 0){
		int channel = availChannel();
		
		if (channel != -1){
			// ketika channel masih ada (tidak lebih dari SERVERCAPACITY)
			thread baru (proses, socketConnection, channel);
			baru.detach();
		}else{
			// channel habis, tolak koneksi
			close(socketConnection);
		}
	}
   }
   
   // I N F I N I T E    L O O P

}
          