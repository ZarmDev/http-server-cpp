// Use header guard to prevent linker issues in server.cpp
#pragma once

#include <string>
using namespace std;

string readFile(const string &path);
void writeToFile(const string &path, const string &data);
// For efficient checking of the first x characters (generated using AI)
// This is declared in the header file because C++ doesn't let you do it inline otherwise
inline bool startsWith(const string &str, const string &prefix)
{
  return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}