#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <limits>
#include <cmath>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;
using namespace std;

/*
    * Normalize matrix with: a_ij* = a_ij - min(a_ij, a_ji)
*/
vector<vector<double>> normalize(const vector<vector<double>> &matrix)
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
    // Read a whitespace-separated matrix from file, skipping the header
    ifstream file(file_path);
    string str;
    bool first = true;
    vector<vector<double>> instances;

    while (getline(file, str))
    {
        if (first) { first = false; continue; }  // skip header

        istringstream iss(str);
        vector<double> row;
        double x;

        while (iss >> x) row.push_back(x);
        if (!row.empty()) instances.push_back(row);
    }

    return instances;
}

long double matrix_sum(const vector<vector<double>>& matrix)
{
    long double sum = 0.0L;
    for (const auto& row : matrix)
        for (double x : row)
            sum += (long double)x;
    return sum;
}

// Round and clamp to ULONG_MAX; for x<=0 returns 0
unsigned long round_ldouble_to_ulong(long double x)
{
    const unsigned long UMAX = numeric_limits<unsigned long>::max();

    long double r = roundl(x);

    if (r <= 0)
        return 0UL;

    if (r >= (long double)UMAX)
        return UMAX;

    return (unsigned long)r;
}

bool overflows_on_normalized(const vector<vector<double>>& norm, unsigned long &c) {
    unsigned long sum = 0UL;

    for (const auto& row : norm) {
        for (double a : row) {
            unsigned long val = (unsigned long) llround((long double)c * (long double)a);

            unsigned long old_sum = sum;
            sum += val;

            // overflow wrap-around su unsigned long
            if (sum < old_sum) 
                return true;
        }
    }
    return false;
}

void save_round_matrix(vector<vector<double>> &norm, string filename, unsigned long &best_global_c)
{
    fstream myfile;
    myfile.open(filename,fstream::out);
    myfile << norm.size() << endl;
    for (auto &row : norm)
    {
        for (size_t j = 0; j < row.size(); ++j) {
            unsigned long val = round_ldouble_to_ulong((long double)row[j] * (long double)best_global_c);
            myfile << val << (j+1 < row.size() ? ' ' : '\n');
        }
    }
    myfile.close();
}


int main()
{
    fs::path in_dir = "find_c_dataset";
    const unsigned long UMAX = numeric_limits<unsigned long>::max();

    cout << setprecision(20);
    cout << "ULONG_MAX = " << UMAX << endl;
    cout << "sizeof(unsigned long) = " << sizeof(unsigned long) << "\n\n";

   
    ofstream log("best_c_per_file.txt");

    for (auto& entry : fs::directory_iterator(in_dir))
    {
        string fname = entry.path().filename().string();
        cout << "=============================" << endl;
        cout << "File: " << fname << endl;

        auto M = read_instances(entry.path().string());
        auto norm = normalize(M);

        long double S = matrix_sum(norm);

        unsigned long best_ok = 0UL;
        unsigned long c = 1000UL; // mille

        while (true)
        {
            long double cS = (long double)c * S;

            cout << "  Testing c = " << c
                 << " | S = " << S
                 << " | c*S = " << cS
                 << " | ULONG_MAX = " << UMAX
                 << endl;

            if (overflows_on_normalized(norm, c)) {
                cout << " -> OVERFLOW at c = " << c
                     << " ; chosen (previous) = " << best_ok << endl;
                break;
            }

            best_ok = c;

            if (c > UMAX / 10UL) {
                cout << " -> reached max representable c; chosen = " << best_ok << endl;
                break;
            }
            c *= 10UL;
        }

        log << fname << " " << best_ok << endl;
    }

    cout << "\nSaved best c for each file into: best_c_per_file.txt\n";
    return 0;
}
