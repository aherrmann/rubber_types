// Rubber-Types -- Type-erasure with merged concepts
// Copyright (C) 2014 Andreas J. Herrmann
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including without
// limitation the rights to use, copy, modify, merge, publish, distribute,
// sublicense, and/or sell copies of the Software, and to permit persons to
// whom the Software is furnished to do so, subject to the following
// conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
// ------------------------------------------------------------------
//
// Usage instructions:
//
// // Include header
// #include "rubber_types.hpp"
//
// // Define spec
// struct MySpec {
//     struct Concept {  // abstract base class for model.
//         virtual ~Concept() = default;
//         // Your (internal) interface goes here.
//         virtual int method() const = 0
//         virtual void free_func_() const = 0;
//     };
//     template <class Holder>
//     struct Model : public Holder, public virtual Concept {
//         using Holder::Holder;
//         // Pass through to encapsulated value.
//         virtual int method() const override {
//             return rubber_types::model_get(this).method();
//         }
//         virtual void free_func_() const override {
//             free_func(rubber_types::model_get(this));
//         }
//     };
//     template <class Container>
//     struct ExternalInterface : public Container {
//         using Container::Container;
//         // Define the external interface. Should match encapsulated type.
//         int method() const {
//             return rubber_types::interface_get(this).method();
//         }
//         friend void free_func(const ExternalInterface &e) {
//             rubber_types::interface_get(e).free_func_();
//         }
//     };
// };
//
// // Make type-erasure class
// using MyConcept = rubber_types::TypeErasure<MySpec>;
// using MyMerge1 = rubber_types::TypeErasure<MySpec1, MySpec2, ...>;
// using MyMerge2 = rubber_types::MergeConcepts<MyConcept1, MyConcept2, ...>;
//
// // Use it with anything that fulfills the interface. (Duck-typing)
// int call_method(const MyConcept &obj) {
//     return obj.method();
// }

#include <memory>

namespace rubber_types {

namespace detail {

template <class T>
class Holder {
  public:
    using Data = T;

    Holder(T obj) : data_(std::move(obj)) {}
    virtual ~Holder() = default;
    T &get_from_rubber_types_holder_() { return data_; }
    const T &get_from_rubber_types_holder_() const { return data_; }

  private:
    T data_;
};

// Merged concept models chaining inheritance of the above holder and models.
// This template meta-function peels off inheritance layers until it reaches
// the above defined holder.
template <class Holder>
struct unwrap_holder;
template <class T>
struct unwrap_holder<Holder<T>> {
    using type = Holder<T>;
};
template <template <class> class Model, class Holder>
struct unwrap_holder<Model<Holder>> {
    using type = Holder;
};
template <class T>
using UnwrapHolder = typename unwrap_holder<T>::type;

template <class Concept_, template <class> class Model>
class Container {
  public:
    using Concept = Concept_;

    template <class T>
    Container(T obj)
        : self_(std::make_shared<Model<Holder<T>>>(std::move(obj))) {}

    const Concept &get_from_rubber_types_container_() const {
        return *self_.get();
    }

  private:
    std::shared_ptr<const Concept> self_;
};

// The same peeling for the concept's external interface to reach the
// container.
template <class Container>
struct unwrap_container;
template <class Concept, template <class> class Model>
struct unwrap_container<Container<Concept, Model>> {
    using type = Container<Concept, Model>;
};
template <template <class> class ExternalInterface, class Container>
struct unwrap_container<ExternalInterface<Container>> {
    using type = Container;
};
template <class T>
using UnwrapContainer = typename unwrap_container<T>::type;

// Helpers for spec merging.
template <class Spec>
using ConceptOf = typename Spec::Concept;
template <class Spec, class Holder>
using ModelOf = typename Spec::template Model<Holder>;
template <class Spec, class Container>
using ExternalInterfaceOf =
    typename Spec::template ExternalInterface<Container>;

// Right-fold. A higher order functino, that maps a two argument template-meta
// function and a list of arguments to the following expression:
//     Rfold<F, a1, a2, ...> = F<a1, F<a2, ...> >
template <template <class, class> class Func, class Arg, class... Args>
struct rfold {
    using type = Func<Arg, typename rfold<Func, Args...>::type>;
};
template <template <class, class> class Func, class L, class R>
struct rfold<Func, L, R> {
    using type = Func<L, R>;
};
template <template <class, class> class Func, class... Args>
using Rfold = typename rfold<Func, Args...>::type;

} // namspace detail

// Get the held value out of a model.
// Use this function inside your model-definitions to access the wrapped
// value.
template <class Model>
const typename detail::UnwrapHolder<Model>::Data &
model_get(const Model *this_) {
    using Holder = detail::UnwrapHolder<Model>;
    return this_->Holder::get_from_rubber_types_holder_();
}
template <class Model>
const typename detail::UnwrapHolder<Model>::Data &
model_get(const Model &this_) {
    using Holder = detail::UnwrapHolder<Model>;
    return this_.Holder::get_from_rubber_types_holder_();
}

// Get the held value out of an external interface.
// Use this function inside your external interface-definitions to access the
// model.
template <class ExternalInterface>
const typename detail::UnwrapContainer<ExternalInterface>::Concept &
interface_get(const ExternalInterface *this_) {
    using Container = detail::UnwrapContainer<ExternalInterface>;
    return this_->Container::get_from_rubber_types_container_();
}
template <class ExternalInterface>
const typename detail::UnwrapContainer<ExternalInterface>::Concept &
interface_get(const ExternalInterface &this_) {
    using Container = detail::UnwrapContainer<ExternalInterface>;
    return this_.Container::get_from_rubber_types_container_();
}

// Merge a list of specs into one spec.
template <class... Specs>
struct MergeSpecs {
    struct Concept : public virtual detail::ConceptOf<Specs>... {};

    template <class Holder>
    struct Model : public detail::Rfold<detail::ModelOf, Specs..., Holder>,
                   public virtual Concept {
        using Base = detail::Rfold<detail::ModelOf, Specs..., Holder>;
        using Base::Base;
    };

    template <class Container>
    struct ExternalInterface
        : public detail::Rfold<detail::ExternalInterfaceOf, Specs...,
                               Container> {
        using Base =
            detail::Rfold<detail::ExternalInterfaceOf, Specs..., Container>;
        using Base::Base;
    };
};
template <class Spec>
struct MergeSpecs<Spec> : public Spec {};

namespace detail {

// Construct a type-erasure out of a given spec.
template <class Spec_>
class TypeErasureSingeSpec
    : public Spec_::template ExternalInterface<
          detail::Container<typename Spec_::Concept, Spec_::template Model>> {
    using Base = typename Spec_::template ExternalInterface<
        detail::Container<typename Spec_::Concept, Spec_::template Model>>;

  public:
    using Base::Base;
    using Spec = Spec_;
};

} // namespace detail

// Generate a type-erasure from the given list of specs.
template <class... Specs>
using TypeErasure = detail::TypeErasureSingeSpec<MergeSpecs<Specs...>>;

// Merge existing concepts (type-erasure classes) into a single one.
template <class... Concepts>
using MergeConcepts = TypeErasure<typename Concepts::Spec...>;

} // namespace rubber_types
