#pragma once

#include <vector>
#include <utility>

namespace glRemix
{

template<typename T>
class FreeListVector
{
public:
    FreeListVector() = default;
    ~FreeListVector() = default;

    FreeListVector(const FreeListVector&) = default;
    FreeListVector& operator=(const FreeListVector&) = default;
    FreeListVector(FreeListVector&&) = default;
    FreeListVector& operator=(FreeListVector&&) = default;

    // Replaces any free slots, otherwise appends to the end
    unsigned push_back(T&& element);

    void free(unsigned id);

    T& operator[](unsigned id);

    const T& operator[](unsigned id) const;

    // Includes both used and freed elements
    size_t size() const
    {
        return m_elements.size();
    }

    // Reserve capacity by growing the vector based on difference between requested length and free slots
    void reserve(size_t desired_size)
    {
        const size_t current_capacity = m_elements.size();
        const size_t free_count = m_free_indices.size();
        const size_t used_count = current_capacity - free_count;

        if (desired_size > used_count)
        {
            const size_t needed = desired_size - used_count;
            m_elements.reserve(current_capacity + needed);
        }
    }

    void clear();

private:
    std::vector<T> m_elements;
    std::vector<unsigned> m_free_indices;
};

template<typename T>
unsigned FreeListVector<T>::push_back(T&& element)
{
    if (!m_free_indices.empty())
    {
        unsigned id = m_free_indices.back();
        m_free_indices.pop_back();

        m_elements[id] = std::move(element);
        return id;
    }

    m_elements.push_back(std::move(element));
    return static_cast<unsigned>(m_elements.size() - 1);
}

template<typename T>
void FreeListVector<T>::free(unsigned id)
{
    m_free_indices.push_back(id);
}

template<typename T>
T& FreeListVector<T>::operator[](unsigned id)
{
    return m_elements[id];
}

template<typename T>
const T& FreeListVector<T>::operator[](unsigned id) const
{
    return m_elements[id];
}

template<typename T>
void FreeListVector<T>::clear()
{
    m_elements.clear();
    m_free_indices.clear();
}

}  // namespace glRemix
