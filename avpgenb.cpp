#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include "modshared_sync.hpp"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <iostream>
#include <string>

using namespace std;
using namespace boost::interprocess;

const int poly_degree = 64;
const int q = 724481;
typedef vector<int> Poly;

int get_rand(int mod = q) { return rand() % mod; }

int small_noise() { return (rand() % 7) - 3; }

Poly rand_poly(bool large = false) {
    Poly p(poly_degree);
    for (int i = 0; i < poly_degree; ++i)
        p[i] = large ? get_rand() : small_noise();
    return p;
}

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
    if (bound <= 2) {
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

// === Main Protocol Step: Compute b_i = a * s_i + e_i ===
int main(int argc, char *argv[]) {
    if (argc != 3) {
        return 1;
    }

    int id = atoi(argv[1]);
    int n = atoi(argv[2]);
    srand(time(0) + id); // Seed for randomness

    // Step 1: Load public polynomial a
    Poly a(poly_degree);
    shared_memory_object shm_a(open_only, "shared_poly_a", read_only);
    mapped_region reg_a(shm_a, read_only);
    memcpy(a.data(), reg_a.get_address(), sizeof(int) * poly_degree);

    // Step 2: Generate s_i, e_i
    Poly s_i = rand_error_poly(q, n); // small random noise
    string s_name = "s_" + to_string(id);
    shared_memory_object::remove(s_name.c_str());
    shared_memory_object shm_s(create_only, s_name.c_str(), read_write);
    shm_s.truncate(sizeof(int) * poly_degree);
    mapped_region reg_s(shm_s, read_write);
    memcpy(reg_s.get_address(), s_i.data(), sizeof(int) * poly_degree);
    Poly e_i = rand_error_poly(q, n);

    // Step 3: Compute b_i = a * s_i + e_i
    Poly b_i = poly_add(poly_mul(a, s_i), e_i);

    // Step 4: Write b_i to shared memory as "b_i"
    string b_name = "b_" + to_string(id);
    shared_memory_object::remove(b_name.c_str());
    shared_memory_object shm_b(create_only, b_name.c_str(), read_write);
    shm_b.truncate(sizeof(int) * poly_degree);
    mapped_region reg_b(shm_b, read_write);
    memcpy(reg_b.get_address(), b_i.data(), sizeof(int) * poly_degree);

    cout << "Party " << id << " published b_i to shared memory.\n";

    return 0;
}
