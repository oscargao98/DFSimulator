//
// Created by Oscar Gao on 10/22/21.
//

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstdlib>
#include <random>
#include "device.h"

using namespace std;

int main(int argc, char*argv[])
{
    // unpack command line args
    if (argc<1)
    {
        cout << "Please follow command line guide line." << endl;
        return 0;
    }
    string filename = argv[1];
    string arg2 = argv[2];
    if (strcmp(argv[2], "-input_option") != 0)
    {
        cout << "Please follow command line guide line." << endl;
        return 0;
    }
    int input_option = atoi(argv[3]);

    int random_option = 0;
    if (strcmp(argv[4], "-true_random") == 0)
    {
        random_option = 1;
    }

    // read in the circuit file
    // get the circuit file into a c++ vector
    ifstream file(filename);
    vector <string> file_contents;
    if (file.is_open())
    {
        string line;
        while (getline(file, line))
        {
            if (!line.empty() && line.find_first_not_of(' ') != std::string::npos)
            { // check for empty line and all white space line
                file_contents.push_back(line);
            }
        }
    }
    // further, unpack into components (into 2D vector)
    int lineNum = 0; // number of valid lines
    vector< vector<string> > comp (file_contents.size());
    int PortNum = 0;
    for (int i=0; i<file_contents.size(); i++)
    {
        istringstream ss(file_contents[i]);
        string word;
        while (ss >> word)
        {
            if (word == "OUTPUT") // the word "OUTPUT" denotes the last valid line
            {
                lineNum = i;
            }
            comp[i].push_back(word);
            if (isdigit(word[0]) && stoi(word)>PortNum) PortNum = stoi(word); // get the largest port number
        }
    }
    // turn input into circuit here
    vector<string> inputs; // vector to hold all inputs ports
    vector<string> outputs; // vector to hold all output ports
    vector<device> device_list; // vector with a list of all devices (device class)
    for (int i = 0; i<= lineNum; i++)
    {
        // keep record of all input ports
        if ((comp[i][0]) == "INPUT")
        {
            int pointer1 = 1;
            while (comp[i][pointer1] != "-1")
            {
                inputs.push_back(comp[i][pointer1]);
                pointer1++;
            }
        }
        else if ((comp[i][0]) == "OUTPUT") // keep record of all output ports
        {
            int pointer2 = 1;
            while (comp[i][pointer2] != "-1")
            {
                outputs.push_back(comp[i][pointer2]);
                pointer2++;
            }
        }
        else if ((comp[i][0]) == "INV" || (comp[i][0]) == "BUF" || (comp[i][0]) == "NOR" || (comp[i][0]) == "OR" ||
        (comp[i][0]) == "AND" || (comp[i][0]) == "NAND") // keep record of circuit devices
        {
            device_list.emplace_back(device(comp[i]));
//            device_list[i].printDevice(); // print out device's information
        }
        else
        {
            throw invalid_argument("ERROR!!! INVALID LINE IN CIRCUIT FILE!!");
        }
    }
#ifdef DEBUG
    for (auto const &pair: inputs) {
        std::cout << "{" << pair.first << ": " << pair.second << "}";
    }
#endif
    size_t input_size = inputs.size();
    // if input_option is 1, ask the user to input test vector
    if (input_option == 1)
    {
        unordered_map<string, vector<string>> all_faults; // map to hold all the detected faults
        unordered_map<string, int> netValues; // map to hold values of nets
        int continue_code = 1;
        vector<int> input_clean;
        while (continue_code == 1)
        {
            string input_vector;
            cout << "Please Input Test Vector of Size " << input_size << endl;
            cin >> input_vector;
            // check if user's input vector has the correct length
            if (input_vector.length() != input_size)
            {
                cout << "Input Vector Not Correct Length." << endl;
                return 0;
            }
            // check if it is a binary vector input: only has 0 and 1
            for (auto a_char: input_vector) {
                if ((a_char != '0') && (a_char != '1')) {
                    cout << "Please Input a Binary Vector! Try Again!" << endl;
                    return 0;
                } else { // clean up the input vector to be 0 or 1
                    for (auto x: input_vector) {
                        input_clean.push_back(x - '0');
                    }
                }
            }
            // log the input vector values into the net value
            for (int i=0; i<inputs.size(); i++)
            {
                vector<string> node_fault;
                netValues[inputs[i]] = input_clean[i];
                node_fault.push_back(inputs[i]+"-s-a-"+to_string(!input_clean[i]));
                all_faults[inputs[i]] = node_fault;
            }
            vector<string> a;
            // run the simulation now
            bool finished = false;
            while(!finished)
            {
                for (auto& a_device: device_list)
                { // run if there is a valid key in map for the gates' input
                    if (a_device.done) continue; // if already ran once, skip

                    // if ran, put output value in map
                    if ((a_device.type == "BUF" || a_device.type == "INV") && netValues.find(a_device.i1) != netValues.end())
                    {
                        int output = a_device.logic(netValues[a_device.i1], 0);
                        netValues[a_device.out] = output;
                        a_device.i1v = netValues[a_device.i1];
                        a_device.ov = output;
                        a_device.fi1 = all_faults[a_device.i1];
                        vector<string> fault_output = a_device.deductive(all_faults[a_device.i1], a);
                        all_faults[a_device.out] = fault_output;
                        a_device.done = true;
                    }
                    else if (netValues.find(a_device.i1) != netValues.end() && netValues.find(a_device.i2) != netValues.end())
                    {
                        int output = a_device.logic(netValues[a_device.i1], netValues[a_device.i2]);
                        netValues[a_device.out] = output;
                        a_device.i1v = netValues[a_device.i1];
                        a_device.i2v = netValues[a_device.i2];
                        a_device.ov = output;
                        a_device.fi1 = all_faults[a_device.i1];
                        a_device.fi2 = all_faults[a_device.i2];
                        vector<string> fault_output = a_device.deductive(all_faults[a_device.i1], all_faults[a_device.i2]);
                        all_faults[a_device.out] = fault_output;
                        a_device.done = true;
                    }
                }
                // check if finished
                finished = true;
                for (auto &out_port: outputs)
                { // simulation finishes only when all ports in the output array has a valid key in map
                    if (netValues.find(out_port) == netValues.end())
                    {
                        finished = false;
                        break;
                    }
                }
            } // end simulation loop

            // print out the output
            cout << "Output Vector: ";
            for (auto &a_output: outputs) {
                cout << netValues[a_output];
            }
            cout << endl;
            // compose fault list
            vector<string> final_fault_list;
            for (auto const &a_output: outputs) {
                for (auto &a_fault: all_faults[a_output]) {
                    if (find(final_fault_list.begin(), final_fault_list.end(), a_fault) == final_fault_list.end()) {
                        final_fault_list.push_back(a_fault);
                    }
                }
            }
            // sort the final faults by port number
            unordered_map<string, string> sorting_map;
            for (auto a_fault: final_fault_list) {
                int index = 0;
                for (int i = 0; a_fault[i] != '-'; i++) {
                    index = 10 * index + (a_fault[i] - '0');
                }
                sorting_map[to_string(index)] = a_fault;
            }
            // print to console
            cout << "Detected " << sorting_map.size() << " faults out of " << 2 * PortNum << endl;
            int counter = 0;
            while (counter < PortNum) {
                if (sorting_map.find(to_string(counter)) != sorting_map.end()) {
                    cout << sorting_map[to_string(counter)] << endl;
                }
                counter++;
            }
            cout << "Fault coverage: " << (double) sorting_map.size() / (2 * (double) PortNum) * 100 << "%." << endl;

            // ask if user want to test another
            cout << "Enter 1 to Test another vector";
            cin >> continue_code;
        } // end while loop
    }
    else if (input_option == 2) // random number of 0 and 1 as input
    {
        int num_input = 0; // initialize number of simulated input
        double target_coverage = 0.0;
        vector<string> final_fault_list;
        cout << "What is the targeted coverage? (0 to 100)" << endl;
        cin >> target_coverage;
        if (target_coverage<0 || target_coverage>100)
        {
            cout << "Please enter a coverage between 0 and 100" << endl;
            return 0;
        }
        double actual_coverage = 0.0;

        while (actual_coverage < target_coverage)
        {
            vector<int> input_clean(input_size);
            // generate random input vector
            num_input ++;

            // true random prep
            std::random_device dev;
            std::mt19937 rng(dev());
            std::uniform_int_distribution<std::mt19937::result_type> dist(0,1);
            for (int i = 0; i < input_size; i++)
            {
                if (random_option) input_clean[i] = (dist(rng)); // true random
                else input_clean[i] = rand()%2; // limited random
            }
            cout << "Random Input Vector #" << num_input << ": ";
            for (auto a_input: input_clean) {
                cout << a_input;
            }
            cout << endl;
            unordered_map<string, int> netValues; // map to hold values of nets
            unordered_map<string, vector<string>> all_faults; // map to hold all the detected faults
            // log the input vector values into the net value
            for (int i=0; i<inputs.size(); i++)
            {
                vector<string> node_fault;
                netValues[inputs[i]] = input_clean[i];
                node_fault.push_back(inputs[i]+"-s-a-"+to_string(!input_clean[i]));
                all_faults[inputs[i]] = node_fault;
            }
            vector<string> a; // dummy vector
            // run the simulation now
            bool finished = false;
            while(!finished)
            {
                for (auto& a_device: device_list)
                { // run if there is a valid key in map for the gates' input
                    if (a_device.done) continue; // if already ran once, skip
                    // if ran, put output value in map
                    if ((a_device.type == "BUF" || a_device.type == "INV") && netValues.find(a_device.i1) != netValues.end())
                    {
                        int output = a_device.logic(netValues[a_device.i1], 0);
                        netValues[a_device.out] = output;
                        a_device.i1v = netValues[a_device.i1];
                        a_device.ov = output;
                        a_device.fi1 = all_faults[a_device.i1];
                        vector<string> fault_output = a_device.deductive(a_device.fi1, a);
                        all_faults[a_device.out] = fault_output;
                        a_device.done = true;
                    }
                    else if (netValues.find(a_device.i1) != netValues.end() && netValues.find(a_device.i2) != netValues.end())
                    {
                        int output = a_device.logic(netValues[a_device.i1], netValues[a_device.i2]);
                        netValues[a_device.out] = output;
                        a_device.i1v = netValues[a_device.i1];
                        a_device.i2v = netValues[a_device.i2];
                        a_device.ov = output;
                        a_device.fi1 = all_faults[a_device.i1];
                        a_device.fi2 = all_faults[a_device.i2];
                        vector<string> fault_output = a_device.deductive(a_device.fi1, a_device.fi2);
                        all_faults[a_device.out] = fault_output;
                        a_device.done = true;
                    }
                }
                // check if finished
                finished = true;
                for (auto &out_port: outputs)
                { // simulation finishes only when all ports in the output array has a valid key in map
                    if (netValues.find(out_port) == netValues.end())
                    {
                        finished = false;
                        break;
                    }
                }
            } // end simulation loop

            // clear device, ready for next round
            for (int i=0; i<device_list.size(); i++)
            {
                device_list[i].i1v = 2; // input 1
                device_list[i].i2v = 2; // input 1
                device_list[i].ov = 2; // input 1
                device_list[i].fi1.clear(); // fault list at i1
                device_list[i].fi2.clear(); // fault list at i1
                device_list[i].done = false;
            }
#ifdef DEBUG
            for (auto a_device: device_list)
            {
                a_device.printDevice();
            }
            cout << "Fault this sim: " << endl;
            for (auto const &a_output: outputs) {
                for (auto &a_fault: all_faults[a_output]) {
                    cout << a_fault << endl;
                }
            }
            cout << endl;
#endif

            // add to final_fault_list
            for (auto const &a_output: outputs) {
                for (auto &a_fault: all_faults[a_output]) {
                    if (find(final_fault_list.begin(), final_fault_list.end(), a_fault) == final_fault_list.end()) {
                        final_fault_list.push_back(a_fault);
                    }
                }
            }
            // calculate coverage
            actual_coverage = (double)final_fault_list.size()/(2 * (double)PortNum) * 100;
            cout << "Coverage after this round: " << actual_coverage << endl;
        } // end while coverage

        // output to console
        // sort the final faults by port number
        unordered_map<string, vector<string>> sorting_map;
        for (auto a_fault: final_fault_list) {
            int index = 0;
            for (int i = 0; a_fault[i] != '-'; i++) {
                index = 10 * index + (a_fault[i] - '0');
            }
            sorting_map[to_string(index)].push_back(a_fault);
        }
        // print to console
        cout << "Detected " << final_fault_list.size() << " faults out of " << 2 * PortNum << endl;
        int counter = 0;
        while (counter < PortNum) {
            if (sorting_map.find(to_string(counter)) != sorting_map.end()) {
                for (auto&a_output:sorting_map[to_string(counter)])
                {
                    cout << a_output << endl;
                }
            }
            counter++;
        }
        cout << "Final Fault Coverage: " << (double) final_fault_list.size() / (2*(double)PortNum) * 100 << "%." << endl;
        cout << "Number of Vectors: " << num_input << endl;
    }
    else
    {
        throw invalid_argument("ERROR!!! INPUT OPTION INVALID");
    }
    return 0;
}