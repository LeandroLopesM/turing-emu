#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <stdbool.h>
#include <string>
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

struct rule {
    struct rule* next;
    int c_state;

    std::vector<struct on_state> then;
};

struct machine {
    std::string alphabet;
    std::string input_syms;
    char blank_sym;

    std::vector<int> states;
    int starter_state;
    int c_state;
    int end_state;

    std::vector<struct rule> rules;
};

enum BBStates {
    A, B, C, HALT
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