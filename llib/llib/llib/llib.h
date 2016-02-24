#ifndef _INCLUDE_LLIB_H_
#define _INCLUDE_LLIB_H_

#include <type_traits>

#include <cassert>

namespace llib {
    namespace functional {
        template<typename F>
        auto make_curry_function(F&& f);

        template<typename F>
        class curry_function {
        private:
            F f_;
        private:
            template<typename... Arg>
            auto call_impl(std::true_type, Arg&&... arg)const {
                return f_(std::forward<Arg>(arg)...);
            }
            template<typename... Arg>
            auto call_impl(std::false_type, Arg&&... arg)const {
                return make_curry_function(
                    [arg..., this](auto&&... args)
                {return f_(arg..., std::forward<decltype(args)...>(args)...);});
            }
            //sfnae
            template<typename... Args, typename = decltype(f_(std::declval<Args>()...), std::true_type())>
            constexpr std::true_type bool_type(int)const { return std::true_type{}; }
            template<typename... Args>
            constexpr std::false_type bool_type(...)const { return std::false_type{}; }
        public:
            curry_function(F&& f) :f_(std::move(f)) {}

            template<typename... Args>
            auto operator()(Args&&... args)const {
                return call_impl(bool_type<Args...>(0), std::forward<Args>(args)...);
            }
        };

        template<typename F>
        auto make_curry_function(F&& f) {
            return curry_function<F>(std::forward<F>(f));
        }
    }

    namespace test {
        inline void test_curry_function() {
            auto f = llib::functional::make_curry_function([](auto x, auto y) {return x + y;});
            auto f0 = f(2);
            auto f1 = f0(1);
            auto f3 = f(2, 1);
            assert(f1 == f3);
        }
    }

  
}


namespace llib {
    namespace meta {
        namespace detail {
            template<typename L>
            struct is_valid_impl {
                template<typename P, typename = decltype(std::declval<L>()(std::declval<P>()))>
                auto test_valid(int)const{
                    return std::true_type{};
                }
                template<typename P>
                auto test_valid(...)const {
                    return std::false_type{};
                }
            public:
                template<typename P>
                auto operator()(P&& x)const {
                    return test_valid<P>(int());
                }
            };
        }

        template<typename F>
        constexpr auto is_valid(F&&) {
            return detail::is_valid_impl<F>();
        }
    }

    namespace test {
        struct x_t {
            int id;
            void valid_test(int x) {}
        };

        inline void test_is_valid() {
            auto has_valid_test = llib::meta::is_valid([](auto&& x)->decltype(x.valid_test(0)) {});
            auto has_id = llib::meta::is_valid([](auto&& x)->decltype(x.id) {});
            x_t x;
            static_assert(decltype(has_valid_test(x))::value, "has valid_test");
            static_assert(decltype(has_id(x))::value, "has valid_test");
            static_assert(!decltype(has_id(0))::value, "has valid_test");
        }
    }
}
#endif