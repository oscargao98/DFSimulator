//
// Created by Oscar Gao on 10/22/21.
//

#ifndef P16140_DEVICE_H
#define P16140_DEVICE_H
#pragma once
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <iterator> // for std::back_inserter

using namespace std;

// define a class of devices
class device {
public:
    explicit device(const vector<string>& comp);
    void printDevice() const; // print information of device
    int logic (int input1, int input2) const; // logic of the gate
    vector<string> deductive(const vector<string>& input1, const vector<string>& input2) const;
    static vector<string> fault_union(vector<string> first_v, vector<string> second_v);
    static vector<string> fault_difference(vector<string> first_v, vector<string> second_v);
    static vector<string> fault_intersection(vector<string> first_v, vector<string> second_v);

    string type; // type of gate
    string i1; // input 1
    string i2; // input 2, set to "" for 1 input gates
    string out; // output

    int i1v = 2; // input 1
    int i2v = 2; // input 2
    int ov = 2; // output

    vector<string> fi1; // fault list at i1
    vector<string> fi2; // fault list at i2

    bool done = false;
};

#endif //P16140_DEVICE_H
