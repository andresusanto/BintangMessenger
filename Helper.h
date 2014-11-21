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
		static void storePesan(string username,Pesan pesan);
		
		static vector<string> &split(const string &s, char delim, vector<string> &elems);
		static vector<string> split(const string &s, char delim);
};