#include <boost/interprocess/shared_memory_object.hpp>
#include <string>

using namespace boost::interprocess;

int main() {
    shared_memory_object::remove("SharedSync");
    shared_memory_object::remove("shared_poly_a");

    for (int i = 0; i < 100; ++i) {
        shared_memory_object::remove(("s_" + std::to_string(i)).c_str());
        shared_memory_object::remove(("b_" + std::to_string(i)).c_str());
        shared_memory_object::remove(("y_" + std::to_string(i)).c_str());
        shared_memory_object::remove(("z_" + std::to_string(i)).c_str());
    }
    return 0;
}
