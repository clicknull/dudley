/**
 * @file parse.cpp
 * @author Brandon Miller
 * @brief parse Dudleyfuzz file
 */
 
#include "../include/parse.hpp"

namespace Parser
{
    /// constructor for DudleyParser class
    DudleyParser::DudleyParser(std::string filepath)
    {
        this->infile  = make_shared<std::ifstream>(filepath);
    }
    
    /// deconstructor for DudleyParser class
    DudleyParser::~DudleyParser()
    {
        // intentionally left blank
    }
    
    /// ensure that quotes contain '\' before them
    bool DudleyParser::good_quotes(std::string &s)
    {
        int i;
        // start iterating after first quote and stop before last
        for (i = 2; i < s.size() - 1; i++)
        {
            if ((s[i] == '"') && (s[i - 1] != '\\'))
                return false;
        }
        
        return true;
    }
    
    /// checks if string is an integer
    bool DudleyParser::is_integer(std::string &s)
    {
          return (s.find_first_not_of("0123456789") == string::npos);
    }
    
    /// check if argument is a string wrapped in quotations
    bool DudleyParser::is_string(std::string &s)
    {
        if (s.front() == '"' && s.back() == '"')
        {
            if (!good_quotes(s))
                return false;
                
            return true;
        }
        else if (s.front() == '\'' && s.back() == '\'')
        {
            size_t n = std::count(s.begin(), s.end(), '\'');
            if (n != 2)
                return false;
                
            return true;
        }
        else
        {
            return false;
        }
    }
    
    /// validate the types can be converted for d_string
    STATUS DudleyParser::check_d_string(std::vector<std::string> &sv)
    {
        if (!DudleyParser::is_string(sv[1]))
        {
            ERRPRINT("first argument for d_string is invalid: " + sv[1]);
            return STATUS::ERROR;
        }

        DBGPRINT("COMMAND: " + sv[0] + " ARG1: " + sv[1]);
        return STATUS::GOOD;
    }
    
    STATUS DudleyParser::check_d_string_repeat(std::vector<std::string> &sv)
    {
        if (!DudleyParser::is_string(sv[1]))
        {
            ERRPRINT("first argument for d_string_repeat is invalid: " + sv[1]);
            return STATUS::ERROR;
        }
        
        if (!DudleyParser::is_integer(sv[2]))
        {
            ERRPRINT("second argument for d_string_repeat is invalid: " + sv[2]);
            return STATUS::ERROR;
        }
        
        DBGPRINT("COMMAND: " + sv[0] + " ARG1: " + sv[1] + " ARG2: " + sv[2]);
        return STATUS::GOOD;
    }
    
    /// return the status of infile
    bool DudleyParser::is_open()
    {
        return this->infile->is_open();
    }
    
    /// trim trailing and leading whitespace
    void DudleyParser::trim_whitespace(string &s)
    {
        size_t p = s.find_first_not_of(" \t");
        s.erase(0, p);
    
        p = s.find_last_not_of(" \t");
        if (std::string::npos != p)
            s.erase(p + 1, s.size());
    }
    
    /// remove comments from line
    void DudleyParser::trim_comments(string &s)
    {
        size_t p = s.find_first_of(";;");
        if (p == std::string::npos)
            return;
            
        s.erase(p, s.size());
    }
    
    /// check function name and command length
    STATUS DudleyParser::check_func(std::vector<std::string> &sv)
    {
        if (sv.size() == 0)
            return STATUS::ERROR;
            
        if (sv[0] == "d_string")
        {
            if (sv.size() != 2)
                return STATUS::ERROR;
            else
                return DudleyParser::check_d_string(sv);
        }
        else if (sv[0] == "d_string_repeat")
        {
            if (sv.size() != 3)
                return STATUS::ERROR;
            else
                return DudleyParser::check_d_string_repeat(sv);
        }
        else if (sv[0] == "d_string_variable")
        {
            if (sv.size() != 2)
                return STATUS::ERROR;
            else
                return DudleyParser::check_d_string(sv);
        }
        else if (sv[0] == "d_binary")
        {
            if (sv.size() != 2)
                return STATUS::ERROR;
        }
        else if (sv[0] == "d_binary_repeat")
        {
            if (sv.size() != 3)
                return STATUS::ERROR;
        }
        else if (sv[0] == "d_block_start")
        {
            if (sv.size() != 2)
                return STATUS::ERROR;
        }
        else if (sv[0] == "d_block_end")
        {
            if (sv.size() != 2)
                return STATUS::ERROR;
        }
        else if (sv[0] == "d_read")
        {
            if (sv.size() != 1)
                return STATUS::ERROR;
        }
        else if (sv[0] == "d_readline")
        {
            if (sv.size() != 1)
                return STATUS::ERROR;
        }
        else if (sv[0] == "d_hexdump")
        {
            if (sv.size() != 1)
                return STATUS::ERROR;
        }
        else if (sv[0] == "d_clear")
        {
            if (sv.size() != 1)
                return STATUS::ERROR;
        }
        else
        {
            return STATUS::ERROR;
        }
        
        return STATUS::GOOD;
    }
    
    /// split the function and parameters
    STATUS DudleyParser::parse_func(std::string &s, std::vector<std::vector<std::string>> &msgs)
    {
        size_t p = s.find_first_of("(");
        if (p == std::string::npos)
            return STATUS::ERROR;
            
        s.erase(0, p + 1);
        
        p = s.find_last_of(")");
        if (p == std::string::npos)
            return STATUS::ERROR;
        
        s.erase(p, s.size());
        
        std::istringstream buffer(s);
        std::vector<std::string> elements;
    
        std::copy(std::istream_iterator<std::string>(buffer), 
                  std::istream_iterator<std::string>(),
                  std::back_inserter(elements));
                
        if (DudleyParser::check_func(elements) == STATUS::GOOD)
        {
            msgs.push_back(elements);
            return STATUS::GOOD;
        }
        
        return STATUS::ERROR;
    }
    
    /// parse lines in Dudleyfuzz file
    STATUS DudleyParser::parse(std::vector<std::vector<std::string>> &msgs)
    {
        std::string line;
        int line_num = 0;
        while (getline(*this->infile, line))
        {
            line_num++;
            DudleyParser::trim_whitespace(line);
            DudleyParser::trim_comments(line);
            if (line.empty())
                continue;
            
            // decode function
            if (DudleyParser::parse_func(line, msgs) != STATUS::GOOD)
            {
                std::string lnstr = std::to_string(line_num);
                ERRPRINT("syntax error line " + lnstr);
                return STATUS::ERROR;
            }
        }
        
        return STATUS::GOOD;
    }
    
}
