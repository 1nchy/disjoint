#include "disjoint.hpp"

#include <string>

int main(void) {
    icy::disjoint_set<std::string> _countries {
        {"chi", "prc"},
        {"jap"},
        {"ger"},
        {"eng", "fra", "pol"},
        {"sov"},
        {"usa"}
    };
    _countries.add("ita", "ger");
    _countries.add("phi", "usa");
    _countries.merge("usa", "eng");
    _countries.merge("chi", "eng");
    auto _support1 = _countries.sibling("prc", "pol");
    _countries.add("ita", "eng");
    auto _support2 = _countries.sibling("ita", "phi");
    return 0;
}