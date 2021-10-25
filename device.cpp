//
// Created by Oscar Gao on 10/22/21.
//

#include "device.h"

// device constructor
device::device(const vector<string>& comp)
{
    this->type = comp[0];
    if (comp.size() == 3) // INV and BUF
    {
        this->i1 = comp[1];
        this->i2 = "";
        this->out = comp[2];
    }
    else if (comp.size() == 4) // all other gates
    {
        this->i1 = comp[1];
        this->i2 = comp[2];
        this->out = comp[3];
    }
    else
    {
        throw invalid_argument("ERROR INITIALIZING DEVICE!!");
    }
}

void device::printDevice() const
{
    cout << "type: " << this->type << ", ";
    cout << "i1: " << this->i1 << ", ";
    cout << "i2: " << this->i2 << ", ";
    cout << "out: " << this->out << endl;
    cout << "i1v: " << this->i1v << ", ";
    cout << "i2v: " << this->i2v << ", ";
    cout << "ov: " << this->ov << endl;
}

int device:: logic(int input1, int input2 = 0) const
{
    int output;
    if (this->type == "INV")
    {
        output = !input1;
    }
    else if (this->type == "BUF")
    {
        output = input1;
    }
    else if (this->type == "AND")
    {
        output = input1&input2;
    }
    else if (this->type == "NAND")
    {
        output = !(input1&input2);
    }
    else if (this->type == "OR")
    {
        output = input1|input2;
    }
    else if (this->type == "NOR")
    {
        output = !(input1|input2);
    }
    else
    {
        throw invalid_argument("ERROR!! INVALID TYPE RAISE ERROR WHEN COMPUTING LOGIC!!");
    }
    return output;
}


vector<string> device:: deductive(const vector<string>& input1, const vector<string>& input2) const
{
    vector<string> output_node;
    output_node.push_back(this->out+"-s-a-"+to_string(!this->ov));
    if (this->type == "INV" | this->type == "BUF")
    {
        return fault_union(output_node, input1);
    }
    else if (this->type == "AND" | this->type == "NAND")
    {
        if (this->i1v == 0 && this->i2v == 0)
        {
            return fault_union(fault_intersection(this->fi1, this->fi2), output_node);
        }
        else if (this->i1v == 0 && this->i2v == 1)
        {
            return fault_union(fault_difference(this->fi1, this->fi2), output_node);
        }
        else if (this->i1v == 1 && this->i2v == 0)
        {
            return fault_union(fault_difference(this->fi2, this->fi1), output_node);
        }
        else if (this->i1v == 1 && this->i2v == 1)
        {
            return fault_union(fault_union(this->fi2, this->fi1), output_node);
        }
        else
        {
            throw invalid_argument("ERROR List Processing! AND");
        }
    }
    else if (this->type == "OR" | this->type == "NOR")
    {
        if (this->i1v == 0 && this->i2v == 0)
        {
            return fault_union(fault_union(this->fi1, this->fi2), output_node);
        }
        else if (this->i1v == 0 && this->i2v == 1)
        {
            return fault_union(fault_difference(this->fi2, this->fi1), output_node);
        }
        else if (this->i1v == 1 && this->i2v == 0)
        {
            return fault_union(fault_difference(this->fi1, this->fi2), output_node);
        }
        else if (this->i1v == 1 && this->i2v == 1)
        {
            return fault_union(fault_intersection(this->fi2, this->fi1), output_node);
        }
        else
        {
            throw invalid_argument("ERROR List Processing! OR");
        }
    }
    else
    {
        throw invalid_argument("ERROR List Processing!");
    }
}

vector<string> device::fault_union(vector<string> first_v, vector<string> second_v)
{
    vector<string> output;
    std::set_union(first_v.begin(), first_v.end(), second_v.begin(), second_v.end(), back_inserter(output));
    return output;
}

vector<string> device::fault_difference(vector<string> first_v, vector<string> second_v)
{
    vector<string> output;
    std::set_difference(first_v.begin(), first_v.end(), second_v.begin(), second_v.end(), back_inserter(output));
    return output;
}

vector<string> device::fault_intersection(vector<string> first_v, vector<string> second_v)
{
    vector<string> output;
    std::set_intersection(first_v.begin(), first_v.end(), second_v.begin(), second_v.end(), back_inserter(output));
    return output;
}

void device::clear_device()
{
    this->i1v = 2;
    this->i2v = 2;
    this->ov = 2;
    this->fi1.clear();
    this->fi2.clear();
    this->done = false;
}

