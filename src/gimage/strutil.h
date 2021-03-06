#ifndef _strutil_h
#define _strutil_h

#include <string>
#include <vector>
#include <map>

std::string tostr(double t);
std::string tostr(unsigned short t);

std::string underscore(std::string str);
std::string de_underscore(std::string str);
std::string filepath_normalize(std::string str);
bool file_exists(const std::string& filename);

std::string getAppConfigFilePath();
std::string getCwdConfigFilePath();

void replace_all(std::string& str, const std::string& from, const std::string& to);
std::vector<std::string> split(std::string s, std::string delim);
std::vector<std::string> bifurcate(std::string strg, char c = ' ');
std::map<std::string, std::string> parseparams(std::string params);
std::string string_format(const std::string fmt, ...);
std::string nexttoken(std::string &strng, std::string delims);

#endif
