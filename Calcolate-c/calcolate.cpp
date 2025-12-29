#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm> 
#include <limits>
#include <cmath>
using namespace std;


vector<vector<double>> normalize(vector<vector<double>> &matrix)
{
    size_t n = matrix.size();
    auto norm = matrix; // copy

    for (size_t i = 0; i < n; ++i)
        for (size_t j = 0; j < n; ++j)
            norm[i][j] = matrix[i][j] - min(matrix[i][j], matrix[j][i]);

    return norm;
}


vector<vector<double>> read_instances(const string& file_path)
{
    ifstream file(file_path);
    string str;
    bool first = true;
    vector<vector<double>> instances;

    while (getline(file, str))
    {
        if (first) { first = false; continue; }  // No header

        istringstream iss(str);
        vector<double> row;
        double x;

        while (iss >> x) {
            row.push_back(x);
        }

        if (!row.empty()) {
            instances.push_back(row); 
        }
    }

    return instances;
}

bool sum_check(const vector<vector<double>>& M, unsigned long c, unsigned long& sum_out) {
    const unsigned long UMAX = numeric_limits<unsigned long>::max();
    unsigned long sum = 0;

    for (const auto& row : M) {
        for (double x : row) {
            unsigned long add = (unsigned long) llround((long double)c * (long double)x);

            if (UMAX - sum < add) {   // overflow
                sum_out = UMAX;
                return false;
            }
            sum += add;
        }
    }
    sum_out = sum;
    return true;
}

unsigned long find_c_by_increment(const vector<vector<double>>& M) {
    unsigned long c = 10000;
    unsigned long sum = 0;

    while (true) {
        if (!sum_check(M, c, sum)) {
            return c - 1; // last valid
        }
        
        if (c % 100UL == 0) {
            cout << "c=" << c << '\n' << flush;
        }  

        ++c;
        // no infinite loop if ovwerflow 
        if (c == 0) 
            return numeric_limits<unsigned long>::max();
    
    }
}

void print_matrix(vector<vector<double>>& M)
{
    for (auto& row : M) {
        for (size_t j = 0; j < row.size(); ++j) {
            cout << row[j] << (j + 1 < row.size() ? ' ' : '\n');
        }
    }
}

int main() {
    string path = "../Dataset/sub_matrix_IT_A";
    auto instances = read_instances(path);
    auto normalize_matrix = normalize(instances);

    auto norm = normalize(instances);

    unsigned long best_c = find_c_by_increment(norm);

    unsigned long final_sum;
    sum_check(norm, best_c, final_sum);

    cout << "best c = " << best_c << "\n";
    cout << "sum = " << final_sum << "\n";


    // print_matrix(instances);

    // cout << "\n";

    // print_matrix(normalize_matrix);

    // fstream myfile;

    // myfile.open("original",fstream::out);
    // for (const auto& row : instances) {
    //     for (size_t j = 0; j < row.size(); ++j) {
    //         myfile << row[j] << (j + 1 < row.size() ? ' ' : '\n');
    //     }
    // }
    // myfile.close();
    

    // myfile.open("normalize",fstream::out);
    // for (const auto& row : normalize_matrix) {
    //     for (size_t j = 0; j < row.size(); ++j) {
    //         myfile << row[j] << (j + 1 < row.size() ? ' ' : '\n');
    //     }
    // }
    // myfile.close();
    
    return 0;
}
