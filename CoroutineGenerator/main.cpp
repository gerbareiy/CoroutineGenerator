#include <algorithm>
#include <coroutine>
#include <functional>
#include <iostream>
#include <optional>

template<typename T> class Generator
{
public:
    struct promise_type
    {
        std::optional<T> current_value;

        auto get_return_object()
        {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() { return {}; }

        std::suspend_always final_suspend() noexcept { return {}; }

        std::suspend_always yield_value(T value)
        {
            current_value = std::move(value);
            return {};
        }

        void return_void() {}

        void unhandled_exception() { throw; }
    };

    class Iterator
    {
    public:
        Iterator() noexcept
            : m_coroutine(nullptr)
        {
        }

        explicit Iterator(std::coroutine_handle<promise_type> coroutine)
            : m_coroutine(coroutine)
        {
            advance();
        }

        Iterator& operator++()
        {
            advance();
            return *this;
        }

        T const& operator*() const { return *m_coroutine.promise().current_value; }

        bool operator==(std::default_sentinel_t) const
        {
            return !m_coroutine || m_coroutine.done();
        }

    private:
        std::coroutine_handle<promise_type> m_coroutine;

        void advance()
        {
            m_coroutine.resume();
            if (m_coroutine.done())
            {
                m_coroutine = nullptr;
            }
        }
    };

    Iterator begin()
    {
        if (!m_coroutine)
        {
            return Iterator{nullptr};
        }
        return Iterator{m_coroutine};
    }

    std::default_sentinel_t end() { return {}; }

    Generator(std::coroutine_handle<promise_type> h)
        : m_coroutine(h)
    {
    }

    Generator(Generator const&) = delete;

    Generator(Generator&& other) noexcept
        : m_coroutine(other.m_coroutine)
    {
        other.m_coroutine = nullptr;
    }

    ~Generator()
    {
        if (m_coroutine)
        {
            m_coroutine.destroy();
        }
    }

    Generator& operator=(Generator const&) = delete;

    Generator& operator=(Generator&& other) noexcept
    {
        if (this != &other)
        {
            if (m_coroutine)
            {
                m_coroutine.destroy();
            }
            m_coroutine = other.m_coroutine;
            other.m_coroutine = nullptr;
        }
        return *this;
    }

private:
    std::coroutine_handle<promise_type> m_coroutine;
};

Generator<int>
cor()
{
    for (int i = 0; i < 1000; ++i)
    {
        co_yield i;
    }
}

std::vector<int>
vec()
{
    std::vector<int> v(1000);
    return v;
}

int
main()
{
    for (auto a : cor())
    {
        if (a == 500)
        {
            break;
        }
        std::cout << a << " ";
    }
    std::cout << std::endl;
    std::cout << std::endl;

    for (auto a : cor())
    {
        std::cout << a << " ";
    }
}
