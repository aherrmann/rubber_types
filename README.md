# Rubber-Types — Type-Erasure with merged concepts

Specify your interface (concept). Generate a type-erasure from it. Merge
existing concepts, or type-erasures.

This library allows you write the following code:

``` cpp
using ConceptA = TypeErasure<ConceptASpec>;
using ConceptB = TypeErasure<ConceptBSpec>;
using Merged = MergeConcepts<ConceptA, ConceptB>;

void func(Merged m) {
    m.func_a();  // Using concept A
    m.func_b();  // Using concept B
}
```

You define `ConceptASpec`, and `ConceptBSpec` yourself. And, you get the
merged concept for free.


## Dependencies

 * C++11
 * Standard-library


## Installation

Simply copy and include the header file `rubber_types.hpp`.


## Usage Instructions

Include the header
``` cpp
#include "rubber_types.hpp"
```

Define your interface.
``` cpp
struct MySpec {
    struct Concept {  // abstract base class for model.
        virtual ~Concept() = default;
        // Your (internal) interface goes here.
        virtual int method() const = 0
        virtual void free_func_() const = 0;
    };
    template <class Holder>
    struct Model : public Holder, public virtual Concept {
        using Holder::Holder;
        // Pass through to encapsulated value.
        virtual int method() const override {
            return rubber_types::model_get(this).method();
        }
        virtual void free_func_() const override {
            free_func(rubber_types::model_get(this));
        }
    };
    template <class Container>
    struct ExternalInterface : public Container {
        using Container::Container;
        // Define the external interface. Should match encapsulated type.
        int method() const {
            return rubber_types::interface_get(this).method();
        }
        friend void free_func(const ExternalInterface &e) {
            rubber_types::interface_get(e).free_func_();
        }
    };
};
```

Generate the type-erasure class.
``` cpp
using MyConcept = rubber_types::TypeErasure<MySpec>;
using MyMerge1 = rubber_types::TypeErasure<MySpec1, MySpec2, ...>;
using MyMerge2 = rubber_types::MergeConcepts<MyConcept1, MyConcept2, ...>;
```

And se it with anything that fulfills the interface. Just like duck-typing.
``` cpp
int call_method(const MyConcept &obj) {
    return obj.method();
}
```

## Examples

Have a look at the `examples` folder, or read my [blog
post](http://aherrmann.github.io/programming/2014/10/19/type-erasure-with-merged-concepts/)
for more behind the scenes information.


## License

MIT
