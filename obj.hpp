#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <map>
#include <boost/variant.hpp>

#define SHARED

namespace obj {
    enum kind {
        val_other,
        val_int,
        val_double,
        val_string,
        val_table,
        val_bool,
        val_cfn
    };
    // static const uint64_t val_other = 1; 
    // static const uint64_t val_int = 1; 
    // static const uint64_t val_double = 2; 
    // static const uint64_t val_string = 3; 
    // static const uint64_t val_table = 4; 
    // static const uint64_t val_bool = 5; 
    // static const uint64_t val_cfn = 6; 

    class obj;
    class table;
    class obj {
        #ifdef SHARED
        std::shared_ptr<void> ptr;
        #else
        void* ptr;
        #endif
    public:
        kind type;
        template<kind i, typename T>
        void from(T);
        template<typename T>
        T to();
    };

    class table {
        std::vector<obj> keys;
        std::vector<obj> vals;
        void set(obj, obj);
        void get(obj);
    };

    using cfn = void(*)(void*);

    template<kind i, typename T>
    void obj::from(T val) {
        type = i;
        #ifdef SHARED
        if constexpr(i == val_int) {
            ptr = std::static_pointer_cast<void>(std::make_shared<uint64_t>(val));
        }
        if constexpr(i == val_double) {
            ptr = std::static_pointer_cast<void>(std::make_shared<double>(val));
        }
        if constexpr(i == val_string) {
            ptr = std::static_pointer_cast<void>(std::make_shared<std::string>(val));
        }
        if constexpr(i == val_table) {
            ptr = std::static_pointer_cast<void>(std::make_shared<table>(val));
        }
        if constexpr(i == val_bool) {
            ptr = std::static_pointer_cast<void>(std::make_shared<bool>(val));
        }
        if constexpr(i == val_bool) {
            ptr = std::static_pointer_cast<void>(std::make_shared<bool>(val));
        }
        if constexpr(i == val_cfn) {
            ptr = std::static_pointer_cast<void>(std::make_shared<cfn>(val));
        }
        if constexpr(i == val_other) {
            ptr = std::static_pointer_cast<void>(std::make_shared<T>(val));
        }
        #else
        ptr = (void*) new T(val);
        #endif
    }

    template<typename T>
    T obj::to() {
        #ifdef SHARED
        return *std::static_pointer_cast<T>(ptr);
        #else
        return *((T*) ptr);
        #endif
    }
    using fn = obj(*)(std::vector<obj>);
    using var = obj;
}