#include "justext.h"
#include <fstream>
#include "tools.h"


Justext::Justext(
				const std::string& stoplistFile,
				long length_low, 
				long length_high,
				float stopwords_low, 
				float stopwords_high, 
				float max_link_density,
				bool no_headings)
	:
	m_debug(false),
	m_length_low(length_low),
	m_length_high(length_high),
	m_stopwords_low(stopwords_low),
	m_stopwords_high(stopwords_high),
	m_max_link_density(max_link_density),
	m_no_headings(no_headings),
	m_cleanEvalFormat(false)
{
	if (stoplistFile.size()){
		std::string nl("\r?\n"), c = file2string(stoplistFile.c_str());
		c = to_lower(c);
		std::vector<std::string> v = split(c, "\r?\n");
		copy(v.begin(), v.end(), inserter(m_stoplist, m_stoplist.end()));
	}
}

void Justext::classify_paragraphs(std::vector<paragraph>& paragraphs){

	for(size_t i = 0; i < paragraphs.size(); i++){
		

		int length = (int)paragraphs[i].text.size();
        int stopword_count = 0;
		std::vector<std::string> words = split(paragraphs[i].text, std::string(""));
		for(std::vector<std::string>::const_iterator it = words.begin();
			it != words.end();
			++it){
				if (m_stoplist.find(to_lower(*it)) != m_stoplist.end())
					stopword_count++;
		}
        long word_count = paragraphs[i].word_count;
		float stopword_density = 0, link_density = 0;
		if (word_count == 0){
            stopword_density = 0;
            link_density = 0;
		}else{
            stopword_density = (float)1.0 * stopword_count / word_count;
            link_density = (float)paragraphs[i].linked_char_count / length;
		}
        paragraphs[i].stopword_count = stopword_count;
        paragraphs[i].stopword_density = stopword_density;
        paragraphs[i].link_density = link_density;

        paragraphs[i].heading = false;
		paragraphs[i].bullet = false;
		if (!m_no_headings && searchPattern(std::string("(^h\\d|\\.h\\d)"), paragraphs[i].dom_path))
			paragraphs[i].heading = true;
		else if (searchPattern(std::string("(^li|\\.li)"), paragraphs[i].dom_path))
			paragraphs[i].bullet = true;

		if (link_density > m_max_link_density){
            paragraphs[i].cfclass = "bad";
		}else if (paragraphs[i].text.find("u\xa9") != std::string::npos ||
			paragraphs[i].text.find("&copy") != std::string::npos){
            paragraphs[i].cfclass = "bad";
		}else if (searchPattern(std::string("(^select|\\.select)"), paragraphs[i].dom_path))
            paragraphs[i].cfclass = "bad";
		else{
			if (length < m_length_low){
                if (paragraphs[i].linked_char_count > 0)
                    paragraphs[i].cfclass = "bad";
                else
                    paragraphs[i].cfclass = "short";
			}else{
				if (stopword_density >= m_stopwords_high){
                    if (length > m_length_high)
                        paragraphs[i].cfclass = "good";
                    else
                        paragraphs[i].cfclass = "neargood";
				}else if (stopword_density >= m_stopwords_low)
                    paragraphs[i].cfclass = "neargood";
                else
                    paragraphs[i].cfclass = "bad";
			}
		}
	}

}

std::string Justext::_get_neighbour(long i, std::vector<paragraph>& paragraphs, bool ignore_neargood, long inc, long boundary){
	while (i + inc != boundary){
        i += inc;
		std::string c = paragraphs[i].finalclass;
        if (c.compare("good") == 0 ||
			c.compare("bad") == 0)
            return c;
        if (c.compare("neargood") == 0 && !ignore_neargood)
            return c;
	}
    return "bad";
}

