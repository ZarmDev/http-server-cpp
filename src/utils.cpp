#include <string>
#include <fstream>
#include <sstream>

using namespace std;

string readFile(const string &path)
{
  ifstream file(path, ios::binary);
  if (!file.is_open())
    return "";
  ostringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

void writeToFile(const string &path, const string &data)
{
  ofstream outFile(path, std::ios::out | std::ios::binary);
  if (outFile.is_open())
  {
    outFile << data;
    outFile.close();
  }
}

string replaceAll(string str, const string& from, const string& to) {
    size_t start_pos = 0; // Initialize starting position for search
    // Loop as long as the 'from' substring is found
    while((start_pos = str.find(from, start_pos)) != string::npos) {
        // Replace the found occurrence
        str.replace(start_pos, from.length(), to);
        // Advance the starting position to avoid replacing the newly inserted 'to' string
        // if it contains the 'from' substring itself (e.g., replacing "x" with "yx")
        start_pos += to.length(); 
    }
    return str; // Return the modified string
}