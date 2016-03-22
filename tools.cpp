/*******************************************************************************
 * Copyright (c) 2013 István Endredy.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser Public License v3
 * which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/
 *  
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 ******************************************************************************/
#include "tools.h"

#include <fstream>
#include <algorithm>
#include <pcrecpp.h>

void ReplaceAtoB(std::string & str, std::string A, std::string B)
{
	size_t i = 0;
	while((i = str.find(A, i)) != -1)
	{
		str.replace(i, A.length(), B);
		i += B.length();
	}
}

void explode(const std::string& s, std::string e, std::set<std::string>& ret) {
	size_t iPos = 0, offset = 0;
	size_t iPit = e.length();
	ret.erase(ret.begin(), ret.end());
	while(((iPos = s.find(e, offset)) != s.npos)) {
		ret.insert(s.substr(offset,iPos-offset));
		offset = iPos+iPit;   
	} 
	if(offset < s.length())
		ret.insert(s.substr(offset,s.length()-offset));
}

void explode(const std::string& s, std::string e, std::vector<std::string>& ret) {
	size_t iPos = 0, offset = 0;
	size_t iPit = e.length();
	ret.erase(ret.begin(), ret.end());
	while(((iPos = s.find(e, offset)) != s.npos)) {
		ret.push_back(s.substr(offset,iPos-offset));
		offset = iPos+iPit;   
	} 
	if(offset < s.length())
		ret.push_back(s.substr(offset,s.length()-offset));
}

std::string implode( const std::set<std::string>& s, const std::string delim){

	std::string ret;
	for(std::set<std::string>::const_iterator it = s.begin();
		it != s.end();
		++it){
			if (it != s.begin()) ret += delim;
			ret += *it;
	}
	return ret;
}

//from vector
std::string implode( const std::vector<std::string>& s, const std::string delim){

	std::string ret;
	for(std::vector<std::string>::const_iterator it = s.begin();
		it != s.end();
		++it){
			if (it != s.begin()) ret += delim;
			ret += *it;
	}
	return ret;
}

//from long vector
std::string implode( const std::vector<long>& s, const std::string delim){

	std::string ret;
	for(std::vector<long>::const_iterator it = s.begin();
		it != s.end();
		++it){
			if (it != s.begin()) ret += delim;
			ret += toString(*it);
	}
	return ret;
}

/**
str: input
leftAndRight: left ('l'), right('r') or both ('b'), def: 'b'
punctuation: remove punctuation as well, def. false
*/
std::string trim(const std::string& str, const char leftAndRight, bool punctuation)
{
	//pcrecpp::RE r("^[ \\t]+|[ \\t]+$"); //      [\ \t\r\n\f]
	pcrecpp::RE r("^\\s+|\\s+$");
	if (leftAndRight == 'l')
		r = pcrecpp::RE("^\\s+");
	else if (leftAndRight == 'r')
		r = pcrecpp::RE("\\s+$");

	if (punctuation){
		if (leftAndRight == 'l')
			r = pcrecpp::RE("^[[:punct:]\\s]+");
		else if (leftAndRight == 'r')
			r = pcrecpp::RE("[[:punct:]\\s]+$");
		else
			r = pcrecpp::RE("^[[:punct:]\\s]+|[[:punct:]\\s]+$");
	}
	std::string out = str;
	r.GlobalReplace("", &out);
	return out;
}

bool searchPattern(const std::string& re, const std::string& str){
	pcrecpp::RE r(re);
	pcrecpp::StringPiece i = str;
	if (r.PartialMatch(i))
		return true;
	return false;

}


//TODO: nem jol splittel: "##a", "#" eseten nem 3 elemu tombot ad
std::vector<std::string> split(const std::string& in, std::string sep)
{
	std::string s = in;
    std::vector<std::string> v;

	if (sep.size() == 0) sep = " ,\\.?:;!+\"%/=\\(\\)\\n-";
	pcrecpp::RE_Options opt; 
	opt.set_utf8(true);

    pcrecpp::StringPiece input(s);
	pcrecpp::RE re("([^"+sep+"]+)["+sep+"]*");

	int consumed = 0;
	std::string match;
	const pcrecpp::Arg *args[1];
	pcrecpp::Arg arg0 = &match;
	args[0] = &arg0;

	while (re.DoMatch(input, pcrecpp::RE::UNANCHORED, &consumed, args, 1)) {
		v.push_back(match);
		input.remove_prefix(consumed);
	}

    return v;
}

std::string detectCodePage(const std::string& html){

	pcrecpp::RE_Options opt; 
	opt.set_caseless(true);

	//pcrecpp::RE  r("<meta http-equiv=['\"]Content-type['\"] +content=['\"]text/html; +charset=(.*?)['\"]", opt);
	pcrecpp::RE r1("<meta\\s+http-equiv=['\"]?content-type['\"]?\\s+content=['\"]?[^'\"]*charset=([^'\"]+)", opt);
    pcrecpp::RE r2("<meta\\s+content=['\"]?[^'\"]*charset=([^'\"]+)['\"]?\\s+http-equiv=['\"]?content-type['\"]?", opt);
    pcrecpp::RE r3("<meta\\s+http-equiv=['\"]?charset['\"]?\\s+content=['\"]?([^'\"]+)", opt);
    pcrecpp::RE r4("<meta\\s+content=['\"]?([^'\"]+)['\"]?\\s+http-equiv=['\"]?charset['\"]?", opt);
    pcrecpp::RE r5("<meta\\s+charset=['\"]?([^'\"]+)", opt);

	pcrecpp::StringPiece out;

	if (r1.PartialMatch(html, &out))
		return out.as_string();
	else if (r2.PartialMatch(html, &out))
		return out.as_string();
	else if (r3.PartialMatch(html, &out))
		return out.as_string();
	else if (r4.PartialMatch(html, &out))
		return out.as_string();
	else if (r5.PartialMatch(html, &out))
		return out.as_string();

	return "";
}

std::string file2string(const char* FileName)
{
	std::string content;
	//std solution
	std::ifstream t(FileName);
	if (!t.good())
		return content;

	//check it is a directory (2012.12.11. weblib)
	//http://stackoverflow.com/questions/9591036/ifstream-open-doesnt-set-error-bits-when-argument-is-a-directory
	getline(t, content);
	content.erase();
	if (t.bad() || t.fail())
		return content;

	t.seekg(0, std::ios::end);   
	content.reserve(t.tellg());
	t.seekg(0, std::ios::beg);

	content.assign((std::istreambuf_iterator<char>(t)),
				std::istreambuf_iterator<char>());
	return content;
}

std::string to_lower(const std::string& in){
	std::string out = in; 
	std::transform(out.begin(), out.end(), out.begin(), ::tolower);
	return out;
}