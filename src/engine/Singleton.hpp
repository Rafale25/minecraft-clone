#pragma once

// template<typename T>
// class Singleton {
//     protected:
//         class P { friend class Singleton<T>; P() = default; };

//     public:
//         Singleton(P) {}

//         static T& instance() {
//             static T instance { P {} };
//             return instance;
//         }
// };

template<typename T>
class Singleton {

    public:
        // Singleton() {}

        // T(const T&) = delete;
        T& operator=(const T&) = delete;
        // T(T&&) = delete;
        T& operator=(T&&) = delete;

        static T& instance() {
            static T instance;
            return instance;
        }
};
