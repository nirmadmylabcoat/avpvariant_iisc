#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <vector>
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

Poly poly_add(const Poly &a, const Poly &b) {
    Poly res(poly_degree);
    for (int i = 0; i < poly_degree; ++i)
        res[i] = (a[i] + b[i]) % q;
    return res;
}

Poly poly_sub(const Poly &a, const Poly &b) {
    Poly res(poly_degree);
    for (int i = 0; i < poly_degree; ++i)
        res[i] = (a[i] - b[i] + q) % q;
    return res;
}

Poly read_poly_from_shared(const string &name) {
    shared_memory_object shm(open_only, name.c_str(), read_only);
    mapped_region reg(shm, read_only);
    Poly p(poly_degree);
    memcpy(p.data(), reg.get_address(), sizeof(int) * poly_degree);
    return p;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Usage: ./compute_y <id> <n>\n";
        return 1;
    }

    int id = atoi(argv[1]);
    int n = atoi(argv[2]);

    Poly y_i(poly_degree, 0);

    for (int j = 0; j < n; ++j) {
        if (j == id) continue;

        string b_name = "b_" + to_string(j);
        Poly b_j = read_poly_from_shared(b_name);

        if (j < id)
            y_i = poly_add(y_i, b_j);
        else
            y_i = poly_sub(y_i, b_j);
    }

    // Write y_i to shared memory
    string y_name = "y_" + to_string(id);
    shared_memory_object::remove(y_name.c_str());
    shared_memory_object shm(create_only, y_name.c_str(), read_write);
    shm.truncate(sizeof(int) * poly_degree);
    mapped_region region(shm, read_write);
    memcpy(region.get_address(), y_i.data(), sizeof(int) * poly_degree);

    cout << "Party " << id << " computed and wrote y_i to shared memory.\n";
    return 0;
}
