/*******************************************************************************
 * Copyright (c) 2013 István Endredy.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser Public License v3
 * which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/
 *  
 * JusText is written by Jan Pomikalek, in python.
 * https://code.google.com/p/justext/
 *  It has very good quality, so I have rewritten it in c++. :)
 * 
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 ******************************************************************************/
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

