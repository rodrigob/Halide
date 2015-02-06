#include "RDom.h"
#include "Util.h"
#include "IROperator.h"
#include "IRPrinter.h"

namespace Halide {

using namespace Internal;

using std::string;
using std::vector;

RVar::operator Expr() const {
    if (!min().defined() || !extent().defined()) {
        user_error << "Use of undefined RDom dimension: " <<
            (name().empty() ? "<unknown>" : name()) << "\n";
    }
    return Variable::make(Int(32), name(), domain());
}

Expr RVar::min() const {
    if (_domain.defined()) {
        return _var().min;
    } else {
        return Expr();
    }
}

Expr RVar::extent() const {
    if (_domain.defined()) {
        return _var().extent;
    } else {
        return Expr();
    }
}

const std::string &RVar::name() const {
    if (_domain.defined()) {
        return _var().var;
    } else {
        return _name;
    }
}


template <int N>
ReductionDomain build_domain(ReductionVariable (&vars)[N]) {
    vector<ReductionVariable> d(&vars[0], &vars[N]);
    ReductionDomain dom(d);
    return dom;
}

// This just initializes the predefined x, y, z, w members of RDom.
void RDom::init_vars(string name) {
    static string var_names[] = { "x", "y", "z", "w" };

    const std::vector<ReductionVariable> &dom_vars = dom.domain();
    RVar *vars[] = { &x, &y, &z, &w };

    for (size_t i = 0; i < sizeof(vars)/sizeof(vars[0]); i++) {
        if (i < dom_vars.size()) {
            *(vars[i]) = RVar(dom, i);
        } else {
            *(vars[i]) = RVar(name + "." + var_names[i]);
        }
    }
}

RDom::RDom(ReductionDomain d) : dom(d) {
    if (d.defined()) {
        init_vars("");
    }
}

// We suffix all RVars with $r to prevent unintentional name matches with pure vars called x, y, z, w.
RDom::RDom(Expr min, Expr extent, string name) {
    if (name == "") {
        name = make_entity_name(this, "Halide::RDom", 'r');
    }

    ReductionVariable vars[] = {
        { name + ".x$r", cast<int>(min), cast<int>(extent) },
    };
    dom = build_domain(vars);
    init_vars(name);
}

RDom::RDom(Expr min0, Expr extent0, Expr min1, Expr extent1, string name) {
    if (name == "") {
        name = make_entity_name(this, "Halide::RDom", 'r');
    }

    ReductionVariable vars[] = {
        { name + ".x$r", cast<int>(min0), cast<int>(extent0) },
        { name + ".y$r", cast<int>(min1), cast<int>(extent1) },
    };
    dom = build_domain(vars);
    init_vars(name);
}

RDom::RDom(Expr min0, Expr extent0, Expr min1, Expr extent1, Expr min2, Expr extent2, string name) {
    if (name == "") {
        name = make_entity_name(this, "Halide::RDom", 'r');
    }

    ReductionVariable vars[] = {
        { name + ".x$r", cast<int>(min0), cast<int>(extent0) },
        { name + ".y$r", cast<int>(min1), cast<int>(extent1) },
        { name + ".z$r", cast<int>(min2), cast<int>(extent2) },
    };
    dom = build_domain(vars);
    init_vars(name);
}

RDom::RDom(Expr min0, Expr extent0, Expr min1, Expr extent1, Expr min2, Expr extent2, Expr min3, Expr extent3,
           string name) {
    if (name == "") {
        name = make_entity_name(this, "Halide::RDom", 'r');
    }

    ReductionVariable vars[] = {
        { name + ".x$r", cast<int>(min0), cast<int>(extent0) },
        { name + ".y$r", cast<int>(min1), cast<int>(extent1) },
        { name + ".z$r", cast<int>(min2), cast<int>(extent2) },
        { name + ".w$r", cast<int>(min3), cast<int>(extent3) },
    };
    dom = build_domain(vars);
    init_vars(name);
}

RDom::RDom(Expr min0, Expr extent0, Expr min1, Expr extent1, Expr min2, Expr extent2, Expr min3, Expr extent3,
           Expr min4, Expr extent4, string name) {
    if (name == "") {
        name = make_entity_name(this, "Halide::RDom", 'r');
    }

    ReductionVariable vars[] = {
        { name + ".x$r", cast<int>(min0), cast<int>(extent0) },
        { name + ".y$r", cast<int>(min1), cast<int>(extent1) },
        { name + ".z$r", cast<int>(min2), cast<int>(extent2) },
        { name + ".w$r", cast<int>(min3), cast<int>(extent3) },
        { name + ".4$r", cast<int>(min4), cast<int>(extent4) },
    };
    dom = build_domain(vars);
    init_vars(name);
}

RDom::RDom(Expr min0, Expr extent0, Expr min1, Expr extent1, Expr min2, Expr extent2, Expr min3, Expr extent3,
           Expr min4, Expr extent4, Expr min5, Expr extent5, string name) {
    if (name == "") {
        name = make_entity_name(this, "Halide::RDom", 'r');
    }

    ReductionVariable vars[] = {
        { name + ".x$r", cast<int>(min0), cast<int>(extent0) },
        { name + ".y$r", cast<int>(min1), cast<int>(extent1) },
        { name + ".z$r", cast<int>(min2), cast<int>(extent2) },
        { name + ".w$r", cast<int>(min3), cast<int>(extent3) },
        { name + ".4$r", cast<int>(min4), cast<int>(extent4) },
        { name + ".5$r", cast<int>(min5), cast<int>(extent5) },
    };
    dom = build_domain(vars);
    init_vars(name);
}

RDom::RDom(Expr min0, Expr extent0, Expr min1, Expr extent1, Expr min2, Expr extent2, Expr min3, Expr extent3,
           Expr min4, Expr extent4, Expr min5, Expr extent5, Expr min6, Expr extent6, string name) {
    if (name == "") {
        name = make_entity_name(this, "Halide::RDom", 'r');
    }

    ReductionVariable vars[] = {
        { name + ".x$r", cast<int>(min0), cast<int>(extent0) },
        { name + ".y$r", cast<int>(min1), cast<int>(extent1) },
        { name + ".z$r", cast<int>(min2), cast<int>(extent2) },
        { name + ".w$r", cast<int>(min3), cast<int>(extent3) },
        { name + ".4$r", cast<int>(min4), cast<int>(extent4) },
        { name + ".5$r", cast<int>(min5), cast<int>(extent5) },
        { name + ".6$r", cast<int>(min6), cast<int>(extent6) },
    };
    dom = build_domain(vars);
    init_vars(name);
}

RDom::RDom(Expr min0, Expr extent0, Expr min1, Expr extent1, Expr min2, Expr extent2, Expr min3, Expr extent3,
           Expr min4, Expr extent4, Expr min5, Expr extent5, Expr min6, Expr extent6, Expr min7, Expr extent7,
           string name) {
    if (name == "") {
        name = make_entity_name(this, "Halide::RDom", 'r');
    }

    ReductionVariable vars[] = {
        { name + ".x$r", cast<int>(min0), cast<int>(extent0) },
        { name + ".y$r", cast<int>(min1), cast<int>(extent1) },
        { name + ".z$r", cast<int>(min2), cast<int>(extent2) },
        { name + ".w$r", cast<int>(min3), cast<int>(extent3) },
        { name + ".4$r", cast<int>(min4), cast<int>(extent4) },
        { name + ".5$r", cast<int>(min5), cast<int>(extent5) },
        { name + ".6$r", cast<int>(min6), cast<int>(extent6) },
        { name + ".7$r", cast<int>(min7), cast<int>(extent7) },
    };
    dom = build_domain(vars);
    init_vars(name);
}

RDom::RDom(Buffer b) {
    static string var_names[] = {"x$r", "y$r", "z$r", "w$r"};
    std::vector<ReductionVariable> vars;
    for (int i = 0; i < b.dimensions(); i++) {
        ReductionVariable var = {
            b.name() + "." + var_names[i],
            b.min(i),
            b.extent(i)
        };
        vars.push_back(var);
    }

    dom = ReductionDomain(vars);
    init_vars(b.name());
}

RDom::RDom(ImageParam p) {
    static string var_names[] = {"x$r", "y$r", "z$r", "w$r"};
    std::vector<ReductionVariable> vars;
    for (int i = 0; i < p.dimensions(); i++) {
        ReductionVariable var = {
            p.name() + "." + var_names[i],
            p.min(i),
            p.extent(i)
        };
        vars.push_back(var);
    }

    dom = ReductionDomain(vars);
    init_vars(p.name());
}


int RDom::dimensions() const {
    return (int)dom.domain().size();
}

RVar RDom::operator[](int i) const {
    if (i == 0) return x;
    if (i == 1) return y;
    if (i == 2) return z;
    if (i == 3) return w;
    if (i < dimensions()) {
        return RVar(dom, i);
    }
    user_error << "Reduction domain index out of bounds: " << i << "\n";
    return x; // Keep the compiler happy
}

RDom::operator Expr() const {
    if (dimensions() != 1) {
        user_error << "Error: Can't treat this multidimensional RDom as an Expr:\n"
                   << (*this) << "\n"
                   << "Only single-dimensional RDoms can be cast to Expr.\n";
    }
    return Expr(x);
}

RDom::operator RVar() const {
    if (dimensions() != 1) {
        user_error << "Error: Can't treat this multidimensional RDom as an RVar:\n"
                   << (*this) << "\n"
                   << "Only single-dimensional RDoms can be cast to RVar.\n";
    }
    return x;
}

/** Emit an RVar in a human-readable form */
std::ostream &operator<<(std::ostream &stream, RVar v) {
    stream << v.name() << "(" << v.min() << ", " << v.extent() << ")";
    return stream;
}

/** Emit an RDom in a human-readable form. */
std::ostream &operator<<(std::ostream &stream, RDom dom) {
    stream << "RDom(\n";
    for (int i = 0; i < dom.dimensions(); i++) {
        stream << "  " << dom[i] << "\n";
    }
    stream << ")\n";
    return stream;
}

}
