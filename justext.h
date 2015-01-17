#ifndef JUSTEXT_H
#define JUSTEXT_H

#include <string>
#include <set>
#include <vector>
#include <html/ParserDom.h>

#include "tools.h"
#include "parseFSM.h"
#include <pcrecpp.h>


#define MAX_LINK_DENSITY_DEFAULT 0.2
#define LENGTH_LOW_DEFAULT  70
#define LENGTH_HIGH_DEFAULT  200
#define STOPWORDS_LOW_DEFAULT  0.30
#define STOPWORDS_HIGH_DEFAULT  0.32
#define NO_HEADINGS_DEFAULT  false
#define MAX_HEADING_DISTANCE_DEFAULT 200


class Justext{


	std::set<std::string> m_stoplist;
	long m_length_low, m_length_high; 
	float m_stopwords_low, m_stopwords_high, m_max_link_density;
    bool m_no_headings;
	bool m_debug;
	bool m_cleanEvalFormat;

	void classify_paragraphs(std::vector<paragraph>& paragraphs);
	void revise_paragraph_classification(std::vector<paragraph>& paragraphs, long max_heading_distance);
	std::string output_default(std::vector<paragraph>& paragraphs, bool no_boilerplate, bool full);

	std::string _get_neighbour(long i, std::vector<paragraph>& paragraphs, bool ignore_neargood, long inc, long boundary);
	std::string get_prev_neighbour(long i, std::vector<paragraph>& paragraphs, bool ignore_neargood);
	std::string get_next_neighbour(long i, std::vector<paragraph>& paragraphs, bool ignore_neargood);

	void makeDebugOutput(std::vector<paragraph>& paragraphs, const std::string& filename, const std::string& url, const std::string& encoding);

public:
	Justext(const std::string& stoplistFile,
			long length_low=LENGTH_LOW_DEFAULT, 
			long length_high=LENGTH_HIGH_DEFAULT,
			float stopwords_low=STOPWORDS_LOW_DEFAULT, 
			float stopwords_high=STOPWORDS_HIGH_DEFAULT, 
			float max_link_density=MAX_LINK_DENSITY_DEFAULT,
			bool no_headings=NO_HEADINGS_DEFAULT);

	std::string getContent(const std::string& html, std::string& encoding, const std::string& url);
	std::string getContent(ParseFSM& fsm, const std::string& in, std::string& encoding, const std::string& url, bool validHtml);
	void log(ParseFSM &fsm, const std::string& msg, int level);
	void setDebug(bool b){m_debug = b;}
	void setCleanEvalFormat(bool b){m_cleanEvalFormat = b;}
};

#endif