std::string Justext::get_prev_neighbour(long i, std::vector<paragraph>& paragraphs, bool ignore_neargood){
    //Return the class of the paragraph at the top end of the short/neargood
    //paragraphs block. If ignore_neargood is True, than only 'bad' or 'good'
    //can be returned, otherwise 'neargood' can be returned, too.
    return _get_neighbour(i, paragraphs, ignore_neargood, -1, -1);
}

std::string Justext::get_next_neighbour(long i, std::vector<paragraph>& paragraphs, bool ignore_neargood){
    //Return the class of the paragraph at the bottom end of the short/neargood
    //paragraphs block. If ignore_neargood is True, than only 'bad' or 'good'
    //can be returned, otherwise 'neargood' can be returned, too.
	return _get_neighbour(i, paragraphs, ignore_neargood, 1, (long)paragraphs.size());
}

void Justext::revise_paragraph_classification(std::vector<paragraph>& paragraphs, long max_heading_distance){
    //Context-sensitive paragraph classification. Assumes that classify_pragraphs
    //has already been called.
    //# copy classes
    for(size_t i = 0; i < paragraphs.size(); i++)
        paragraphs[i].finalclass = paragraphs[i].cfclass;

    //# good headings
	size_t j = 0, distance = 0;
	for(size_t i = 0; i < paragraphs.size(); i++){
        if (!(paragraphs[i].heading && 
			  paragraphs[i].finalclass.compare("short") == 0))
            continue;
        j = i + 1;
        distance = 0;
		while (j < paragraphs.size() && (long)distance <= max_heading_distance){
			if (paragraphs[j].finalclass == "good"){
                paragraphs[i].finalclass = "neargood";
                break;
			}
            distance += paragraphs[j].text.size();
            j++;
		}
	}

    //# classify short
	std::vector<std::string> new_classes; new_classes.resize(paragraphs.size());
	for(long i = 0; i < (long)paragraphs.size(); i++){
        if (paragraphs[i].finalclass.compare("short"))
            continue;
		std::string prev_neighbour = get_prev_neighbour(i, paragraphs, true);
        std::string next_neighbour = get_next_neighbour(i, paragraphs, true);
        //neighbours = set((prev_neighbour, next_neighbour))
        if (prev_neighbour.compare("good") == 0 &&
			next_neighbour.compare("good") == 0)
            new_classes[i] = "good";
        else if (prev_neighbour.compare("bad") == 0 &&
				next_neighbour.compare("bad") == 0)
            new_classes[i] = "bad";
        //# it must be set(['good', 'bad'])
        else if ((prev_neighbour.compare("bad") == 0 && get_prev_neighbour(i, paragraphs, false) == "neargood") || 
             (next_neighbour.compare("bad") == 0 && get_next_neighbour(i, paragraphs, false) == "neargood"))
            new_classes[i] = "good";
        else
            new_classes[i] = "bad";
	}

	for(size_t i = 0; i < paragraphs.size(); i++)
        if (new_classes[i].size() != 0) paragraphs[i].finalclass = new_classes[i];

    //# revise neargood
	for(long i = 0; i < (long)paragraphs.size(); i++){
        if (paragraphs[i].finalclass.compare("neargood"))
            continue;
		std::string prev_neighbour = get_prev_neighbour(i, paragraphs, true);
        std::string next_neighbour = get_next_neighbour(i, paragraphs, true);
        if (prev_neighbour.compare("bad") == 0 &&
			next_neighbour.compare("bad") == 0)
            paragraphs[i].finalclass = "bad";
        else
            paragraphs[i].finalclass = "good";
	}

    //# more good headings
	for(size_t i = 0; i < paragraphs.size(); i++){
        if (!(paragraphs[i].heading && 
			  paragraphs[i].finalclass.compare("bad") == 0 && 
			  paragraphs[i].cfclass.compare("bad") != 0))
            continue;
        j = i + 1;
        distance = 0;
		while (j < paragraphs.size() && (long)distance <= max_heading_distance){
			if (paragraphs[j].finalclass.compare("good") == 0){
                paragraphs[i].finalclass = "good";
                break;
			}
            distance += paragraphs[j].text.size();
            j += 1;
		}
	}
}

