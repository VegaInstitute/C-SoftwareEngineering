#include <algorithm>
#include <iostream>
#include <iterator>

class MyArrayForwardIterator {
    int *ptr;

    MyArrayForwardIterator(int *ptr) : ptr{ptr} {}

    template <int N>
    friend class MyArray;

   public:
    using value_type = int;
    using difference_type = ptrdiff_t;
    using reference = value_type &;
    using pointer = value_type *;
    using iterator_category = std::forward_iterator_tag;

    friend bool operator==(const MyArrayForwardIterator &lhs,
                           const MyArrayForwardIterator &rhs) {
        return lhs.ptr == rhs.ptr;
    }

    friend bool operator!=(const MyArrayForwardIterator &lhs,
                           const MyArrayForwardIterator &rhs) {
        return lhs.ptr != rhs.ptr;
    }

    reference operator*() const { return *ptr; }
    MyArrayForwardIterator &operator++() {
        ++ptr;
        return *this;
    }
};

template <int N>
class MyArray final {
    int *arr;

   public:
    using iterator = MyArrayForwardIterator;

    MyArray() = delete;

    template <typename ForwardIt>
    explicit MyArray(ForwardIt from) {
        arr = new int[N];
        for (size_t i = 0; i < N; ++i) {
            arr[i] = *from++;
        }
    }

    iterator begin() { return iterator(arr); }
    iterator end() { return iterator(&arr[N]); }

    friend std::ostream &operator<<(std::ostream &os, const MyArray &arr) {
        for (size_t i = 0; i < N; ++i) {
            os << arr.arr[i] << '\n';
        }
        return os;
    }

    ~MyArray() { delete[] arr; }
};

int main() {
    auto numbers = {0, 1, 2, 2, 3};
    auto from = numbers.begin();
    MyArray<5> arr(from);
    std::cout << arr << '\n';
    std::cout << std::count(arr.begin(), arr.end(), 2) << "\n\n";
    std::fill(arr.begin(), arr.end(), 5);
    std::cout << arr << '\n';

    return 0;
}
