#include <iostream>
using std::cout;

#include "../rubber_types.hpp"
namespace rt = rubber_types;


// Define the Concept that your api will expect.
// Encapsulate it in a spec class.
struct WidgetSpec {
    // Define the abstract base class.
    // It has to be called `Concept`.
    struct Concept {
        virtual ~Concept() = default;
        virtual void render() const = 0;
    };

    // Define the model class.
    // It is a template class that expects a value holder parameter.
    // The Model has to inherit publicly from `Holder`, and publicly and
    // virtually from `Concept`.
    template <class Holder>
    struct Model : public Holder, public virtual Concept {
        // It is mandatory to pull in the constructor.
        using Holder::Holder;
        // The model defers calls to the wrapped object.
        // Gain access to the wrapped object through the free function
        // `model_get`. It returns a const reference to the held value.
        void render() const {
            rt::model_get(this).render();
        }
    };

    // Define the external interface.
    // This is the interface that the type-erasure container will provide to
    // the outside world. It should generally follow the interface that you
    // expect of the wrapped types.
    // The external interface inherits from a template parameter, which is the
    // Model container.
    template <class Container>
    struct ExternalInterface : public Container {
        // It is mandatory to pull in the constructor.
        using Container::Container;
        void render() const {
            // The free function `interface_get` obtains the Model from the
            // container. It returns a const reference to a Model, as defined
            // above by yourself.
            rt::interface_get(this).render();
        }
    };
};

// Use the Concept spec to generate a type-erasure container.
using Widget = rt::TypeErasure<WidgetSpec>;

// Use the concept according to the defined interface.
void render_widget(const Widget &w) {
    w.render();
}

// The above function can now be used for an arbitrary object, provided it
// fulfills the interface.
struct UserWidget {
    void render() const {
        cout << "UserWidget::render\n";
    }
};

int main() {
    UserWidget w;
    render_widget(w);
}
