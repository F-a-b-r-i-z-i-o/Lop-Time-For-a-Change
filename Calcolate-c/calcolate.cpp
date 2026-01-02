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
    // Compute sum of all entries of a matrix
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

    // Round to nearest integer (e.g., 68.5 -> 69)
    long double round = roundl(x);
    
    if (round <= 0)
        return 0L;

    // Clamp if it does not fit into unsigned long
    if (round >= (long double)UMAX) 
        return UMAX;

    return (unsigned long)round;
}

/*
 * Return true if: SUM_{i,j} round(c * a_ij) > ULONG_MAX
 * Early exit if overflow is detected.
 */
bool sum_round_overflows(const vector<vector<double>>& matrix, unsigned long c) 
{
    const unsigned long UMAX = numeric_limits<unsigned long>::max();
    unsigned long acc = 0UL;

    for (const auto& row : matrix) {
        for (double a : row) {
            unsigned long term = round_ldouble_to_ulong((long double)c * (long double)a);

            // If term is clamped to UMAX:
            // - if acc != 0, then acc + UMAX must overflow
            // - if acc == 0, we can set acc=UMAX
            if (term == UMAX) {
                if (acc != 0UL) 
                    return true;
                acc = UMAX;
                continue;
            }

            // Safe overflow check for acc + term
            if (acc > UMAX - term) 
                return true;

            acc += term;
        }
    }
    return false; 
}

/*
 * Find the maximum c such that sum_{i,j} round(c * a_ij) <= ULONG_MAX.
 * Exponential search (doubling) to find an upper bound where overflow happens.
 * Binary search between the last good value and the first overflowing value.
 */
unsigned long find_c(const vector<vector<double>>& matrix) 
{
    const unsigned long UMAX = numeric_limits<unsigned long>::max();

    // If it overflows already at c=1, then the maximum feasible c is 0
    if (sum_round_overflows(matrix, 1UL)) 
        return 0UL;

    unsigned long lo = 1UL;
    unsigned long hi = 2UL;

    // Exponential search for an upper bound that overflows (or reach UMAX)
    while (hi < UMAX && !sum_round_overflows(matrix, hi)) {
        lo = hi;
        if (hi > UMAX / 2) { hi = UMAX; break; } // avoid hi *= 2 overflow
        hi *= 2;
    }

    // If even c=UMAX does not overflow value is UMAX
    if (hi == UMAX && !sum_round_overflows(matrix, hi)) 
        return UMAX;

    // Binary search in (lo, hi], where lo is feasible and hi overflows
    while (lo + 1 < hi) {
        unsigned long mid = lo + (hi - lo) / 2;
        if (sum_round_overflows(matrix, mid)) 
            hi = mid;
        else lo = mid;
    }
    return lo;
}

void test_sum_round(const vector<vector<double>>& matrix, unsigned long c) 
{
    cout << "Check at c: " << (sum_round_overflows(matrix, c) ? "OVERFLOW" : "OK") << endl;
    cout << "Check at c + 1: " << (sum_round_overflows(matrix, c + 1) ? "OVERFLOW" : "OK") << endl;
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


unsigned long calcolate_global_c(fs::path &path, string name_output)
{
    fstream c_log;
    c_log.open(name_output, fstream::out);
    unsigned long global_c = numeric_limits<unsigned long>::max();
    
    for (auto &entry : fs::directory_iterator(path))
    {
        string files = entry.path();
        vector<vector<double>> instances = read_instances(files);
        vector<vector<double>> norm = normalize(instances);

        unsigned long best_c = find_c(norm);
        long double S = matrix_sum(norm);
        global_c = min(global_c, best_c);

        cout << setprecision(20);
        cout << "BEST c = " << best_c << endl;
        cout << "Sum of normalized matrix (S) = " << S << endl;
        cout << "c*S = " << best_c * S << endl;
        cout << "ULONG_MAX = " << numeric_limits<unsigned long>::max() << endl;
        cout << "sizeof(unsigned long) = " << sizeof(unsigned long) << endl;
        c_log << best_c << "\n";
        
    }
    
    c_log << "Best global c found: " << global_c;
    c_log.close();

    return global_c;
}


int main() {
    
    fs::path in_dir = "../Dataset";
    fs::path out_dir = "../pxp_it_2022_n200";
    unsigned long best_global_c = calcolate_global_c(in_dir, "calculate_global_c.txt");
    
    cout << "GLOBAL BEST c = " << best_global_c << endl;

    fs::create_directories(out_dir);

    for (const auto& entry : fs::directory_iterator(in_dir)) {
        if (!entry.is_regular_file()) continue;

        auto instances = read_instances(entry.path().string());
        auto norm = normalize(instances);

        unsigned long best_c = find_c(norm);

        fs::path out_file = out_dir / (entry.path().stem().string());
        save_round_matrix(norm, out_file.string(), best_c);
    }

    
    return 0;


  
}
