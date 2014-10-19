#include <iostream>
using std::cout;

#include "../rubber_types.hpp"
namespace rt = rubber_types;


struct WidgetSpec {
    struct Concept {
        virtual ~Concept() = default;
        virtual void render() const = 0;
    };

    template <class Holder>
    struct Model : public Holder, public virtual Concept {
        using Holder::Holder;
        void render() const {
            rt::model_get(this).render();
        }
    };

    template <class Container>
    struct ExternalInterface : public Container {
        using Container::Container;
        void render() const {
            rt::interface_get(this).render();
        }
    };
};

struct GeometrySpec {
    struct Concept {
        virtual ~Concept() = default;
        virtual int size() const = 0;
    };

    template <class Holder>
    struct Model : public Holder, public virtual Concept {
        using Holder::Holder;
        int size() const {
            return rt::model_get(this).size();
        }
    };

    template <class Container>
    struct ExternalInterface : public Container {
        using Container::Container;
        int size() const {
            return rt::interface_get(this).size();
        }
    };
};

using Widget = rt::TypeErasure<WidgetSpec>;
using Geometry = rt::TypeErasure<GeometrySpec>;

// You can directly combine two concepts:
using GeomWidget = rt::MergeConcepts<Geometry, Widget>;
// You could also create the type-erasure from a list of specs.
// using GeomWidget = rt::TypeErasure<GeometrySpec, WidgetSpec>;

void render_widget(const Widget &w) {
    w.render();
}
void measure_geometry(const Geometry &g) {
    cout << "size: " << g.size() << "\n";
}
void measure_and_render(const GeomWidget &gw) {
    cout << "size: " << gw.size() << "\n";
    gw.render();
}

// The above function can now be used for an arbitrary object, provided it
// fulfills the interface.
struct UserWidget {
    void render() const {
        cout << "UserWidget::render\n";
    }
    int size() const {
        return 42;
    }
};

int main() {
    UserWidget w;

    render_widget(w);
    cout << "\n";
    measure_geometry(w);
    cout << "\n";
    measure_and_render(w);
}

