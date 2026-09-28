#include "http/HttpParser.h"
#include<algorithm>
#include<cctype>

bool HttpParser::extractLine(std::string& buf,std::string& line)
{
    size_t pos=buf.find("\r\n");
    size_t skip=2;
    if(pos==std::string::npos)
    {
        pos=buf.find('\n');
        skip=1;
    }
    if(pos==std::string::npos)return false;

    line=buf.substr(0,pos);
    buf.erase(0,pos+skip);
    return true;
}

bool HttpParser::parse(std::string& buf)
{
    while(state_!=State::COMPLETE&&state_!=State::ERROR)
    {
        if(state_==State::REQUEST_LINE)
        {
            std::string line;
            if(!extractLine(buf,line))return false;
            if(!parseRequestLine(line)){state_=State::ERROR;return false;}
            state_=State::HEADERS;
        }
        else if(state_==State::HEADERS)
        {
            std::string line;
            if(!extractLine(buf,line))return false;

            if(line.empty())
            {
                state_=State::COMPLETE;
                return true;
            }
            if(!parseHeaderLine(line)){state_=State::ERROR;return false;}
        }
    }
    return state_==State::COMPLETE;
}

bool HttpParser::parseRequestLine(const std::string& line)
{
    size_t p1=line.find(' ');
    if(p1==std::string::npos)return false;
    size_t p2=line.find(' ',p1+1);
    if(p2==std::string::npos)return false;

    request_.method=line.substr(0,p1);
    request_.path=line.substr(p1+1,p2-p1-1);
    request_.version=line.substr(p2+1);
    return !request_.method.empty()&&!request_.path.empty();
}

bool HttpParser::parseHeaderLine(const std::string& line)
{
    size_t colon=line.find(':');
    if(colon==std::string::npos)return false;

    std::string key=line.substr(0,colon);
    std::string value=line.substr(colon+1);

    size_t start=value.find_first_not_of("\t");
    if(start!=std::string::npos)value=value.substr(start);
    else value.clear();

    std::transform(key.begin(),key.end(),key.begin(),[](unsigned char c){return std::tolower(c);});
    request_.headers[key]=value;
    return true;
}

void HttpParser::reset()
{
    state_=State::REQUEST_LINE;
    request_.reset();
}