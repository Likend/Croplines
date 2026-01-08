#include "utils/Compare.hpp"

#include <cctype>
#include <compare>
#include <iterator>
#include <string_view>

std::strong_ordering croplines::NaturalCompare(std::string_view a, std::string_view b) {
    for (const char *i1 = a.begin(), *i2 = b.begin(); i1 != a.end() && i2 != b.end(); ++i1, ++i2) {
        if (std::isdigit(*i1) && std::isdigit(*i2)) {
            const char *ii1 = i1, *ii2 = i2;
            while (ii1 != a.end() && *ii1 == '0') ++ii1;
            while (ii2 != b.end() && *ii2 == '0') ++ii2;
            auto zero_count1 = std::distance(i1, ii1);
            auto zero_count2 = std::distance(i2, ii2);

            i1 = ii1;
            i2 = ii2;
            while (ii1 != a.end() && std::isdigit(*ii1)) ++ii1;
            while (ii2 != b.end() && std::isdigit(*ii2)) ++ii2;
            auto num1 = std::string_view(i1, ii1 - i1);
            auto num2 = std::string_view(i2, ii2 - i2);

            if (auto cmp = num1.length() <=> num2.length(); cmp != 0) return cmp;
            if (auto cmp = num1 <=> num2; cmp != 0) return cmp;
            if (auto cmp = zero_count1 <=> zero_count2; cmp != 0) return cmp;
        } else {
            return std::toupper(*i1) <=> std::toupper(*i2);
        }
    }
    return std::strong_ordering::equal;
}
