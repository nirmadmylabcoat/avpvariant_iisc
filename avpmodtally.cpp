#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <vector>
#include <cstring>
#include <iostream>
#include <cmath>

using namespace std;
using namespace boost::interprocess;

const int poly_degree = 64;
const int q = 724481;
typedef vector<int> Poly;

// Read a polynomial from shared memory
Poly read_poly(const string &name) {
    shared_memory_object shm(open_only, name.c_str(), read_only);
    mapped_region region(shm, read_only);
    Poly p(poly_degree);
    memcpy(p.data(), region.get_address(), sizeof(int) * poly_degree);
    return p;
}

// Add two polynomials modulo q
Poly poly_add(const Poly &a, const Poly &b) {
    Poly res(poly_degree);
    for (int i = 0; i < poly_degree; ++i)
        res[i] = (a[i] + b[i]) % q;
    return res;
}

// Compute ∞-norm (max absolute value from centered range)
int inf_norm(const Poly &p) {
    int max_norm = 0;
    for (int coeff : p) {
        int centered = coeff > q/2 ? coeff - q : coeff;
        max_norm = max(max_norm, abs(centered));
        //cout << max_norm << "vs." << centered << endl;
    }
    return max_norm;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        return 1;
    }

    int n = atoi(argv[1]);

    // Sum up all z_i polynomials
    Poly z_total(poly_degree, 0);
    for (int i = 0; i < n; ++i) {
        Poly z_i = read_poly("z_" + to_string(i));
        z_total = poly_add(z_total, z_i);
    }

    int norm = inf_norm(z_total);
    int threshold = (q / 4) - 2;

    cout << "Infinity norm: " << norm << "\n";
    cout << "Threshold: " << threshold << "\n";

    if (norm <= threshold) {
        cout << "Final vote tally: 0 (likely all parties voted 0)\n";
    } else {
        cout << "Final vote tally: 1 (at least one party voted 1)\n";
    }

    return 0;
}
