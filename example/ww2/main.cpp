#include "disjoint_set.hpp"

#include <string>

int main(void) {
    icy::disjoint_set<std::string> _countries;
    _countries.add("chi");
    _countries.add_to("prc", "chi");
    _countries.add("jap");
    _countries.add("ger");
    _countries.add("eng");
    _countries.add_to("fra", "eng");
    _countries.add("pol");
    _countries.merge("pol", "fra");
    _countries.add_to("ita", "ger");
    _countries.add("sov");
    _countries.add("usa");
    _countries.add_to("phi", "usa");
    _countries.merge("usa", "eng");
    _countries.merge("chi", "eng");
    auto _support1 = _countries.sibling("prc", "pol");
    _countries.add_to("ita", "eng");
    auto _support2 = _countries.sibling("ita", "phi");
    return 0;
}