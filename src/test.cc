#include <cstdio>
#include <iostream>
#include <string>

using namespace std;

string unhex(const char *hex)
{
	int n = 1;
	unsigned char c,b = 0;
	std::string r;
	while ((c = (unsigned char)*(hex++))) {
		if ((c >= 48)&&(c <= 57)) { // 0..9
			if ((n ^= 1)) {
				cout << (char)(b | c-48) << '\n';
				r.push_back((char)(b | (c - 48)));
			}	
			else b = (c - 48) << 4;
		} else if ((c >= 65)&&(c <= 70)) { // A..F
			if ((n ^= 1))
				r.push_back((char)(b | (c - (65 - 10))));
			else b = (c - (65 - 10)) << 4;
		} else if ((c >= 97)&&(c <= 102)) { // a..f
			if ((n ^= 1)) {
				cout << (b | c-87) << '\n';
				r.push_back((char)(b | (c - (97 - 10))));
			}
			else b = (c - (97 - 10)) << 4;
		}
	}

	return r;
}

int main() {
  string hex = "30:31:32:ff:39";
  string t = unhex(hex.c_str());
  cout << t << '\n';
  return 0;
}