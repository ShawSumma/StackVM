#include "../obj.hpp"
using namespace obj;

extern "C" void newline(void* arg) {
    std::vector<var> *objs_ptr = (std::vector<var> *) arg;
    std::vector<var> objs = *objs_ptr;
    std::cout << std::endl;
    var ret;
    ret.from<val_bool, bool>(true);
    objs_ptr->push_back(ret);
}

extern "C" void print(void* arg) {
    std::vector<var> *objs_ptr = (std::vector<var> *) arg;
    std::vector<var> objs = *objs_ptr;
    uint64_t size = objs.size();
    for (uint64_t i = 0; i < size; i++) {
        if (i != 0) {
            std::cout << " " << std::endl;
        }
        var v = objs[i];
        switch (v.type) {
            case val_bool: {
                std::cout << (v.to<bool>() ? "true" : "false");
                break;
            }
            case val_int: {
                std::cout << v.to<uint64_t>();
                break;
            }
            case val_double: {
                std::cout << v.to<double>();
                break;
            }
            case val_string: {
                std::cout << v.to<std::string>();
                break;
            }
            case val_table: {
                std::cout << "<table>";
                break;
            }
            case val_cfn: {
                std::cout << "<func>";
                break;
            }
            case val_other: {
                std::cout << "<other>";
                break;
            }
        }
    }
    std::cout << std::endl;
    var ret;
    ret.from<val_bool, bool>(true);
    objs_ptr->push_back(ret);
}