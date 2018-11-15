#include <iostream>
#include <fstream>
#include <sstream>
#include <dlfcn.h>
#include <vector>
#include <memory>
#include <map>
#include <chrono>

#include "obj.hpp"

namespace vm {
    namespace op {
        class op;

        enum kind {
            push_type,
            pop_type,
            push_int_type,
            jmp_mem_type,
            skip_not_zero_type,
            skip_zero_type,
            load_type,
            call_type,
            rev_call_type,
            sym_type,
            def_type,
        };

        class op {
        public:
            kind type;
            uint64_t helper;
        };
    }

    class vm;

    class vm {
        std::vector<obj::obj> objects;
        std::vector<op::op> ops;
        std::vector<obj::obj> stack;
        std::unordered_map<std::string, obj::obj> world;
        uint64_t ptr = 0;
    public:
        void run_op();
        void run_all();
        void repr(std::istream &);
        obj::cfn load(std::string, std::string);
    };

    uint64_t nano() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()
        ).count();
    }

    // obj::fn 
    obj::cfn vm::load(std::string file, std::string name) {
        void *handle;
        std::string sum = std::string("./") + file;
        // std::cout << sum << std::endl;
        handle = dlopen(sum.c_str(), RTLD_NOW);
        if (!handle) {
            throw "cannot open";
        }
        obj::cfn fn = obj::cfn(dlsym(handle, name.c_str()));
        if (!fn) {
            throw "cannot get";
        }
        return fn;
    }

    void vm::run_op() {
        op::op cur = ops[ptr];
        switch (cur.type) {
            case op::push_type: {
                stack.push_back(objects[cur.helper]);
                break;
            }
            case op::pop_type: {
                stack.pop_back();
                break;
            }
            case op::push_int_type: {
                obj::obj val;
                val.from<obj::val_int, uint64_t>(cur.helper);
                stack.push_back(val);
                break;
            }

            case op::jmp_mem_type: {
                obj::obj back = stack.back();
                if (back.type == obj::val_int) {
                    ptr = back.to<uint64_t>();
                }
                else {
                    throw "can only jump to int";
                }
                break;
            }

            case op::skip_not_zero_type: {
                obj::obj back = stack.back();
                stack.pop_back();
                if (back.type == obj::val_int) {
                    ptr += back.to<uint64_t>() != 0;
                }
                break;
            }

            case op::skip_zero_type: {
                obj::obj back = stack.back();
                stack.pop_back();
                if (back.type == obj::val_int) {
                    ptr += back.to<uint64_t>() == 0;
                }
                break;
            }

            case op::load_type: {
                std::string ind = objects[cur.helper].to<std::string>();
                if (world.count(ind) == 0) {
                    if (ind == "$") {
                        obj::obj obj;
                        obj.from<obj::val_int, uint64_t>(ptr);
                        stack.push_back(obj);
                    }
                    else {
                        throw (std::string("no variable") + ind).c_str();
                    }
                }
                else {
                    stack.push_back(world[ind]);
                }
                break;
            }

            case op::sym_type: {
                std::string ind = objects[cur.helper].to<std::string>();
                void *sym = stack.back().to<void*>();
                obj::obj obj;
                obj.from<obj::val_cfn, obj::cfn>((obj::cfn) dlsym(sym, ind.c_str()));
                stack.push_back(obj);
                break;
            }

            case op::def_type: {
                std::string ind = objects[cur.helper].to<std::string>();
                world[ind] = stack.back();
                stack.pop_back();
                break;
            }

            case op::call_type: {
                uint64_t argc = cur.helper;
                std::vector<obj::obj> args(argc);
                for (int64_t i = argc-1; i >= 0; i--) {
                    args[i] = stack.back();
                    stack.pop_back();
                }
                obj::obj fn = stack.back();
                if (fn.type != obj::val_cfn) {
                    throw "cannot call anything but a function";
                }
                obj::cfn cfn = fn.to<obj::cfn>();
                cfn((void*) &args);
                stack.back() = args.back();
                break;
            }

            case op::rev_call_type: {
                obj::obj fn = stack.back();
                stack.pop_back();
                uint64_t argc = cur.helper;
                std::vector<obj::obj> args(argc);
                for (int64_t i = argc-1; i >= 0; i--) {
                    args[i] = stack.back();
                    stack.pop_back();
                }
                if (fn.type != obj::val_cfn) {
                    throw "cannot call anything but a function";
                }
                obj::cfn cfn = fn.to<obj::cfn>();
                cfn((void*) &args);
                stack.push_back(args.back());
                break;
            }
        }
        ptr ++;
    }

    void vm::run_all() {
        uint64_t size = ops.size();
        // std::vector<uint64_t> pls;
        // std::vector<uint64_t> count;
        // for (uint64_t i = 0; i < size; i++) {
        //     pls.push_back(0);
        //     count.push_back(0);
        // }
        // uint64_t begin = nano();
        while (ptr < size) {
            // std::cout  << ptr << std::endl;
            run_op();
            // uint64_t next = nano();
            // pls[ptr] += next - begin;
            // begin = next;
            // count[ptr] ++;
        }
        // for (uint64_t i = 0; i < size; i++) {
        //     if (count[i] > 0) {
        //         std::cout << i << ": " << pls[i] / count[i] << std::endl;
        //     }
        // }
    }

    bool is_name(char c) {
        return !(c == '\n' || c == '\t' || c == '\r' || c == ' ' || c == '\"' || c == EOF);
    }

    void vm::repr(std::istream &is) {
        struct token {
            enum {
                name_type,
                str_type,
                num_type,
                newline_type,
            } type;
            std::string str;
        };
        char got;
        got = is.get();
        std::vector<token> tokens;
        while (1) {
            if (got == EOF) {
                break;
            }
            else if (got == '\n' || got == '\r') {
                token tok;
                tok.type = token::newline_type;
                tokens.push_back(tok);
                got = is.get();
            }
            else if (got == '\t' || got == ' ') {
                got = is.get();
            }
            else if (isdigit(got) || got == '.' || got == '_') {
                std::string name;
                while ((isdigit(got)  || got == '.' || got == '_') && got != EOF) {
                    name += got;
                    got = is.get();
                }
                token tok;
                tok.type = token::num_type;
                tok.str = name;
                tokens.push_back(tok);
            }
            else if (got == '\"') {
                got = is.get();
                std::string name;
                while (got != '\"' && got != EOF) {
                    name += got;
                    got = is.get();
                }
                got = is.get();
                token tok;
                tok.type = token::str_type;
                tok.str = name;
                tokens.push_back(tok);
            }
            else if (is_name(got) && !isdigit(got)) {
                std::string name;
                while (is_name(got)) {
                    name += got;
                    got = is.get();
                }
                token tok;
                tok.type = token::name_type;
                tok.str = name;
                tokens.push_back(tok);
            }
            else {
                throw "cannot tokenize that vm code";
            }
        }
        std::vector<std::vector<token>> lines(1);
        for (token t: tokens) {
            if (t.type != token::newline_type) {
                lines.back().push_back(t);
            }
            else {
                if (lines.back().size() > 0) {
                    lines.push_back({});
                }
            }
        }
        for (std::vector<token> toks: lines) {
            if (toks.size() > 0) {
                if (toks[0].type == token::name_type) {
                    if (toks[0].str == "int") {
                        op::op op;
                        op.type = op::push_int_type;
                        try {
                            op.helper = stol(toks[1].str);
                        }
                        catch (std::invalid_argument){
                            throw (std::string("invalid integer ") + toks[1].str).c_str();
                        }
                        ops.push_back(op);
                    }

                    else if (toks[0].str == "jmp-mem") {
                        op::op op;
                        op.type = op::jmp_mem_type;
                        op.helper = 0;
                        ops.push_back(op);
                    }

                    else if (toks[0].str == "skip-not-zero") {
                        op::op op;
                        op.type = op::skip_not_zero_type;
                        op.helper = 0;
                        ops.push_back(op);
                    }

                    else if (toks[0].str == "skip-zero") {
                        op::op op;
                        op.type = op::skip_zero_type;
                        op.helper = 0;
                        ops.push_back(op);
                    }

                    else if (toks[0].str == "pop") {
                        op::op op;
                        op.type = op::pop_type;
                        op.helper = 0;
                        ops.push_back(op);
                    }

                    else if (toks[0].str == "call") {
                        op::op op;
                        op.type = op::call_type;
                        op.helper = stol(toks[1].str);
                        ops.push_back(op);
                    }

                    else if (toks[0].str == "rev-call") {
                        op::op op;
                        op.type = op::rev_call_type;
                        op.helper = stol(toks[1].str);
                        ops.push_back(op);
                    }
                    
                    else if (toks[0].str == "load") {
                        op::op op;
                        op.type = op::load_type;
                        op.helper = objects.size();
                        ops.push_back(op);
                        obj::obj obj;
                        obj.from<obj::val_string, std::string>(toks[1].str);
                        objects.push_back(obj);
                    }
                    
                    else if (toks[0].str == "def") {
                        op::op op;
                        op.type = op::def_type;
                        op.helper = objects.size();
                        ops.push_back(op);
                        obj::obj obj;
                        obj.from<obj::val_string, std::string>(toks[1].str);
                        objects.push_back(obj);
                    }
                    
                    else if (toks[0].str == "symbol") {
                        op::op op;
                        op.type = op::sym_type;
                        op.helper = objects.size();
                        ops.push_back(op);
                        obj::obj obj;
                        obj.from<obj::val_string, std::string>(toks[1].str);
                        objects.push_back(obj);
                    }
                    
                    else if (toks[0].str == "open") {
                        op::op op;
                        op.type = op::push_type;
                        op.helper = objects.size();
                        ops.push_back(op);
                        obj::obj obj;
                        obj.from<obj::val_other, void *>(dlopen(toks[1].str.c_str(), RTLD_NOW));
                        objects.push_back(obj);
                    }

                    else if (toks[0].str == "extern") {
                        obj::cfn fn = load(toks[1].str, toks[2].str);
                        obj::obj obj;
                        obj.from<obj::val_cfn, obj::cfn>(fn);
                        world[toks[2].str] = obj;
                    }
                    
                }
            }
        }
    }
    
}

int main(int argc, char** argv) {
    if (argc > 1) {
        try {
            vm::vm vm;
            std::ifstream f(argv[1]);
            vm.repr(f);
            vm.run_all();
        }
        catch (char const *err){
            std::cout << "error: "<< err << std::endl;
        }
    }
}