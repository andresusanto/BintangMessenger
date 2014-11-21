#pragma once 
#include <iostream>

using namespace std;

class Pesan{
	public:
		string dari; // sebenernya bukan dari sih, lebih tepatnya user yang sedang berkomunikasi dengan pengguna aktif lah
		string pesan;
		string gid;
		
		long waktu;
		char tipe;
		bool read;
};