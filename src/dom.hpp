#pragma once
#include "cfg.hpp"

#include <vector>
#include <string>
#include <algorithm>

struct DomResult {
    std::vector<std::vector<bool>> dom;
};

inline std::vector<bool> set_all(int n, bool value) {
    return std::vector<bool>(n, value);
}

inline std::vector<bool> set_intersection(const std::vector<bool>& a,
                                          const std::vector<bool>& b) {

    std::vector<bool> out = a;
    for (int i = 0; i < (int)out.size(); ++i) out[i] = out[i] && b[i];
    return out;
}

inline bool set_equal(const std::vector<bool>& a,
                      const std::vector<bool>& b) {
    if (a.size() != b.size()) return false;
    for (int i = 0; i < (int)a.size(); ++i)
        if (a[i] != b[i]) return false;
    return true;
}
inline DomResult compute_dominators(const CFG& cfg) {
    const int n = (int)cfg.blocks.size();
    DomResult res;
    res.dom.resize(n);

    if (n == 0) return res;

    for (int b = 0; b < n; ++b) {
        if (b == cfg.entry) {
            res.dom[b] = set_all(n, false);
            res.dom[b][b] = true;
        } else {
            res.dom[b] = set_all(n, true);
        }
    }

    bool changed = true;
    while (changed) {
        changed = false;

        for (int b = 0; b < n; ++b) {
            if (b == cfg.entry) continue;

            const auto& preds = cfg.blocks[b].pred;
            std::vector<bool> newDom;

            if (preds.empty()) {
                newDom = set_all(n, false);
            } else {
                newDom = res.dom[preds[0]];
                for (int i = 1; i < (int)preds.size(); ++i) {
                    newDom = set_intersection(newDom, res.dom[preds[i]]);
                }
            }

            newDom[b] = true;

            if (!set_equal(newDom, res.dom[b])) {
                res.dom[b] = std::move(newDom);
                changed = true;
            }
        }
    }

    return res;
}

inline void dump_dominators(const CFG& cfg, const DomResult& dom) {
    const int n = (int)cfg.blocks.size();
    std::cout << "== Dominators ==\n";
    if (n == 0 || cfg.entry < 0) {
        std::cout << "(empty CFG)\n\n";
        return;
    }
    std::cout << "Entry: " << cfg.entry << " (" << cfg.blocks[cfg.entry].name << ")\n\n";

    for (int b = 0; b < n; ++b) {
        std::cout << "Dom(" << b << ":" << cfg.blocks[b].name << ") = { ";
        for (int x = 0; x < n; ++x) {
            if (dom.dom[b][x]) {
                std::cout << x << ":" << cfg.blocks[x].name << " ";
            }
        }
        std::cout << "}\n";
    }
    std::cout << "\n";
}
