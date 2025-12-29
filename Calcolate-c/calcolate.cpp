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

long double matrix_sum(vector<vector<double>>& M) {
    long double sum = 0.0L;
    for (const auto& row : M)
        for (double x : row)
            sum += (long double) x;
    return sum;
}

unsigned long find_c(vector<vector<double>>& M) {
    const unsigned long UMAX = numeric_limits<unsigned long>::max();
    const long double S = matrix_sum(M);

    unsigned long c = 100;
    while (true) {
        long double v = (long double)c * S;
        long double r_ld = llround(v); 

        if (c % 100UL == 0) {
            cout << "c=" << c << "  round(c*S)=" << (unsigned long long)r_ld << "\n";
        }

        // if round value over UMAX take last value
        if (r_ld > (long double)UMAX) {
            return (c == 0 ? 0 : c - 1);
        }

        ++c;
        if (c == 0) return UMAX; 
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
    
    unsigned long best_c = find_c(norm);

    long double S = matrix_sum(norm);
    unsigned long rounded_final = (unsigned long) llround((long double)best_c * S);

    cout << "BEST c = " << best_c << "\n";
    cout << "round(BEST c * S) = " << rounded_final << "\n";
    cout << "ULONG_MAX = " << numeric_limits<unsigned long>::max() << "\n";



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
