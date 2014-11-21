#include <unistd.h>
#include <thread>  
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <cstring>
#include <algorithm>

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
								cout << buf << "\n";
								if (Helper::login(query[1], query[2])){
									strcpy(buf, ((string)"LOGINOK").c_str());
									send(sock, buf, strlen(buf)+1, 0);
									
									
									vector<Pesan> offData = Helper::pesanUser(query[1]);
									int banyakOff = offData.size();
									
									cout << dakon[channel].username << " Start sending pending message" << "\n";
									
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
									
									strcpy(buf, ((string)"OK").c_str());
									send(sock, buf, strlen(buf)+1, 0);
									
									dakon[channel].username = query[1];
									dakon[channel].state = 2;	// state udah login
									dakon[channel].sock = sock;
									
									cout << dakon[channel].username << " OK sent" << "\n";
									
								}else{
									strcpy(buf, ((string)"LOGINNO").c_str());
									send(sock, buf, strlen(buf)+1, 0);
								}
								
							}else if(query[0].compare("SIGNUP") == 0){
								cout << buf << "\n";
								if (Helper::signup(query[1], query[2])) {
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
								
							}else{
								replace(query[2].begin(), query[2].end(), '~', ' ');
								
								Pesan pesan;
								
								pesan.dari = dakon[channel].username;
								pesan.gid = "";
								pesan.tipe = 'S';
								pesan.pesan = query[2];
								pesan.waktu = waktu;
								
								Helper::storePesan(query[1], pesan);
							}
						}else if (query[0].compare("MSGGROUPTO") == 0){
						
						}
						
						rc = recv(sock, buf, 512, 0);
			
			}break;
			default:	break;
		
		}
		
	}
	
	cout << "[] Client '" << dakon[channel].username << "' Disconnected" << endl;
    dakon[channel].state = 0; //Biar channel bisa dipake yang lain
	dakon[channel].username = "";
	
	
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
   char sysHost[MAXHOSTNAME+1];  // Hostname of this computer we are running on
   struct hostent *hPtr;
   int socketHandle;
   int portNumber = 8080;
   
   
   dakon = new DataKoneksi[SERVERCAPACITY];
   for (int i = 0; i < SERVERCAPACITY; i++){
		dakon[i].username = "";
		dakon[i].state = 0;
   }

   bzero(&socketInfo, sizeof(sockaddr_in));  // Clear structure memory

   // Get system information

   gethostname(sysHost, MAXHOSTNAME);  // Get the name of this computer we are running on
   if((hPtr = gethostbyname(sysHost)) == NULL)
   {
      cerr << "System hostname misconfigured." << endl;
      exit(EXIT_FAILURE);
   }

   // create socket

   if((socketHandle = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      close(socketHandle);
      exit(EXIT_FAILURE);
   }

   // Load system information into socket data structures

   socketInfo.sin_family = AF_INET;
   socketInfo.sin_addr.s_addr = htonl(INADDR_ANY); // Use any address available to the system
   socketInfo.sin_port = htons(portNumber);      // Set port number

   // Bind the socket to a local socket address

   if( bind(socketHandle, (struct sockaddr *) &socketInfo, sizeof(socketInfo)) < 0)
   {
      close(socketHandle);
      perror("bind");
      exit(EXIT_FAILURE);
   }

   
   listen(socketHandle, 1);

   int socketConnection;
   while( 1 )
   {
    int rcod = (socketConnection = accept(socketHandle, NULL, NULL));
	if (rcod >= 0){
		int channel = availChannel();
		
		if (channel != -1){
			thread baru (proses, socketConnection, channel);
			baru.detach();
		}else{
			close(socketConnection);
		}
	}
      //exit(EXIT_FAILURE);
   }
   //close(socketHandle);

}
          