void Justext::makeDebugOutput(std::vector<paragraph>& paragraphs, const std::string& filename, const std::string& url, const std::string& encoding){
	std::string html, currClass, params;


	params += "details.push(";

	html += "<html>\n";
	html += "<head>\n";
	html += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset="+encoding+"\" />\n";
	html += "<link rel=\"stylesheet\" type=\"text/css\" href=\"css/style.css\" />\n";
	html += "<script type=\"text/javascript\" src=\"js/jquery.min.js\"></script>";
	html += "<script type=\"text/javascript\" src=\"js/jquery.qtip.min.js\"></script>";
	html += "<script type=\"text/javascript\" src=\"js/tooltip.js\"></script>";
	html += "<title>jusText demo</title>\n";
	html += "</head>\n";
	html += "<body>\n";
	html += "original page: <a href='" + url + "'>"+url+"</a>\n";
	html += "<div id=\"output_wrapper\">\n";
	for(size_t i = 0; i < paragraphs.size(); i++){
		currClass = paragraphs[i].finalclass;
		if (currClass.size() == 0)
			currClass = paragraphs[i].cfclass;

		if (i != 0) params += ",";
		params += "{id: 'pd" + toString(i) + "', parameters:{";
		params += "final_class:'"+currClass+"',";
		params += "	context_free_class:'"+paragraphs[i].cfclass+"',";
		params += "	heading:'"+std::string(paragraphs[i].heading?"True":"False")+"',";
		params += "	length: "+toString(paragraphs[i].text.size())+",";
		params += "	characters_within_links: "+toString(paragraphs[i].linked_char_count)+",";
		params += "	link_density:'"+toString(paragraphs[i].link_density)+"',";
		params += "	number_of_words: "+toString(paragraphs[i].word_count)+",";
		params += "	number_of_stopwords: "+toString(paragraphs[i].stopword_count)+",";
		params += "	stopword_density: '"+toString(paragraphs[i].stopword_density)+"',";
		//params += "	dom:'"+paragraphs[i].dom_path+"',";
		params += "	other:'"+toString(i)+". paragraph',";

		//what is its reason
		std::string reason = "<b>reason</b> of ";
		if (paragraphs[i].link_density > m_max_link_density){
			reason += "bad: link_density > " + toString(m_max_link_density);
		}else if (paragraphs[i].text.find("u\xa9") != std::string::npos ||
			paragraphs[i].text.find("&copy") != std::string::npos){
			reason += "bad: spec. characters";
		}else if (searchPattern(std::string("(^select|\\.select)"), paragraphs[i].dom_path))
            reason += "bad: select tag in DOM";
		else{
			if (paragraphs[i].text.size() < (size_t)m_length_low){
                if (paragraphs[i].linked_char_count > 0)
					reason += "bad: length < " + toString(m_length_low) + " && linked_char_count > 0";
                else
					reason += "short: length < " + toString(m_length_low) + " && linked_char_count == 0";
			}else{
				if (paragraphs[i].stopword_density >= m_stopwords_high){
                    if (paragraphs[i].text.size() > (size_t)m_length_high)
                        reason += "good: stopword_density >= "+toString(m_stopwords_high)+" && length > " + toString(m_length_high);
                    else
						reason += "neargood: stopword_density >= "+toString(m_stopwords_high)+" && length <= " + toString(m_length_high);
				}else if (paragraphs[i].stopword_density >= m_stopwords_low)
					reason += "neargood: stopword_density < "+toString(m_stopwords_high)+" && stopword_density >= " + toString(m_stopwords_low);
                else
					reason += "bad: stopword_density < "+toString(m_stopwords_high)+" && stopword_density < " + toString(m_stopwords_low);
			}
		}
		params += "	reason: '"+reason+"'}}\n";

		html += "<p class=\"" + currClass + "\" qtip-content=\"pd" + toString(i) + "\">" + paragraphs[i].text + "</p>";
	}
	params += "	);";
	html += "<script>"+params+"</script>";
	html += "</div><br/><br/><br/><br/><br/><br/><br/><br/><br/></html>";

	std::ofstream f;
	f.open(filename.c_str(), std::ios::out);
	f << html;
	f.close();
}

