#ifndef _SINGLETON_HPP_
#define _SINGLETON_HPP_

#include <memory>
#include <mutex>

#include "DazzleEngine.hpp"

namespace Dazzle
{
    template<typename Interface, typename Implementation>
    class Singleton
    {
    public:
        Singleton(const Singleton& other) = delete;
        void operator=(const Singleton&) = delete;

        template<typename... Args>
        static Interface& GetInstance(Args&&... args)
        {
            std::call_once( mInitFlag, [](Args&&... args)
            {
                mInstance.reset(new Implementation(std::forward<Args>(args)...));
            }, std::forward<Args>(args)...);
            return *mInstance;
        }

    private:
        Singleton() = default;
        ~Singleton() = default;

        static std::unique_ptr<Interface> mInstance;
        static std::once_flag mInitFlag;
    };

    template<typename Interface, typename Implementation>
    std::unique_ptr<Interface> Singleton<Interface, Implementation>::mInstance = nullptr;
    template<typename Interface, typename Implementation>
    std::once_flag Singleton<Interface, Implementation>::mInitFlag;
}

#endif // _SINGLETON_HPP_