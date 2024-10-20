/* A backport of C++23's `std::expected` class template
 *   for compatibility with C++17.
 */

#pragma once

#include <type_traits>
#include <utility>
#include <stdexcept>
#include <functional>
#include <variant>
#include <exception>
#include <optional>

template<class E>
class unexpected {
public:
    unexpected() = delete;
    constexpr explicit unexpected(const E& e) : val(e) {}
    constexpr explicit unexpected(E&& e) : val(std::move(e)) {}

    constexpr const E& value() const & noexcept { return val; }
    constexpr E& value() & noexcept { return val; }
    constexpr const E&& value() const && noexcept { return std::move(val); }
    constexpr E&& value() && noexcept { return std::move(val); }

private:
    E val;
};

template<class E>
unexpected(E) -> unexpected<E>;

struct unexpect_t {
    explicit unexpect_t() = default;
};
inline constexpr unexpect_t unexpect{};

template<class T, class E>
class Maybe {
public:
    using value_type = T;
    using error_type = E;
    using unexpected_type = unexpected<E>;

    template<class U = T, 
             std::enable_if_t<!std::is_same_v<std::remove_cv_t<std::remove_reference_t<U>>, Maybe> &&
                              !std::is_same_v<std::remove_cv_t<std::remove_reference_t<U>>, unexpect_t> &&
                              std::is_constructible_v<T, U>, int> = 0>
    constexpr Maybe(U&& v) : var(std::in_place_index<0>, std::forward<U>(v)) {}

    template<class G = E>
    constexpr Maybe(const unexpected<G>& e) : var(std::in_place_index<1>, e.value()) {}

    template<class G = E>
    constexpr Maybe(unexpected<G>&& e) : var(std::in_place_index<1>, std::move(e.value())) {}

    template<class... Args>
    constexpr explicit Maybe(unexpect_t, Args&&... args) 
        : var(std::in_place_index<1>, std::forward<Args>(args)...) {}

    constexpr bool has_value() const noexcept { return var.index() == 0; }
    constexpr explicit operator bool() const noexcept { return has_value(); }

    constexpr T& value() & {
        if (!has_value()) throw std::bad_optional_access();
        return std::get<0>(var);
    }
    constexpr const T& value() const & {
        if (!has_value()) throw std::bad_optional_access();
        return std::get<0>(var);
    }
    constexpr T&& value() && {
        if (!has_value()) throw std::bad_optional_access();
        return std::move(std::get<0>(var));
    }
    constexpr const T&& value() const && {
        if (!has_value()) throw std::bad_optional_access();
        return std::move(std::get<0>(var));
    }

    constexpr E& error() & noexcept { return std::get<1>(var); }
    constexpr const E& error() const & noexcept { return std::get<1>(var); }
    constexpr E&& error() && noexcept { return std::move(std::get<1>(var)); }
    constexpr const E&& error() const && noexcept { return std::move(std::get<1>(var)); }

    template<class U>
    constexpr T value_or(U&& default_value) const & {
        return has_value() ? value() : static_cast<T>(std::forward<U>(default_value));
    }

    template<class U>
    constexpr T value_or(U&& default_value) && {
        return has_value() ? std::move(value()) : static_cast<T>(std::forward<U>(default_value));
    }

    template<class F>
    constexpr auto and_then(F&& f) const & {
        using U = std::invoke_result_t<F, const T&>;
        static_assert(is_maybe<U>::value, "F must return a Maybe");

        return has_value() ? std::invoke(std::forward<F>(f), value())
                           : U(unexpect, error());
    }

    template<class F>
    constexpr auto and_then(F&& f) && {
        using U = std::invoke_result_t<F, T&&>;
        static_assert(is_maybe<U>::value, "F must return a Maybe");

        return has_value() ? std::invoke(std::forward<F>(f), std::move(value()))
                           : U(unexpect, std::move(error()));
    }

    template<class F>
    constexpr auto or_else(F&& f) const & {
        using G = std::invoke_result_t<F, const E&>;
        static_assert(is_maybe<G>::value, "F must return a Maybe");

        return has_value() ? G(value()) : std::invoke(std::forward<F>(f), error());
    }

    template<class F>
    constexpr auto or_else(F&& f) && {
        using G = std::invoke_result_t<F, E&&>;
        static_assert(is_maybe<G>::value, "F must return a Maybe");

        return has_value() ? G(std::move(value())) : std::invoke(std::forward<F>(f), std::move(error()));
    }

    template<class F>
    constexpr auto transform(F&& f) const & {
        using U = std::invoke_result_t<F, const T&>;
        return has_value() ? Maybe<U, E>(std::invoke(std::forward<F>(f), value()))
                           : Maybe<U, E>(unexpect, error());
    }

    template<class F>
    constexpr auto transform(F&& f) && {
        using U = std::invoke_result_t<F, T&&>;
        return has_value() ? Maybe<U, E>(std::invoke(std::forward<F>(f), std::move(value())))
                           : Maybe<U, E>(unexpect, std::move(error()));
    }

    template<class F>
    constexpr auto transform_error(F&& f) const & {
        using G = std::invoke_result_t<F, const E&>;
        return has_value() ? Maybe<T, G>(value())
                           : Maybe<T, G>(unexpect, std::invoke(std::forward<F>(f), error()));
    }

    template<class F>
    constexpr auto transform_error(F&& f) && {
        using G = std::invoke_result_t<F, E&&>;
        return has_value() ? Maybe<T, G>(std::move(value()))
                           : Maybe<T, G>(unexpect, std::invoke(std::forward<F>(f), std::move(error())));
    }

private:
    template<class U>
    struct is_maybe : std::false_type {};

    template<class U, class G>
    struct is_maybe<Maybe<U, G>> : std::true_type {};

    std::variant<T, E> var;
};

template<class T, class E>
constexpr bool operator==(const Maybe<T, E>& lhs, const Maybe<T, E>& rhs) {
    if (lhs.has_value() != rhs.has_value()) return false;
    return lhs.has_value() ? lhs.value() == rhs.value() : lhs.error() == rhs.error();
}

template<class T, class E>
constexpr bool operator!=(const Maybe<T, E>& lhs, const Maybe<T, E>& rhs) {
    return !(lhs == rhs);
}

template<class T, class E>
constexpr bool operator==(const Maybe<T, E>& lhs, const unexpected<E>& rhs) {
    return !lhs.has_value() && lhs.error() == rhs.value();
}

template<class T, class E>
constexpr bool operator==(const unexpected<E>& lhs, const Maybe<T, E>& rhs) {
    return rhs == lhs;
}

template<class T, class E>
constexpr bool operator!=(const Maybe<T, E>& lhs, const unexpected<E>& rhs) {
    return !(lhs == rhs);
}

template<class T, class E>
constexpr bool operator!=(const unexpected<E>& lhs, const Maybe<T, E>& rhs) {
    return !(rhs == lhs);
}

template<class T, class E, class U>
constexpr bool operator==(const Maybe<T, E>& lhs, const U& rhs) {
    return lhs.has_value() && lhs.value() == rhs;
}

template<class T, class E, class U>
constexpr bool operator==(const U& lhs, const Maybe<T, E>& rhs) {
    return rhs == lhs;
}

template<class T, class E, class U>
constexpr bool operator!=(const Maybe<T, E>& lhs, const U& rhs) {
    return !(lhs == rhs);
}

template<class T, class E, class U>
constexpr bool operator!=(const U& lhs, const Maybe<T, E>& rhs) {
    return !(rhs == lhs);
}