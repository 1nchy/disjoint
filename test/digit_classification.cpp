#include "main.hpp"

#include "disjoint_map.hpp"

#include <string>

int main(void) {
    icy::disjoint_map<unsigned, std::string> _digit {
        {{2u, "two"}, {4u, "four"}, {6u, "six"}, {8u, "eight"}},
        {{1u, "first"}, {3u, "three"}, {5u, "five"}, {7u, "seven"}}
    };
    EXPECT_EQ(_digit.size(), 8);
    EXPECT_EQ(_digit.classification(), 2);
    // odd & even
    EXPECT_TRUE(_digit.add({0u, "ZERO"}, 2u));
    _digit[0u] = "zero";
    _digit[9u] = "nine";
    EXPECT_TRUE(_digit.merge(1u, 9u));
    EXPECT_TRUE(_digit.update(1u, "one"));

    EXPECT_EQ(_digit.size(), 10);
    EXPECT_EQ(_digit.classification(), 2);
    EXPECT_EQ(_digit.at(0u), "zero");
    EXPECT_EQ(_digit.at(1u), "one");
    EXPECT_EQ(_digit.sibling(3u), 5);
    /**
     * {"zero", "two", "four", "six", "eight"}
     * {"one", "three", "five", "seven", "nine"}
     */
    // prime
    icy::disjoint_map<unsigned, std::string> _prime = _digit;
    EXPECT_TRUE(_prime.join(0u));
    EXPECT_TRUE(_prime.join(1u));
    EXPECT_TRUE(_prime.join(2u, 3u));
    EXPECT_TRUE(_prime.join(9u, 4u));
    /**
     * {0}, {1}
     * {2, 3, 5, 7}
     * {4, 6, 8, 9}
     */
    EXPECT_EQ(_prime.size(), 10);
    EXPECT_EQ(_prime.classification(), 4);
    EXPECT_TRUE(_prime.sibling(2u, 7u));
    EXPECT_EQ(_prime.sibling(3u), 4);
    EXPECT_NQ(_prime, _digit);
    EXPECT_EQ(_prime[1], _digit[1]);
    // length
    icy::disjoint_map<unsigned, std::string> _length = _digit;
    EXPECT_TRUE(_length.join(2u));
    EXPECT_TRUE(_length.join(6u, 2u));
    EXPECT_TRUE(_length.join(1u, 2u));
    EXPECT_TRUE(_length.join(9u, 0u));
    EXPECT_TRUE(_length.join(5u, 4u));
    EXPECT_TRUE(_length.join(8u, 3u));
    /**
     * {1, 2, 6}
     * {0, 4, 5, 9}
     * {3, 7, 8}
     */
    EXPECT_EQ(_length.sibling(1u), 3);
    EXPECT_EQ(_length.sibling(0u), 4);
    EXPECT_EQ(_length.sibling(3u), 3);
    // _prime -> _length
    EXPECT_TRUE(_prime.join(2u, 1u));
    EXPECT_TRUE(_prime.join(0u, 4u));
    EXPECT_TRUE(_prime.join(5u, 9u));
    EXPECT_TRUE(_prime.join(6u, 2u));
    EXPECT_TRUE(_prime.join(8u, 3u));
    EXPECT_EQ(_length, _prime);
    return 0;
}