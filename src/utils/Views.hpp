#include <concepts>
#include <cstddef>
#include <iterator>
#include <ranges>

namespace croplines {
// Alternative to C++26 std::ranges::concat_view
template <std::ranges::view View1, std::ranges::view View2>
    requires std::same_as<std::ranges::range_value_t<View1>, std::ranges::range_value_t<View2>>
// std::same_as<
//     typename std::iterator_traits<decltype(std::ranges::begin(view1))>::value_type,
//     typename std::iterator_traits<decltype(std::ranges::begin(view2))>::value_type>;
// std::same_as<typename
// std::iterator_traits<decltype(std::ranges::begin(view1))>::pointer,
//              typename
//              std::iterator_traits<decltype(std::ranges::begin(view2))>::pointer>;
// std::same_as<typename
// std::iterator_traits<decltype(std::ranges::begin(view1))>::reference,
//              typename
//              std::iterator_traits<decltype(std::ranges::begin(view2))>::reference>;
class ConcatView : public std::ranges::view_base {
    View1 view1;
    View2 view2;

   public:
    ConcatView(View1 view1, View2 view2) : view1(view1), view2(view2) {}

    class It {
        using iterator1 = decltype(std::ranges::begin(view1));
        using iterator2 = decltype(std::ranges::begin(view2));

       public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = typename iterator1::value_type;
        using pointer           = typename iterator1::pointer;
        using reference         = typename iterator1::reference;

        It() : parent(nullptr), it1{}, it2{} {}

        It(ConcatView& parent)
            : parent(&parent),
              it1(std::ranges::begin(parent.view1)),
              it2(std::ranges::begin(parent.view2)) {}

        It(ConcatView& parent, iterator1 it1, iterator2 it2)
            : parent(&parent), it1(it1), it2(it2) {}

        reference operator*() const {
            if (is_first())
                return *it1;
            else
                return *it2;
        }
        pointer operator->() {
            if (is_first())
                return it1.operator->();
            else
                return it2.operator->();
        }
        It& operator++() {
            if (is_first())
                it1++;
            else
                it2++;
            return *this;
        }
        It operator++(int) {
            It tmp = *this;
            ++(*this);
            return tmp;
        }

        friend bool operator==(const It& a, const It& b) {
            if (a.is_first() != b.is_first()) return false;
            if (a.is_first())
                return a.it1 == b.it1;
            else
                return a.it2 == b.it2;
        };

       private:
        ConcatView* parent;
        iterator1   it1;
        iterator2   it2;

        [[nodiscard]] bool is_first() const { return it1 != std::ranges::end(parent->view1); }
    };

    It begin() { return It{*this}; }
    It end() { return It{*this, std::ranges::end(view1), std::ranges::end(view2)}; }
};

}  // namespace croplines

// #include <vector>
// inline void test() {
//     std::vector<int> i1;
//     auto             c = croplines::Concat(std::views::all(i1), std::views::single(1));
//     using it           = decltype(c.begin());
//     static_assert(std::input_iterator<it>);
//     static_assert(std::ranges::view<decltype(c)>);
// }
