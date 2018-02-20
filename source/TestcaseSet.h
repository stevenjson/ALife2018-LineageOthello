#ifndef TEST_CASE_SET_H
#define TEST_CASE_SET_H

// This code was pulled and modified from: https://github.com/emilydolson/ecology_of_evolutionary_computation/blob/master/source/TestcaseSet.h
// - Original version by Emily Dolson

#include <iostream>
#include <fstream>
#include <set>
#include <algorithm>
#include <functional>

#include "base/array.h"
#include "base/vector.h"
#include "tools/string_utils.h"
#include "tools/Random.h"
#include "tools/random_utils.h"


template <typename INPUT_TYPE, typename OUTPUT_TYPE>
class TestcaseSet {
protected:
    using input_t = emp::vector<INPUT_TYPE>;
    using output_t = OUTPUT_TYPE;
    using test_case_t = std::pair<input_t, output_t>;
    using test_case_reader_func_t = std::function<test_case_t(emp::vector<std::string>, std::string)>;
    emp::vector<test_case_t> test_cases;
    test_case_reader_func_t input_test_case;

public:
    TestcaseSet(const test_case_reader_func_t & reader, std::string filename) {
        RegisterTestcaseReader(reader);
        LoadTestcases(filename);
    }

    TestcaseSet() {}

    emp::vector<std::pair<input_t, output_t> >& GetTestcases() {
        return test_cases;
    }

    emp::vector<size_t> GetSubset(int trials, emp::Random * random) {
        return emp::Choose(*random, test_cases.size(), trials);
    }

    // Probably want a better name for this...
    void RegisterTestcaseReader(const test_case_reader_func_t & reader) { input_test_case = reader; }

    void LoadTestcases(std::string filename, bool contains_output = true) {
        std::ifstream infile(filename);
        std::string line;

        if (!infile.is_open()){
            std::cout << "ERROR: " << filename << " did not open correctly" << std::endl;
            return;
        }

        // Ignore header
        getline(infile, line);

        while ( getline (infile,line)) {
            emp::vector<std::string> split_line = emp::slice(line, ',');
            emp::vector<std::string> test_case_input;
            std::string test_case_answer;
            for (size_t i = 0; i < split_line.size() - int(contains_output); i++) {
              test_case_input.push_back(split_line[i].c_str());
            }
            if (contains_output) {
              test_case_answer = split_line[split_line.size()-1].c_str();
            }
            test_cases.push_back(input_test_case(test_case_input, test_case_answer));
            // std::cout << emp::to_string(test_case) << " " << answer << std::endl;
        }
        infile.close();
    }

};

#endif
