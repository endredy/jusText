#include "justext.h"
#include "tools.h"

#include <stdio.h>
#include <string.h>


int main(int argc, char* argv[])
{

	std::string url = "http://www.autoblog.com/2015/01/16/german-gp-f1-moving-hockenheim/"; 
	std::string html = file2string("test/sample.html");
	std::string encoding = detectCodePage(html);

	//create jusText object, with stopwords file
	Justext j("stoplists/English.txt");

	//if you want a debug html output as well
	j.setDebug(true); 
	
	//call justext algorithm
	std::string result = j.getContent(html, encoding, url);
	printf("result: %s\n", result.c_str());
	return 0;
}

