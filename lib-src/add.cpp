#include "../obj.hpp"
using namespace obj;
extern "C" void int_add(void* arg) {
    std::vector<var> *objs_ptr = (std::vector<var> *) arg;
    std::vector<var> objs = *objs_ptr;
    uint64_t size = objs.size();
    uint64_t sum = 0;
    for (uint64_t i = 0; i < size; i++) {
        sum += objs[i].to<uint64_t>();
    }
    var ret;
    ret.from<val_int, uint64_t>(sum);
    objs_ptr->push_back(ret);
}

extern "C" void int_sub(void* arg) {
    std::vector<var> *objs_ptr = (std::vector<var> *) arg;
    std::vector<var> objs = *objs_ptr;
    uint64_t size = objs.size();
    uint64_t sum = objs[0].to<uint64_t>();
    for (uint64_t i = 1; i < size; i++) {
        sum -= objs[i].to<uint64_t>();
    }
    var ret;
    ret.from<val_int, uint64_t>(sum);
    objs_ptr->push_back(ret);
}