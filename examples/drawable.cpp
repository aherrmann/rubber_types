// Drawable Example
//
// This is a re-implementation of the example, that Sean Parent gives in his
// presentation [1] using the Rubber-Type library.

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
using std::cout;
using std::ostream;
using std::string;
using std::vector;

#include "../rubber_types.hpp"
namespace rt = rubber_types;

void draw(int i, ostream &o, int pos = 0) {
    o << string(pos * 2, ' ') << i << "\n";
}

void draw(const string &s, ostream &o, int pos = 0) {
    o << string(pos * 2, ' ') << s << "\n";
}

struct DrawableSpec {
    struct Concept {
        virtual ~Concept() = default;
        virtual void draw_(ostream &o, int pos) const = 0;
    };

    template <class Holder>
    struct Model : public Holder, public virtual Concept {
        using Holder::Holder;
        virtual void draw_(ostream &o, int pos) const {
            draw(rt::model_get(this), o, pos);
        }
    };

    template <class Container>
    struct ExternalInterface : public Container {
        using Container::Container;
        friend void draw(const ExternalInterface &e, ostream &o, int pos) {
            rt::interface_get(e).draw_(o, pos);
        }
    };
};

using Drawable = rt::TypeErasure<DrawableSpec>;

using Document = vector<Drawable>;

void draw(const Document &doc, ostream &o, int pos = 0) {
    o << string(pos * 2, ' ') << "<document>\n";
    for (auto& item : doc) {
        draw(item, o, pos + 1);
    }
    o << string(pos * 2, ' ') << "</document>\n";
}

using History = vector<Document>;

void commit(History &h) {
    assert(h.size());
    h.push_back(h.back());
}

void undo(History &h) {
    assert(h.size());
    h.pop_back();
}

Document &current(History &h) {
    assert(h.size());
    return h.back();
}

int main() {
    History h(1);

    current(h).emplace_back(0);
    current(h).emplace_back(1);

    draw(current(h), cout);
    cout << "--------------------------\n";

    commit(h);

    current(h).emplace_back(string("Hello World"));
    current(h).emplace_back(current(h));
    current(h)[1] = string("Roof");

    draw(current(h), cout);
    cout << "--------------------------\n";

    undo(h);

    draw(current(h), cout);
}
