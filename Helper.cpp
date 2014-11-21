// ceritanya satu untuk semua, satu helper untuk client dan server (code reusable :D )

#include "Helper.h"
#include <cstdio>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

vector<string>& Helper::split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


vector<string> Helper::split(const string &s, char delim) {
    vector<string> elems;
    Helper::split(s, delim, elems);
    return elems;
}

bool Helper::login(string username,string password){
	string i_user, i_password;
	ifstream infile("user.txt");
	while (infile >> i_user >> i_password)
	{
		if (i_user.compare(username) == 0){
			if (i_password.compare(password) == 0){
				return true;
			}else{
				return false;
			}
		}
	}
	
	return false;
}

bool Helper::signup(string username,string password){
	string i_user, i_password;
	ifstream infile("user.txt");
	while (infile >> i_user >> i_password)
	{
		if (i_user.compare(username) == 0){
			return false;
		}
	}
	ofstream outfile("user.txt", ios_base::app | ios_base::out);
	outfile << username << " " << password << "\n";
	return true;
}
	
vector<Pesan> Helper::pesanUser(string username){
	int i;
	string line;
	vector<Pesan> res;
	Pesan tmp;
	
	string namafile = "pending_" + username + ".txt";
	
	ifstream infile(namafile.c_str());
	
	while (getline(infile, line))
	{
		vector<string> dataLine = split(line, ';');
		tmp.dari = dataLine[0];
		tmp.tujuan = "";
		tmp.waktu = stol(dataLine[4]);
		tmp.pesan = dataLine[3];
		tmp.gid = dataLine[1];
		tmp.tipe = dataLine[2][0];
		tmp.read = false;
		
		res.push_back(tmp);
	}
	
	remove( namafile.c_str() );
	
	return res;
}

void Helper::storePesan(string username, Pesan pesan){
	
}