#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <iostream>
#include <string>
#include <cmath>

using namespace std;
using namespace boost::interprocess;

const int poly_degree = 512;
const int q = 724481;
typedef vector<int> Poly;

// Read polynomial from shared memory
Poly read_poly(const string &name) {
    shared_memory_object shm(open_only, name.c_str(), read_only);
    mapped_region region(shm, read_only);
    Poly p(poly_degree);
    memcpy(p.data(), region.get_address(), sizeof(int) * poly_degree);
    return p;
}

// Polynomial arithmetic
Poly poly_add(const Poly &a, const Poly &b) {
    Poly res(poly_degree);
    for (int i = 0; i < poly_degree; ++i)
        res[i] = (a[i] + b[i]) % q;
    return res;
}

Poly poly_mul(const Poly &a, const Poly &b) {
    vector<int> res(2 * poly_degree - 1, 0);
    for (int i = 0; i < poly_degree; ++i)
        for (int j = 0; j < poly_degree; ++j)
            res[i + j] = (res[i + j] + a[i] * b[j]) % q;
    for (int i = poly_degree; i < 2 * poly_degree - 1; ++i)
        res[i - poly_degree] = (res[i - poly_degree] - res[i] + q) % q;

    Poly final(poly_degree);
    for (int i = 0; i < poly_degree; ++i)
        final[i] = res[i];
    return final;
}

// Error polynomial with controlled small noise
double compute_max_zeta(int q, int n) {
    double A = n * n;
    double B = n;
    double C = -q / 8.0;
    double disc = B * B - 4 * A * C;

    if (disc < 0) return 0.0;

    double root1 = (-B + sqrt(disc)) / (2 * A);
    double root2 = (-B - sqrt(disc)) / (2 * A);
    return std::max(root1, root2);
}

Poly rand_error_poly(int q, int n) {
    double zeta = compute_max_zeta(q, n);
    int bound = static_cast<int>(floor(zeta));
    // Enforce a minimum error range for unpredictability
    if (bound <= 1) {
        cerr << "⚠️  zeta too small (" << bound << "). Error polynomial would be too predictable.\n";
        cerr << "❌  Consider increasing q or reducing n.\n";
        exit(1);
    }
    Poly p(poly_degree);
    for (int i = 0; i < poly_degree; ++i) {
        int e = (rand() % (2 * bound + 1)) - bound;
        p[i] = e;  // signed noise in [-bound, +bound]
    }
    return p;
}

// High-magnitude noise
Poly rand_high_noise_poly() {
    Poly p(poly_degree);
    for (int i = 0; i < poly_degree; ++i) {
        int sign = (rand() % 2 == 0) ? 1 : -1;
        int mag = (q / 3) + rand() % (q / 4);
        p[i] = (sign * mag + q) % q;
    }
    return p;
}

// Main logic
int main(int argc, char *argv[]) {
    if (argc != 4) {
        return 1;
    }

    int id = atoi(argv[1]);
    int vote = atoi(argv[2]);
    int n = atoi(argv[3]);
    srand(time(0) + id);  // Ensure different randomness for each party

    // Load s_i (secret) and y_i (output from compute_y)
    Poly s_i = read_poly("s_" + to_string(id));
    Poly y_i = read_poly("y_" + to_string(id));

    Poly z_i;
    if (vote == 0) {
        Poly e = rand_error_poly(q, n);
        z_i = poly_add(poly_mul(s_i, y_i), e);
    } else {
        z_i = rand_high_noise_poly();
    }

    // Write z_i to shared memory
    string name = "z_" + to_string(id);
    shared_memory_object::remove(name.c_str());
    shared_memory_object shm(create_only, name.c_str(), read_write);
    shm.truncate(sizeof(int) * poly_degree);
    mapped_region region(shm, read_write);
    memcpy(region.get_address(), z_i.data(), sizeof(int) * poly_degree);

    cout << "Party " << id << " encoded vote and wrote z_i to shared memory.\n";
    return 0;
}
