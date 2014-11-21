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
	string line;
	vector<Pesan> res;
	Pesan tmp;
	
	string namafile = "pending_" + username + ".txt";
	
	ifstream infile(namafile.c_str());
	
	while (getline(infile, line))
	{
		vector<string> dataLine = split(line, ';');
		tmp.dari = dataLine[0];
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

void Helper::storePesan(string username, Pesan &pesan){
	string namafile = "pending_" + username + ".txt";
	ofstream outfile(namafile, ios_base::app | ios_base::out);
	outfile << pesan.dari << ";" << pesan.gid << ";" << pesan.tipe <<  ";"  << pesan.pesan << ";"  << pesan.waktu <<  "\n";
}

vector<Pesan> Helper::loadChat(string username){
	string line;
	vector<Pesan> res;
	Pesan tmp;
	
	string namafile = "pesan_" + username + ".txt";
	ifstream infile(namafile.c_str());
	
	while (getline(infile, line))
	{
		vector<string> dataLine = split(line, ';');
		tmp.dari = dataLine[0];
		tmp.waktu = stol(dataLine[4]);
		tmp.pesan = dataLine[3];
		tmp.gid = dataLine[1];
		tmp.tipe = dataLine[2][0];
		tmp.read = viewACK( username, tmp.dari, tmp.tipe, tmp.waktu );
		
		
		int ukuran = res.size();
		for(int i = 0; i < ukuran; i++){
			if (res[i].tipe == 'G' && tmp.tipe == 'G'){
				if (res[i].gid.compare(tmp.gid) == 0){
					res.erase(res.begin() + i);
					res.insert(res.begin(), tmp);
					i = 99;
				}
			}else{
				if (res[i].dari.compare(tmp.dari) == 0 && tmp.tipe != 'G' && res[i].tipe != 'G'){
					res.erase(res.begin() + i);
					res.insert(res.begin(), tmp);
					i = 99;
				}
			}
			
			if (i == ukuran - 1){
				if (ukuran == 10){
					res.erase(res.begin() + 9);
				}
				res.insert(res.begin(), tmp);
			}
		}
		
		if (ukuran == 0){
			res.insert(res.begin(), tmp);
		}
	}
	
	return res;
}

vector<Pesan> Helper::loadpesanUser(string username, string filter, char tipe){
	string line;
	vector<Pesan> res;
	Pesan tmp;
	
	string namafile = "pesan_" + username + ".txt";
	
	ifstream infile(namafile.c_str());
	
	while (getline(infile, line))
	{
		vector<string> dataLine = split(line, ';');
		tmp.dari = dataLine[0];
		tmp.waktu = stol(dataLine[4]);
		tmp.pesan = dataLine[3];
		tmp.gid = dataLine[1];
		tmp.tipe = dataLine[2][0];
		tmp.read = readACK ( username, tmp.dari, tmp.tipe, tmp.waktu );
		
		if (tipe == 'G' && tmp.tipe == 'G' && tmp.gid.compare(filter) == 0){
			res.push_back(tmp);
		}else if(tmp.tipe != 'G' && tipe != 'G' && tmp.dari.compare(filter) == 0){
			res.push_back(tmp);
		}
		
	}
	
	remove( namafile.c_str() );
	
	return res;
}

void Helper::simpanpesanUser(string username, Pesan &pesan){
	string namafile = "pesan_" + username + ".txt";
	ofstream outfile(namafile, ios_base::app | ios_base::out);
	outfile << pesan.dari << ";" << pesan.gid << ";" << pesan.tipe <<  ";"  << pesan.pesan << ";"  << pesan.waktu <<  "\n";
}

bool Helper::readACK(string username, string dari, char tipe, long time){
	string i_dari;
	char i_tipe;
	long i_time;
	
	string namafile = "read_" + username + ".txt";
	ifstream infile(namafile);
	while (infile >> i_dari >> i_tipe >> i_time)
	{
		if (i_dari.compare(dari) == 0 && i_tipe == tipe){
			if (time < i_time)
				return true;
			else
				return false;
		}
	}
	
	return false;
}

void Helper::savereadACK(string username, string dari, char tipe, long time){
	string i_dari;
	char i_tipe;
	long i_time;
	
	stringstream ss;
	
	string namafile = "read_" + username + ".txt";
	ifstream infile(namafile);
	while (infile >> i_dari >> i_tipe >> i_time)
	{
		if (!(i_dari.compare(dari) == 0 && i_tipe == tipe)){
			ss << i_dari << " " << i_tipe << " " << i_time << "\n";
		}
	}
	
	ss << dari << " " << tipe << " " << time << "\n";
	ofstream outfile(namafile, ios_base::out);
	outfile << ss.str();
}

bool Helper::viewACK(string username, string dari, char tipe, long time){
	string i_dari;
	char i_tipe;
	long i_time;
	
	string namafile = "view_" + username + ".txt";
	ifstream infile(namafile);
	while (infile >> i_dari >> i_tipe >> i_time)
	{
		if (i_dari.compare(dari) == 0 && i_tipe == tipe){
			if (time < i_time)
				return true;
			else
				return false;
		}
	}
	
	return false;
}

void Helper::saveviewACK(string username, string dari, char tipe, long time){
	string i_dari;
	char i_tipe;
	long i_time;
	
	stringstream ss;
	
	string namafile = "view_" + username + ".txt";
	ifstream infile(namafile);
	while (infile >> i_dari >> i_tipe >> i_time)
	{
		if (!(i_dari.compare(dari) == 0 && i_tipe == tipe)){
			ss << i_dari << " " << i_tipe << " " << i_time << "\n";
		}
	}
	
	ss << dari << " " << tipe << " " << time << "\n";
	ofstream outfile(namafile, ios_base::out);
	outfile << ss.str();
}