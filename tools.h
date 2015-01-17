#pragma once

#include <string>
#include <set>
#include <vector>
#include <sstream>

void ReplaceAtoB(std::string & str, std::string A, std::string B);
void explode(const std::string& s, std::string e, std::set<std::string>& ret);
void explode(const std::string& s, std::string e, std::vector<std::string>& ret);
std::string implode( const std::set<std::string>& s, const std::string delim);
std::string implode( const std::vector<std::string>& s, const std::string delim);
std::string implode( const std::vector<long>& s, const std::string delim);
std::string trim(const std::string& str, const char leftAndRight = 'b', bool punctuation = false);
bool searchPattern(const std::string& re, const std::string& str);
std::vector<std::string> split(const std::string& s, std::string sep);
std::string detectCodePage(const std::string& html);
std::string file2string(const char* FileName);
std::string to_lower(const std::string& s);

template<typename T>
std::string toString(const T& value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}