std::string wrapText(const std::string& str, int len){
	std::string r(str);
	size_t lineWidth=0, lastSpace=0, nextS=0, nextN=0;
	for(size_t i = 0; i<r.size(); ){
		nextS = r.find_first_of(" \n", i);
		if (nextS == r.npos) break;

		if (r.at(nextS) == '\n'){
			lineWidth = 0;
			lastSpace = nextS;
			i = nextS + 1;
			continue;
		}

		lineWidth += nextS - lastSpace + 1;

		if (lineWidth > (size_t)len){
			r.insert(lastSpace, "\n");
			lineWidth = nextS - lastSpace + 1;
		}

		i = nextS + 1;
		lastSpace = nextS;
	}
	return r;
}

std::string Justext::output_default(std::vector<paragraph>& paragraphs, bool no_boilerplate, bool full){
    //Outputs the paragraphs as:
    //<tag> text of the first paragraph
    //<tag> text of the second paragraph
    //...
    //where <tag> is <p>, <h> or <b> which indicates
    //standard paragraph, heading or boilerplate respecitvely.

	std::string tag, out;
	for(size_t i = 0; i < paragraphs.size(); i++){
		if (paragraphs[i].finalclass.compare("good") == 0){
            if (paragraphs[i].heading)
                tag = "h";
            else if (paragraphs[i].bullet)
				tag = "l";
			else
                tag = "p";
			if (!full) out += "\n" + paragraphs[i].text + "\n";
		}else{
            if (no_boilerplate)
                continue;
            else
                tag = "b";
		}
		ReplaceAtoB(paragraphs[i].text, "&nbsp;", " ");
		ReplaceAtoB(paragraphs[i].text, "&quot;", "\"");
		ReplaceAtoB(paragraphs[i].text, "&gt;", ">");
		ReplaceAtoB(paragraphs[i].text, "&lt;", "<");
		if (full) {
			//paragraphs[i].text = wrapText(paragraphs[i].text, 60);
			std::string tmp = wrapText(paragraphs[i].text, 80);
			if (paragraphs[i].m_tag.compare("pre") == 0){
				pcrecpp::RE("\n>?[ \t]?\n>?").GlobalReplace(std::string("\n\n<"+tag+">"), &tmp);
			}
			out += "<"+tag+">"+tmp+"\n\n";
		}
	}

	return out;
}

void Justext::log(ParseFSM &fsm, const std::string& msg, int level){

	//printf("%s\n", msg.c_str());
}

std::string Justext::getContent(const std::string& in, std::string& encoding, const std::string& url){

	ParseFSM fsm(*this);
	return getContent(fsm, in, encoding, url, false);
}

std::string Justext::getContent(ParseFSM& fsm, const std::string& in, std::string& encoding, const std::string& url, bool validHtml){

	std::string out;
//	if (in.size() == 0) return out;

	fsm.getContentHtmlCxx(in, url, validHtml);
	classify_paragraphs(fsm.getPara());
	revise_paragraph_classification(fsm.getPara(), MAX_HEADING_DISTANCE_DEFAULT);

	if (m_debug) makeDebugOutput(fsm.getPara(), "test/debugJusText.html", url, encoding);
	
	out = output_default(fsm.getPara(), true, m_cleanEvalFormat);
	if (out.size() == 0){
		fsm.setGood(false);
	}else 
		fsm.setGood(true);

	return out;
}
