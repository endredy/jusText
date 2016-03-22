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
#include "parseFSM.h"
#include "justext.h"
#include <pcrecpp.h>


std::string ParseFSM::getContentHtmlCxx(const std::string& in, const std::string& url, bool validHtml){
//with htmlcxx parser

	std::string out;

	try
	{
		htmlcxx::HTML::ParserDom parser;
		tree<htmlcxx::HTML::Node> dom = parser.parseTree(in);

		//perhaps it is better: http://stackoverflow.com/questions/11480570/htmlcxx-api-usage
		walk_tree(dom.begin(), 0);
	} catch (std::exception &e) {
		m_parent.log(*this, e.what(), 1);
	} catch (...) {
		m_parent.log(*this, "Unknow exception caught in getContentHtmlCxx()", 1);
	}

	return out;
}

void ParseFSM::walk_tree( tree<htmlcxx::HTML::Node>::iterator  const & pit, int depth ){
	tree<htmlcxx::HTML::Node>::iterator curr;
	depth++;
	for ( curr = pit.node->first_child;
		curr != NULL;
		curr = curr.node->next_sibling ){

		if (curr->isTag() &&
			m_dontcareTags.find(curr->tagName()) != m_dontcareTags.end()){
			continue;
		}

		handleNode( curr, depth);
		walk_tree( curr, depth);
	}

	depth--;
	std::string tag = to_lower(pit->tagName());
	if (m_pTags.find( tag ) != m_pTags.end()) 
	//if (m_pTags.find( dom->tagName() ) != m_pTags.end())
		_start_new_pragraph();
	if (tag.compare("a")==0)
			m_link = false;
	//cout<<it->closingText();
}

void ParseFSM::handleNode(tree<htmlcxx::HTML::Node>::iterator it, int depth){

	std::string name = to_lower(it->tagName());
	if(name.length() == 0 || it->isComment())
		return;//continue;

	while((int)m_dom.size() >= depth)
		m_dom.pop_back();

	if (it->isTag()){
		if (m_dontcareTags.find(name) != m_dontcareTags.end()){
			  it.skip_children();
			  return;//continue;
		}

	//int depth = dom.depth(it);

	//manage dom path
	if ((int)m_dom.size() < depth)
		m_dom.push_back( name );
	else{
//		while((int)m_dom.size() >= depth)
//			m_dom.pop_back();
		m_dom.push_back( name );
	}


	if (m_pTags.find(name) != m_pTags.end() || (name.compare("br")==0 && m_br)){
		if (name.compare("br")==0)
			 /*   # the <br><br> is a paragraph separator and should
				# not be included in the number of tags within the
				# paragraph*/
			   m_currParagraph.tag_count--;
		_start_new_pragraph();
	}else{
		if (name.compare("br")==0)
			m_br = true;
		else
			m_br = false;
		if (name.compare("a")==0)
			m_link = true;
		//else
		//	m_link = false;
		m_currParagraph.tag_count++;
	}

	if (m_currParagraph.m_tag.size() == 0)
		m_currParagraph.m_tag = name;
	if (m_currParagraph.m_originalTags.size() == 0)
		m_currParagraph.m_originalTags.push_back(it->text());
	}

	if (m_currParagraph.m_htmlPosition1 == 0){
		m_currParagraph.m_htmlPosition1 = it->offset();
		m_currParagraph.m_htmlPosition2 = it->offset() + it->length();
	}else if (m_br)
		m_currParagraph.m_htmlPosition2 = it->offset() + it->length();

	if (!it->isComment() && !it->isTag()){
		//text data
		std::string content = it->text();

		pcrecpp::RE("&nbsp;").GlobalReplace(" ", &content); //decode non breaking spaces
		bool pre = m_currParagraph.m_tag.compare("pre") == 0;
		if (pre)
			pcrecpp::RE("[ \t]+").GlobalReplace(" ", &content); //in this case we need newline characters
		else
			pcrecpp::RE("\\s+").GlobalReplace(" ", &content); //eliminate them, combine white spaces into one
		
		//pcrecpp::RE("^\\s+|\\s+$").GlobalReplace(" ", &content); //starting white spaces into one
		//content = trim(content);
		if (trim(content).size() == 0) {
			if (m_br){
				if (m_dom.size()> 2 &&
					m_dom[ m_dom.size()-2 ].compare("b") == 0 &&
					m_dom[ m_dom.size()-1 ].compare("br") == 0 && 
					m_currParagraph.text_nodes.size() == 1){
					//ha egy elemu es BOLD, akkor sztem ez cim
					m_currParagraph.heading = true;
					_start_new_pragraph();
				}else if (m_currParagraph.text_nodes.size() > 0) //ha van mar benne: legyen sortores
					m_currParagraph.text_nodes.push_back("\n");
			}else if (pre)
				m_currParagraph.text_nodes.push_back(" ");
			return;//continue;
		}

		if(m_br){ 
			content = "\n" + content;
			m_currParagraph.m_htmlPosition2 = it->offset() + it->length();
		}
		//text = re.sub("\s+", " ", content)
		m_currParagraph.text_nodes.push_back(content);
		m_currParagraph.m_htmlPosition2 = it->offset() + it->length(); //ez mindig igaz?
		if (m_link){
			m_currParagraph.linked_char_count += (long)content.size();
		}
		m_br = false;
		//m_link = false;

		if (m_learning){
			//beszerezzuk a szuleit, elore, a tanulashoz jo lesz: :)
			//csak akkor, ha tanulunk
			tree<htmlcxx::HTML::Node>::iterator parentIt = it;

			while(m_currParagraph.m_htmlParents.size() <= 8){
				parentIt = parentIt.node->parent;
				if (parentIt == NULL) 
					break;
				if (parentIt->isTag() /*&& parentIt.number_of_children() > 0*/)
					m_currParagraph.m_htmlParents.push_back( parentIt->text() );
			}	
		}
	}
}

