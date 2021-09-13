#pragma once

#include <CL/sycl.hpp>
#include "vec.h"


namespace fdb {


static constexpr struct HostHandler {} host;


struct DeviceHandler {
    sycl::handler *m_cgh;

    DeviceHandler(sycl::handler &cgh) : m_cgh(&cgh) {}

    template <class Key, size_t Dim, class Kernel>
    void parallelFor(vec<Dim, size_t> range, Kernel kernel) const {
        m_cgh->parallel_for<Key>(vec_to_other<sycl::range<Dim>>(range), [=] (sycl::id<Dim> idx) {
            auto id = other_to_vec<Dim>(idx);
            kernel(std::as_const(id));
        });
    }

    template <class Key, class Kernel>
    void parallelFor(size_t range, Kernel kernel) const {
        return parallelFor<Key>(vec<1, size_t>(range), [=] (vec<1, size_t> id) {
            return kernel(std::as_const(id[0]));
        });
    }
};


struct CommandQueue {
    sycl::queue m_que;

    template <bool IsBlocked = true, class Functor>
    void submit(Functor const &functor) {
        auto event = m_que.submit([&] (sycl::handler &cgh) {
            DeviceHandler dev(cgh);
            functor(std::as_const(dev));
        });
        if constexpr (IsBlocked) {
            event.wait();
        }
    }

    void wait() {
        m_que.wait();
    }
};


enum class Access {
    read = (int)sycl::access::mode::read,
    write = (int)sycl::access::mode::write,
    read_write = (int)sycl::access::mode::read_write,
    discard_write = (int)sycl::access::mode::write,
    discard_read_write = (int)sycl::access::mode::read_write,
};


template <class T, size_t Dim = 1>
struct NDArray {
    static_assert(Dim > 0);

    sycl::buffer<T, Dim> m_buffer;
    vec<Dim, size_t> m_shape;

    NDArray() = default;

    explicit NDArray(vec<Dim, size_t> shape, T *data = nullptr)
        : m_buffer(data, vec_to_other<sycl::range<Dim>>(shape))
        , m_shape(shape)
    {}

    auto const &shape() const {
        return m_shape;
    }

    void reshape(vec<Dim, size_t> shape) {
        m_buffer = sycl::buffer<T, Dim>(
                (T *)nullptr, vec_to_other<sycl::range<Dim>>(shape));
        m_shape = shape;
    }

    template <sycl::access::mode Mode, sycl::access::target Target>
    struct Accessor {
        sycl::accessor<T, Dim, Mode, Target> acc;

        template <class ...Args>
        Accessor(NDArray &parent, Args &&...args)
            : acc(parent.m_buffer.template get_access<Mode>(
                        std::forward<Args>(args)...))
        {}

        using ReferenceT = std::conditional_t<Mode == sycl::access::mode::read,
              T const &, T &>;

        inline ReferenceT operator[](vec<Dim, size_t> indices) const {
            return acc[vec_to_other<sycl::id<Dim>>(indices)];
        }

        template <class ...Indices>
        inline ReferenceT operator()(Indices &&...indices) const {
            return operator[](vec<Dim, size_t>(std::forward<Indices>(indices)...));
        }
    };

    template <auto Mode = Access::read_write>
    auto accessor(HostHandler hand) {
        return Accessor<(sycl::access::mode)(int)Mode,
               sycl::access::target::host_buffer>(*this);
    }

    template <auto Mode = Access::read_write>
    auto accessor(DeviceHandler hand) {
        return Accessor<(sycl::access::mode)(int)Mode,
               sycl::access::target::global_buffer>(*this, *hand.m_cgh);
    }
};


CommandQueue &getQueue();


}