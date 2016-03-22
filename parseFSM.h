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
#ifndef PARSEFSM_H
#define PARSEFSM_H

#include <string>
#include <set>
#include <vector>
#include <html/ParserDom.h>
#include "tools.h"

const std::string paragraphTags = "blockquote,caption,center,col,colgroup,dd,\
div,dl,dt,fieldset,form,legend,optgroup,option,\
p,pre,table,td,textarea,tfoot,th,thead,tr,\
ul,li,h1,h2,h3,h4,h5,h6";

struct paragraph{
	std::string dom_path;
    std::vector<std::string> text_nodes;
	std::string text;
	std::string cfclass;

	std::string finalclass;
	bool heading;
	bool bullet;
    long word_count;
    long linked_char_count;
    long tag_count;

    long stopword_count;
    float stopword_density;
    float link_density;

	long m_htmlPosition1;
	long m_htmlPosition2;
	std::string m_tag, m_htmlSrc;
	std::vector<std::string> m_originalTags;
	std::vector<std::string> m_htmlParents;

};

class Justext;
class ParseFSM
{

	std::vector<paragraph> m_paragraphs;
	//std::vector<paragraph> m_nodes; //more details just for learning
	paragraph m_currParagraph;
	bool m_link;
	bool m_br;
	long m_dontcare;
	std::vector<std::string> m_dom;

	std::set<std::string> m_pTags;

	std::set<std::string> m_dontcareTags;

	std::string m_url;
	std::string m_convertedHtml;
	bool m_invalid_characters;
	bool m_first_pass;
	bool m_haveGood;
	bool m_learning;
	bool m_basicJustext; //ha csak sima justext, nem pedig aranyaso valtozat (ekkor mas nevu html-be teszi a debug infot)
	Justext& m_parent;

public:
	std::vector<paragraph>& getPara(){return m_paragraphs;}
	const std::vector<paragraph>& getParaConst()const {return m_paragraphs;}

	ParseFSM(Justext& p)
		:	m_parent(p),
			m_invalid_characters(false),
			m_first_pass(true),
			m_haveGood(false),
			m_basicJustext(true)
	{

		m_dontcareTags.insert("head");
		m_dontcareTags.insert("script");
		m_dontcareTags.insert("style");

		std::string d(",");
		explode(paragraphTags, d, m_pTags);

		m_link = false;
        m_br = false;
		m_dontcare = 0;
        _start_new_pragraph();

	}

	void setUrl(const std::string& u){m_url = u;};
	std::string& getUrl(){return m_url;};
	void setInvalidChars(bool b){m_invalid_characters = b;};
	void setFirstPass(bool b){m_first_pass = b;};
	bool getFirstPass(){return m_first_pass;};
	bool getInvalidChars(){return m_invalid_characters;}
	Justext& getParent(){return m_parent;};
	std::string& getConvertedHtml(){return m_convertedHtml;}
	void setConvertedHtml(const std::string& s){ m_convertedHtml = s;}

	bool getGood(){return m_haveGood;}
	void setGood(bool b){m_haveGood = b;}

	bool getLearning(){return m_learning;}
	void setLearning(bool b){m_learning = b;}

	bool getBasicJustext(){return m_basicJustext;}
	void setBasicJustext(bool b){m_basicJustext = b;}
	
	void resetDom(){m_dom.erase(m_dom.begin(), m_dom.end()); m_dontcare=0;}

//	bool isDontcareTag(const std::string& tag){ return m_dontcareTags.find(tag) != m_dontcareTags.end(); }
//	bool isTextTag(const std::string& tag){ return m_pTags.find(tag) != m_pTags.end(); }

	std::string getContentHtmlCxx(const std::string& in, const std::string& url, bool validHtml);
	void walk_tree(tree<htmlcxx::HTML::Node>::iterator  const & it, int depth );
	void handleNode(tree<htmlcxx::HTML::Node>::iterator it, int depth);

	void _start_new_pragraph(){
		m_currParagraph.dom_path = (implode(m_dom, "."));

		if (m_currParagraph.text_nodes.size() != 0){
			m_currParagraph.text = implode(m_currParagraph.text_nodes, std::string(""));

			//neha darabokban adta a szoveget, tobb szot szamolt
			std::vector<std::string> words = split(m_currParagraph.text, std::string(""));
			m_currParagraph.word_count = (long)words.size();

			m_paragraphs.push_back(m_currParagraph);
		}
        //m_currParagraph.dom_path = (implode(m_dom, "."));
		m_currParagraph.text_nodes.erase(m_currParagraph.text_nodes.begin(), m_currParagraph.text_nodes.end());
		m_currParagraph.text.clear();
		m_currParagraph.linked_char_count = 0;
		m_currParagraph.word_count = 0;
		m_currParagraph.tag_count = 0;
		m_currParagraph.stopword_count = 0;
		m_currParagraph.stopword_density = 0.0;

		m_currParagraph.m_htmlPosition1 = 0;
		m_currParagraph.m_htmlPosition2 = 0;
		m_currParagraph.m_tag.erase();
		m_currParagraph.m_htmlSrc.erase();
		m_currParagraph.m_originalTags.erase(m_currParagraph.m_originalTags.begin(), m_currParagraph.m_originalTags.end()); 
		m_currParagraph.m_htmlParents.erase(m_currParagraph.m_htmlParents.begin(), m_currParagraph.m_htmlParents.end());

	}
};


#endif