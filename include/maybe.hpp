#pragma once

#include <type_traits>
#include <utility>
#include <functional>

namespace maybe {

template<typename T> class Value;
template<typename E> class Error;

template<typename T, typename E>
class Maybe;

template<typename T>
struct is_maybe : std::false_type {};

template<typename T, typename E>
struct is_maybe<Maybe<T, E>> : std::true_type {};

template<typename T>
class Value {
    T val;

    template<typename U, typename E>
    friend class Maybe;

        template<typename... Args>
    explicit constexpr Value(Args&&... args) 
        : val(std::forward<Args>(args)...) {}

public:
    constexpr T& get() & noexcept { return val; }
    constexpr const T& get() const & noexcept { return val; }
    constexpr T&& get() && noexcept { return std::move(val); }
    constexpr const T&& get() const && noexcept { return std::move(val); }
};

template<typename E>
class Error {
    E err;

    template<typename T, typename F>
    friend class Maybe;

        template<typename... Args>
    explicit constexpr Error(Args&&... args) 
        : err(std::forward<Args>(args)...) {}

public:
    constexpr E& get() & noexcept { return err; }
    constexpr const E& get() const & noexcept { return err; }
    constexpr E&& get() && noexcept { return std::move(err); }
    constexpr const E&& get() const && noexcept { return std::move(err); }
};

template<typename E>
class unexpected {
    E val;
public:
    unexpected() = delete;
    constexpr explicit unexpected(const E& e) : val(e) {}
    constexpr explicit unexpected(E&& e) : val(std::move(e)) {}

    constexpr const E& value() const & noexcept { return val; }
    constexpr E& value() & noexcept { return val; }
    constexpr const E&& value() const && noexcept { return std::move(val); }
    constexpr E&& value() && noexcept { return std::move(val); }
};

template<typename E>
unexpected(E) -> unexpected<E>;

struct unexpect_t { explicit unexpect_t() = default; };
inline constexpr unexpect_t unexpect{};

template<typename T, typename E>
class Maybe {
    union {
        Value<T> value;
        Error<E> error;
    };
    bool has_val;

public:
    using value_type = T;
    using error_type = E;
    using unexpected_type = unexpected<E>;

        template<typename U = T,
             std::enable_if_t<!std::is_same_v<std::remove_cv_t<std::remove_reference_t<U>>, Maybe> &&
                             !std::is_same_v<std::remove_cv_t<std::remove_reference_t<U>>, unexpect_t> &&
                             std::is_constructible_v<T, U>, int> = 0>
    constexpr Maybe(U&& v) 
        : value(std::forward<U>(v)), has_val(true) {}

        template<typename G = E>
    constexpr Maybe(const unexpected<G>& e)
        : error(e.value()), has_val(false) {}

    template<typename G = E>
    constexpr Maybe(unexpected<G>&& e)
        : error(std::move(e.value())), has_val(false) {}

    template<typename... Args>
    constexpr explicit Maybe(unexpect_t, Args&&... args)
        : error(std::forward<Args>(args)...), has_val(false) {}

        ~Maybe() {
        if (has_val) {
            value.~Value();
        } else {
            error.~Error();
        }
    }

        Maybe(Maybe&& other) noexcept
        : has_val(other.has_val) {
        if (has_val) {
            new (&value) Value<T>(std::move(other.value));
        } else {
            new (&error) Error<E>(std::move(other.error));
        }
    }

    Maybe& operator=(Maybe&& other) noexcept {
        if (this != &other) {
            if (has_val) {
                value.~Value();
            } else {
                error.~Error();
            }
            
            has_val = other.has_val;
            if (has_val) {
                new (&value) Value<T>(std::move(other.value));
            } else {
                new (&error) Error<E>(std::move(other.error));
            }
        }
        return *this;
    }

        Maybe(const Maybe&) = delete;
    Maybe& operator=(const Maybe&) = delete;

    constexpr bool has_value() const noexcept { return has_val; }
    constexpr explicit operator bool() const noexcept { return has_val; }

        template<typename U = T>
    constexpr const Value<T>& as_value() const & {
        if (!has_val) throw std::bad_variant_access{};
        return value;
    }

    template<typename U = T>
    constexpr Value<T>& as_value() & {
        if (!has_val) throw std::bad_variant_access{};
        return value;
    }

    template<typename U = T>
    constexpr Value<T>&& as_value() && {
        if (!has_val) throw std::bad_variant_access{};
        return std::move(value);
    }

        constexpr const Error<E>& as_error() const & noexcept {
        return error;
    }

    constexpr Error<E>& as_error() & noexcept {
        return error;
    }

    constexpr Error<E>&& as_error() && noexcept {
        return std::move(error);
    }

        template<typename F>
    constexpr auto and_then(F&& f) & {
        using U = std::invoke_result_t<F, Value<T>&>;
        static_assert(is_maybe<U>::value, "F must return a Maybe");

        if (has_val) {
            return std::invoke(std::forward<F>(f), value);
        } else {
            return U(unexpect, error.get());
        }
    }

    template<typename F>
    constexpr auto transform(F&& f) & {
        using U = std::invoke_result_t<F, Value<T>&>;
        using Result = Maybe<U, E>;

        if (has_val) {
            return Result(std::invoke(std::forward<F>(f), value));
        } else {
            return Result(unexpect, error.get());
        }
    }

    template<typename F>
    constexpr auto transform_error(F&& f) & {
        using G = std::invoke_result_t<F, Error<E>&>;
        using Result = Maybe<T, G>;

        if (has_val) {
            return Result(value.get());
        } else {
            return Result(unexpect, std::invoke(std::forward<F>(f), error));
        }
    }
};

} // namespace maybe