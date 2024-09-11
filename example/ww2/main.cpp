#include "disjoint_set.hpp"

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
    _countries.add_to("ita", "ger");
    _countries.add_to("phi", "usa");
    _countries.merge("usa", "eng");
    _countries.merge("chi", "eng");
    auto _support1 = _countries.sibling("prc", "pol");
    _countries.add_to("ita", "eng");
    auto _support2 = _countries.sibling("ita", "phi");
    return 0;
}