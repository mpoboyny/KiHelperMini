//
// ScopeGuard.hxx
//

#ifndef SCOPE_GUARD_HXX
#define SCOPE_GUARD_HXX

//
// example of use in some scope
//
// auto cleanup = ScopeGuard{[&]{ std::cout << "Cleanup !\n"; }};
// auto cleanup = ScopeGuard{[&]{ MyCleanUpFu(); }};
//
// Some thing like this must work

//     MyClass1 a = nullptr;
//     MyClass1 b = nullptr;
//     auto cleanup = ScopeGuard {[&] { 
//         std::cout << "Cleanup !\n"; 
//         delete a;
//         delete b;
//         CleanFu();
//     }};

template <typename F>
class ScopeGuard 
{
    // Compile-time check: Ensure F can be called with zero arguments
    static_assert(std::is_invocable_v<F>, "ScopeGuard: F must be a callable (lambda, function pointer, etc.)");
    
    public:
        // Move the callable into the guard
        explicit ScopeGuard(F f) : func(std::move(f)), active(true) {}

        // Destructor runs the cleanup only if active
        ~ScopeGuard() {
            if (active) {
                func();
            }
        }

        // Disable copying for safety
        ScopeGuard(const ScopeGuard&) = delete;
        ScopeGuard& operator=(const ScopeGuard&) = delete;

        // Support moving the guard
        ScopeGuard(ScopeGuard&& other) noexcept 
            : func(std::move(other.func)), active(other.active) {
            other.active = false;
        }

        // Cancel the cleanup if needed
        void dismiss() noexcept {
            active = false;
        }

    private:
        F func;
        bool active;
};

// Deduction Guide for C++17
template <typename F>
ScopeGuard(F) -> ScopeGuard<F>;

#endif  // SCOPEGUARD_HXX
