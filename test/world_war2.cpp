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
    EXPECT_EQ(_world.sibling("usa"), 15);
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
    EXPECT_TRUE(_world_tno.del("men"));
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
    /**
     * {"chi"}, {"prc"}, {"yog"}, {"jap"}, {"ger"}
     * {"eng", "can", "raj", "ast", "nzl", "saf", "fra", "hol", "bel", "lux", "gre", "usa", "phi", "ita"}
     * {"sov", "mon", "rom", "hun", "bul", "cze", "pol"}
     */
    EXPECT_TRUE(_world.sibling("pol", "mon"));
    EXPECT_FALSE(_world.contains("xsm"));
    EXPECT_EQ(_world.size(), 26);
    EXPECT_EQ(_world.classification(), 7);
    EXPECT_EQ(_world.sibling("sov"), 7);
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
    /**
     * {"xsm"}, {"sik"}, {"raj"}
     * {"chi, "gxc", "shx", "jap", "man", "mon", "phi"}
     * {"usa", "can", "ast", "nzl"}
     * {"ger", "ita", "eng", "fra", "gre", "yog", "hun", "rom", "bul"}
     */
    EXPECT_EQ(_world_tno.size(), 23);
    EXPECT_EQ(_world_tno.classification(), 6);
    EXPECT_EQ(_world_tno.sibling("sov"), 0);
    EXPECT_EQ(_world_tno.sibling("raj"), 1);
    EXPECT_NQ(_world, _world_tno);
    /// otl -> tno
    /**
     * del ["hol", "bel", "lux", "saf", "cze", "pol", "sov", "prc"]
     * add ["man", "xsm", "sik", "gxc", "shx"]
     */
    EXPECT_TRUE(_world.join("usa"));
    EXPECT_TRUE(_world.join("eng"));
    EXPECT_TRUE(_world.join("fra", "eng"));
    EXPECT_TRUE(_world.merge("eng", "ger"));
    EXPECT_TRUE(_world.join("can", "usa"));
    EXPECT_TRUE(_world.join("raj"));
    EXPECT_TRUE(_world.join("ast"));
    EXPECT_TRUE(_world.join("nzl", "ast"));
    EXPECT_TRUE(_world.merge("nzl", "can"));
    EXPECT_TRUE(_world.join("ita"));
    EXPECT_TRUE(_world.join("gre", "ita"));
    EXPECT_TRUE(_world.merge("gre", "fra"));
    EXPECT_TRUE(_world.del_except("phi"));
    EXPECT_TRUE(_world.join("phi", "jap"));
    EXPECT_TRUE(_world.join("mon", "jap"));
    EXPECT_TRUE(_world.join("rom"));
    EXPECT_TRUE(_world.join("hun", "rom"));
    EXPECT_TRUE(_world.join("bul", "rom"));
    EXPECT_TRUE(_world.merge("ger", "rom"));
    EXPECT_TRUE(_world.del_all("sov"));
    EXPECT_TRUE(_world.join("yog", "ita"));
    EXPECT_TRUE(_world.add("gxc", "chi"));
    EXPECT_TRUE(_world.add("shx", "chi"));
    EXPECT_TRUE(_world.merge("jap", "chi"));
    EXPECT_TRUE(_world.add("man", "jap"));
    EXPECT_TRUE(_world.add("xsm"));
    EXPECT_TRUE(_world.add("sik"));
    EXPECT_TRUE(_world.del("prc"));

    EXPECT_EQ(_world.size(), _world_tno.size());
    EXPECT_EQ(_world.classification(), _world_tno.classification());
    EXPECT_EQ(_world, _world_tno);
    return 0;
}