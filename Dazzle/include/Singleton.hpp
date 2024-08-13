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

        static Interface* GetInstance()
        {
            std::call_once( mOnceFlag, []()
            {
                mInstance.reset(new Implementation());
            });
            return mInstance.get();
        }

    private:
        Singleton() = default;
        ~Singleton() = default;

        static std::unique_ptr<Interface> mInstance;
        static std::once_flag mOnceFlag;
    };

    template<typename Interface, typename Implementation>
    std::unique_ptr<Interface> Singleton<Interface, Implementation>::mInstance;
    template<typename Interface, typename Implementation>
    std::once_flag Singleton<Interface, Implementation>::mOnceFlag;
}

#endif // _SINGLETON_HPP_