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

bool Helper::userExist(string username){
	string i_user, i_password;
	ifstream infile("user.txt");
	while (infile >> i_user >> i_password)
	{
		if (i_user.compare(username) == 0){
			return true;
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
		
		if (tmp.tipe == 'G'){
			tmp.read = viewACK( username, tmp.gid, tmp.tipe, tmp.waktu );
		}else{
			tmp.read = viewACK( username, tmp.dari, tmp.tipe, tmp.waktu );
		}
		
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
		if (i_dari.compare(dari) == 0){
			if (time <= i_time)
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

vector<string> Helper::loadRaw(string username){
	string namafile = "raw_" + username + ".txt";
	ifstream infile(namafile);
	string line;
	
	vector<string> tmp;
	
	while (std::getline(infile, line))
	{
		tmp.push_back(line);
	}
	
	remove( namafile.c_str() );
	
	return tmp;
}

void Helper::saveRaw(string username, string rawData){
	string namafile = "raw_" + username + ".txt";
	ofstream outfile(namafile, ios_base::app | ios_base::out);
	outfile << rawData <<  "\n";
}

bool Helper::viewACK(string username, string dari, char tipe, long time){
	string i_dari;
	char i_tipe;
	long i_time;
	
	string namafile = "view_" + username + ".txt";
	ifstream infile(namafile);
	while (infile >> i_dari >> i_tipe >> i_time)
	{
		if (i_tipe == tipe && tipe == 'G' && i_dari.compare(dari) == 0){
			if (time <= i_time)
				return true;
			else
				return false;
		}else if(i_tipe != 'G' && tipe != 'G' && i_dari.compare(dari) == 0){
			if (time <= i_time)
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

bool Helper::createGroup(string name){
	string i_name;
	ifstream infile("group.txt");
	while (infile >> i_name)
	{
		if (i_name.compare(name) == 0){
			return false;
		}
	}
	ofstream outfile("group.txt", ios_base::app | ios_base::out);
	outfile << name << "\n";
	return true;
}

bool Helper::isGroup(string name){
	string i_name;
	ifstream infile("group.txt");
	while (infile >> i_name)
	{
		if (i_name.compare(name) == 0){
			return true;
		}
	}
	return false;
}

bool Helper::isMemberGroup(string name, string username){
	string i_name;
	string namafile = "group_" + name + ".txt";
	ifstream infile(namafile);
	
	while (infile >> i_name)
	{
		if (i_name.compare(username) == 0){
			return true;
		}
	}
	return false;
}

void Helper::deleteGroup(string name){
	string i_name;
	stringstream ss;
	
	ifstream infile("group.txt");
	while (infile >> i_name)
	{
		if (i_name.compare(name) != 0){
			ss << i_name << "\n";
		}
	}
	ofstream outfile("group.txt", ios_base::out);
	outfile << ss.str();
	
	string namafile = "group_" + name + ".txt";
	remove( namafile.c_str() );
}

bool Helper::joinGroup(string name, string user){
	if (!isGroup(name)) return false;
	string i_name;
	string namafile = "group_" + name + ".txt";
	ifstream infile(namafile);
	while (infile >> i_name)
	{
		if (i_name.compare(user) == 0){
			return false;
		}
	}
	ofstream outfile(namafile, ios_base::app | ios_base::out);
	outfile << user << "\n";
	return true;
}

bool Helper::leaveGroup(string name, string user){
	bool isi = false;
	bool isRemoved = false;
	string i_name;
	stringstream ss;
	string namafile = "group_" + name + ".txt";
	ifstream infile(namafile);
	
	while (infile >> i_name)
	{
		if (i_name.compare(user) == 0){
			isRemoved = true;
		}else{
			isi = true;
			ss << i_name << "\n";
		}
	}
	ofstream outfile(namafile, ios_base::out);
	outfile << ss.str();
	
	if (!isi){
		deleteGroup(name);
	}
	return isRemoved;
}

vector<string> Helper::memberGroup(string name){
	vector<string> member;
	string i_name;
	string namafile = "group_" + name + ".txt";
	ifstream infile(namafile);
	while (infile >> i_name)
	{
		member.push_back(i_name);
	}
	return member;
}