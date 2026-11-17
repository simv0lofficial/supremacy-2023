#pragma once
#include <cstddef>
#include <span>
#include <utility>

namespace skeet::skins {
namespace catalog_detail {
template <class Option>
void adjust_heap(std::span<Option> rows, std::size_t first, std::size_t count, std::size_t hole,
                 Option value) {
    const auto top = hole;
    auto child = hole * 2 + 2;
    while (child < count) {
        if (rows[first + child].name < rows[first + child - 1].name)
            --child;
        rows[first + hole] = std::move(rows[first + child]);
        hole = child;
        child = hole * 2 + 2;
    }
    if (child == count) {
        rows[first + hole] = std::move(rows[first + child - 1]);
        hole = child - 1;
    }
    while (hole > top) {
        const auto parent = (hole - 1) / 2;
        if (!(rows[first + parent].name < value.name))
            break;
        rows[first + hole] = std::move(rows[first + parent]);
        hole = parent;
    }
    rows[first + hole] = std::move(value);
}
template <class Option>
void heap_sort(std::span<Option> rows, std::size_t first, std::size_t last) {
    const auto count = last - first;
    if (count < 2)
        return;
    for (auto parent = (count - 2) / 2 + 1; parent != 0;) {
        --parent;
        auto value = std::move(rows[first + parent]);
        adjust_heap(rows, first, count, parent, std::move(value));
    }
    for (auto length = count; length > 1; --length) {
        auto value = std::move(rows[first + length - 1]);
        rows[first + length - 1] = std::move(rows[first]);
        adjust_heap(rows, first, length - 1, 0, std::move(value));
    }
}
template <class Option>
void introsort(std::span<Option> rows, std::size_t first, std::size_t last, std::size_t depth) {
    for (;;) {
        const auto count = last - first;
        if (count < 29 || depth == 0) {
            if (depth == 0)
                heap_sort(rows, first, last);
            return;
        }
        auto middle = first + count / 2;
        const auto tail = last - 1;
        if (rows[first].name < rows[middle].name) {
            if (!(rows[middle].name < rows[tail].name))
                middle = rows[first].name < rows[tail].name ? tail : first;
        } else if (rows[first].name < rows[tail].name)
            middle = first;
        else if (rows[middle].name < rows[tail].name)
            middle = tail;
        const auto pivot = rows[middle].name;
        auto left = first, right = last;
        for (;;) {
            while (rows[left].name < pivot)
                ++left;
            do {
                --right;
            } while (pivot < rows[right].name);
            if (left >= right)
                break;
            std::swap(rows[left], rows[right]);
            ++left;
        }
        --depth;
        introsort(rows, left, last, depth);
        last = left;
    }
}
template <class Option>
void insertion_sort(std::span<Option> rows, std::size_t first, std::size_t last, bool guarded) {
    for (auto index = first; index < last; ++index) {
        auto value = std::move(rows[index]);
        auto hole = index;
        while ((!guarded || hole != 0) && value.name < rows[hole - 1].name) {
            rows[hole] = std::move(rows[hole - 1]);
            --hole;
        }
        rows[hole] = std::move(value);
    }
}
} //namespace catalog_detail
// Complete433B69E8 family for glove options and the identical20-byte-row
// sorting policy in434084F0 for weapon paints. The native median-of-three,
// right-first partition recursion, threshold28 and heap tie choices determine
// the order of IDs sharing a label. UTF16 names compare unsigned code units.
template <class Option> void sort_catalog_choices(std::span<Option> rows) {
    if (rows.empty())
        return;
    auto bits = std::size_t(0);
    for (auto count = rows.size(); count; count >>= 1)
        ++bits;
    catalog_detail::introsort(rows, 0, rows.size(), bits * 2 - 2);
    const auto boundary = rows.size() > 28 ? std::size_t(28) : rows.size();
    catalog_detail::insertion_sort(rows, 1, boundary, true);
    if (rows.size() > 28)
        catalog_detail::insertion_sort(rows, boundary, rows.size(), false);
}
} //namespace skeet::skins
