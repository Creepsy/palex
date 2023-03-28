#pragma once

// used for easier use of std::visit of std::variant
// inspired by https://stackoverflow.com/questions/64017982/c-equivalent-of-rust-enums 

template <class ...visitors_T>
struct Visitor : public visitors_T... {
    using visitors_T::operator()...;
};

template <class ...visitors_T>
Visitor(visitors_T...) -> Visitor<visitors_T...>;