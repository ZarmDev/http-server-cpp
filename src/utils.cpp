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