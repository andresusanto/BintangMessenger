#pragma once 
#include "Pesan.h"
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

class Helper{
	public:
		static bool login(string username,string password);
		static bool signup(string username,string password);
	
		static vector<Pesan> pesanUser(string username);
		static void storePesan(string username,Pesan &pesan);
		
		static vector<string> &split(const string &s, char delim, vector<string> &elems);
		static vector<string> split(const string &s, char delim);
		
		static vector<Pesan> loadChat(string username);
		static vector<Pesan> loadpesanUser(string username, string filter, char tipe);
		static void simpanpesanUser(string username, Pesan &pesan);
		
		static bool readACK(string username, string dari, char tipe, long time);
		static void savereadACK(string username, string dari, char tipe, long time);
		
		static bool viewACK(string username, string dari, char tipe, long time);
		static void saveviewACK(string username, string dari, char tipe, long time);
		
};