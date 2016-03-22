
## JusText boilerplate removal algorithm in c++


JusText is written by Jan Pomikalek, in python. 
https://code.google.com/p/justext/


It has very good quality, so I have rewritten it in c++. :)

## Background

This work is a part of a great project, which compared more boilerplate removal algorithms, and aims to create a better web crawler.
JusText gives better quality if we add a web site specific html preprocess. This ideal was called 'gold miner' algorithm and was introduced at MICAI conference, 2013 (http://polibits.gelbukh.com/2013_48/More%20Effective%20Boilerplate%20Removal%20-%20the%20GoldMiner%20Algorithm.pdf) 

## Dependencies

It has two dependencies: pcrecpp and htmlcxx library. Last one is attached to project, it helps to parse the html. 
Other one has to be linked.

## Compile

windows:  
vcproj files might help. (goldMinerTester.vcproj depends on src/goldMiner.vcproj, jusText/jusText.vcproj, jusText/htmlcxx-0.84/htmlcxx.vcproj)  
  
linux:  
make  


## Usage

Example for usage is in the test.cpp

	//create jusText object with stopwords file
	Justext j("stoplists/English.txt");

	//if you want a debug html output as well
	j.setDebug(true); 
	
	//call justext algorithm
	std::string result = j.getContent(html, encoding, url);


## Features

When i tested the tool on web pages, it took longer time to debug: why a given paragraph is bad or good.
That's why it can show in the tooltip of the paragraphs not only the stopwords/length/etc info, but the reason as well: why it is bad or good according to jusText.

For example a tooltip of paragraphs you can see this:
 final_class:'bad',	
 context_free_class:'bad',	
 heading:'False',	
 length: 8,	
 characters_within_links: 8,	
 link_density:1,	
 number_of_words: 2,	
 number_of_stopwords: 1,	
 stopword_density: '0.5',	
 reason of bad: link_density > 0.2


It can produce a html debug output as well, which is based on online demo (http://nlp.fi.muni.cz/projects/justext/)
If debug is set true, it creates a html page to investigate the results:

	//if you want a debug html output as well
	j.setDebug(true); 

And the debug file will be created: test/debugJusText.html
Its css and js are there, too.

Istvan Endredy