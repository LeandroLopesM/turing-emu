#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <stdbool.h>
#include <string>
#include <unordered_map>
#include <vector>

enum direction {
    L, R
};

struct on_state {
    char sym;
    char write;
    int next_state;
    enum direction move;
};

struct in_rule {
    std::string write;
    direction move;
    std::string nstate;
};

struct rule {
    std::unordered_map<std::string, in_rule> cases;
};

struct machine {
    std::vector<std::string> alphabet;
    std::vector<std::string> input_syms;
    std::string blank_sym;

    std::vector<std::string> states;
    std::string starter_state;
    std::string c_state;
    std::string end_state;

    std::unordered_map<std::string, rule> rules;
};

std::string slurp_file(char* path)
{
    std::ifstream f{std::string(path)};

    if (!f.good())
    {
        std::cerr << "Failed to open turing rule file '" << path << "'" << std::endl;
        exit(1);
    }

    std::ostringstream contents;
    contents << f.rdbuf();

    return contents.str();
}

std::vector<std::string> tokenize(std::string file)
{
    std::vector<std::string> out;
    std::string buffer = "";

    for ( char c : file )
    {
        if (c == ' ' || c == '\n' || c == '\t')
        {
            if (buffer != "")
            {
                out.push_back(buffer);
                buffer = "";
            }
        }
        else if (c == ';')
        {
            if (buffer != "")
            {
                out.push_back(buffer);
                buffer = "";
            }

            out.push_back(";");
        }
        else buffer += c;
    }

    if (buffer != "")
    {
        out.push_back(buffer);
        buffer = "";
    }

    out.push_back("EOF");
    return out;
}

template<typename ...A>
void error(std::string fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    vprintf(fmt.data(), va);
    printf("\n");
    va_end(va);

    exit(1);
}

template<typename T>
bool vec_find(const std::vector<T>& vec, const T& val)
{
    return std::find(
                    vec.begin(),
                    vec.end(),
                    val) != vec.end();
}

void read_list(std::vector<std::string>& out, const std::vector<std::string>& tokens, size_t& offset)
{
    printf("Looking for list at %s\n", tokens[offset].data());

    while (tokens[offset++] != ";") {
        printf("Member %s", tokens[offset].data());
        if (tokens[offset] == "EOF")
        {
            error("Expected list item, got EOF");
        }

        out.push_back(tokens[offset]);
    }

    printf ("\n");
}

rule parse_rule(machine& out, const std::vector<std::string>& s, size_t& o)
{
    rule r;

    while (s[o] != "ELUR")
    {
        if (s[o].rfind("CASE") == 0)
        {
            if (!vec_find(out.input_syms, s[++o]))
                error("%s is not a valid input symbol", s[o].data());

            auto casename = s[o];

            r.cases[s[o]] = in_rule {

            };

            do {
                if (s[o] == "WRITE")
                    r.cases[casename].write = s[++o];
                else if(s[o] == "MOVE")
                    r.cases[casename].move = s[++o] == "L"? L : R;
                else if(s[o] == "STATE")
                {
                    if (!vec_find(out.states, s[++o]))
                        error("%s is not a valid state", s[o].data());

                    r.cases[casename].nstate = s[o];
                }
                else error("Unknown token in rulelist '%s'", s[o].data());

                o++;
            } while (s[o] != ";");

printf("RULE %s:\n\
    WRITE: %s\n\
    STATE: %s\n\
    MOVE:  %d\n", r.cases[casename].write.data(), r.cases[casename].nstate.data(), r.cases[casename].move);
        }
        else error("Expected case, got %s", s[o].data());

        o++;
    }

    return r;
}

machine parse_rules(std::string file)
{
    std::vector<std::string> tokens = tokenize(file);
    machine out;

    for (size_t i = 0; i < tokens.size();)
    {
        if (tokens[i] == "ALPHABET")
        {
            read_list(out.alphabet, tokens, i);
        }
        else if(tokens[i] == "INPUTS")
        {
            read_list(out.input_syms, tokens, i);
            --i;
        }
        else if(tokens[i] == "BLANK")
        {
            out.blank_sym = tokens[++i];
        }
        else if(tokens[i] == "STATES")
        {
            read_list(out.states, tokens, i);
        }
        else if(tokens[i] == "STARTS")
        {
            out.starter_state = tokens[++i];
        }
        else if(tokens[i].rfind("RULE") == 0)
        {
            auto statename = tokens[++i];
            if (vec_find(out.states, statename))
            {
                out.rules[statename] = parse_rule(out, tokens, ++i);
            }
            else error("No such state %s", statename.data());
        }
        else if(tokens[i] == "BLANK")
        {
            out.blank_sym = tokens[++i];
        }
        else error("Unknown token %s", tokens[i].data());
    }

    return out;
}

int main(int argc, char** argv)
{
    if (argc == 1)
    {
        std::cerr << "Usage: tur.exe [rules.tur]" << std::endl;
        return 1;
    }

    std::string contents = slurp_file(argv[1]);
    machine m = parse_rules(contents);
}