#include "main.hpp"

#include "disjoint_set.hpp"

#include <string>

int main(void) {
    icy::disjoint_set<std::string> _world {
        {"chi", "prc", "shx", "xsm", "gxc", "sik"},
        {"jap", "man", "men"},
        {"ger"}, // axis
        {"eng", "can", "raj", "ast", "nzl", "saf"}, // allies
        {"sov", "mon", "tan"}, // communist
        {"usa", "phi"},
        {"fra"},
        {"ita"},
        {"aus", "cze", "pol"}, // mid-europe countries
        {"hol", "bel", "lux"}, // low countries
        {"rom", "hun", "bul", "gre", "yog"}, // balkan peninsula
    };
    EXPECT_EQ(_world.size(), 34);
    EXPECT_EQ(_world.classification(), 11);
    /// 1938
    EXPECT_TRUE(_world.del("aus"));
    EXPECT_TRUE(_world.del("cze"));
    EXPECT_TRUE(_world.join("fra", "eng"));

    EXPECT_EQ(_world.classification(), 10);
    /// 1939
    EXPECT_TRUE(_world.join("pol", "fra"));
    EXPECT_TRUE(_world.join("ita", "ger"));

    EXPECT_EQ(_world.classification(), 8);
    /// 1940
    EXPECT_TRUE(_world.merge("hol", "eng"));
    EXPECT_TRUE(_world.join("hun", "ger"));
    EXPECT_TRUE(_world.join("rom", "ger"));
    EXPECT_TRUE(_world.join("bul", "ger"));
    EXPECT_TRUE(_world.merge("gre", "eng"));

    EXPECT_TRUE(_world.sibling("hol", "can"));
    EXPECT_TRUE(_world.contains("pol"));
    /// 1941
    EXPECT_TRUE(_world.merge("usa", "eng"));

    EXPECT_TRUE(_world.sibling("phi", "lux"));
    EXPECT_EQ(_world.size(), 32);
    EXPECT_EQ(_world.classification(), 5);
    /**
     * {"chi", "prc", "shx", "xsm", "gxc", "sik"},
     * {"jap", "man", "men"},
     * {"ger", "ita", "rom", "hun", "bul"}, // axis
     * {"eng", "can", "raj", "ast", "nzl", "saf", "fra", "pol", "hol", "bel", "lux", "gre", "yog", "usa", "phi"}, // allies
     * {"sov", "mon", "tan"}, // communist
     */
    icy::disjoint_set<std::string> _world_tno = _world;
    /// 1943 tno
    EXPECT_TRUE(_world_tno.join("mon", "jap"));
    EXPECT_TRUE(_world_tno.del_all("sov"));

    EXPECT_EQ(_world_tno.classification(), 4);
    /// 1944
    EXPECT_TRUE(_world.join("ita", "eng"));

    EXPECT_FALSE(_world.sibling("ita", "ger"));
    /// 1945
    EXPECT_TRUE(_world.del("tan"));
    EXPECT_TRUE(_world.join("rom", "sov"));
    EXPECT_TRUE(_world.join("hun", "sov"));
    EXPECT_TRUE(_world.join("bul", "sov"));
    EXPECT_TRUE(_world.join("prc"));
    EXPECT_TRUE(_world.del_except("chi"));
    EXPECT_TRUE(_world.del_except("jap"));
    EXPECT_TRUE(_world.del_except("ger"));
    EXPECT_TRUE(_world.join("pol", "sov"));
    EXPECT_FALSE(_world.join("cze", "sov"));
    EXPECT_TRUE(_world.add("cze", "sov"));
    EXPECT_TRUE(_world.join("ita", "usa"));
    EXPECT_TRUE(_world.join("yog"));

    EXPECT_TRUE(_world.sibling("pol", "mon"));
    EXPECT_FALSE(_world.contains("xsm"));
    EXPECT_EQ(_world.size(), 26);
    EXPECT_EQ(_world.classification(), 7);
    /// 1945 tno
    EXPECT_TRUE(_world_tno.join("usa"));
    EXPECT_TRUE(_world_tno.join("can", "usa"));
    EXPECT_TRUE(_world_tno.join("ast", "usa"));
    EXPECT_TRUE(_world_tno.join("nzl", "usa"));
    EXPECT_TRUE(_world_tno.join("raj"));
    EXPECT_TRUE(_world_tno.join("phi", "jap"));
    EXPECT_TRUE(_world_tno.join("eng", "ger"));
    EXPECT_TRUE(_world_tno.join("fra", "ger"));
    EXPECT_TRUE(_world_tno.join("gre", "ita"));
    EXPECT_TRUE(_world_tno.join("yog", "ita"));
    EXPECT_TRUE(_world_tno.del_all("pol"));

    EXPECT_FALSE(_world_tno.contains("pol"));
    /// 1946 tno
    EXPECT_TRUE(_world_tno.merge("chi", "jap"));
    EXPECT_TRUE(_world_tno.del("prc"));
    EXPECT_TRUE(_world_tno.join("xsm"));
    EXPECT_TRUE(_world_tno.join("sik"));

    EXPECT_EQ(_world_tno.size(), 24);
    EXPECT_EQ(_world_tno.classification(), 6);
    return 0;
}