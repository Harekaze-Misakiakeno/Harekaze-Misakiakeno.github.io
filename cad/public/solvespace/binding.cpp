#include <emscripten/bind.h>
#include "slvs.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(module) {
    value_object<Slvs_Param>("Slvs_Param")
        .field("h", &Slvs_Param::h)
        .field("group", &Slvs_Param::group)
        .field("val", &Slvs_Param::val);

    function("Slvs_MakeParam", &Slvs_MakeParam);
}

/*
struct ProcessMessageOpts {
    bool reverse;
    bool exclaim;
    int repeat;
};

std::string processMessage(std::string message, ProcessMessageOpts opts) {
    std::string copy = std::string(message);
    if(opts.reverse) {
    std::reverse(copy.begin(), copy.end());
    }
    if(opts.exclaim) {
    copy += "!";
    }
    std::string acc = std::string("");
    for(int i = 0; i < opts.repeat; i++) {
    acc += copy;
    }
    return acc;
}

EMSCRIPTEN_BINDINGS(my_module) {
    value_object<ProcessMessageOpts>("ProcessMessageOpts")
    .field("reverse", &ProcessMessageOpts::reverse)
    .field("exclaim", &ProcessMessageOpts::exclaim)
    .field("repeat", &ProcessMessageOpts::repeat);

    function("processMessage", &processMessage);
}
*/

//em++ -o fmt_example.html ../fmt_example.cpp -I../fmt/include -L../fmt/wasm -lfmt