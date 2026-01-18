#include "ir.hpp"
#include "cfg.hpp"
#include "dom.hpp"
#include <iostream>

static void dump_ir(const Program& p) {
    std::cout << "== Linear IR ==\n";
    for (int i = 0; i < (int)p.code.size(); ++i) {
        const auto& ins = p.code[i];
        std::cout << i << ": [" << kind_name(ins.kind) << "] " << ins.raw;

        if (ins.kind == Kind::Label && ins.labelDef) {
            std::cout << "   (def " << *ins.labelDef << ")";
        }
        if ((ins.kind == Kind::Cbr || ins.kind == Kind::JumpI) && ins.t1) {
            std::cout << "   (t1=" << *ins.t1;
            if (ins.t2) std::cout << ", t2=" << *ins.t2;
            std::cout << ")";
        }
        std::cout << "\n";
    }

    std::cout << "\n== Labels ==\n";
    std::vector<std::pair<int,std::string>> byPos;
    byPos.reserve(p.labelToIndex.size());
    for (const auto& [lab, idx] : p.labelToIndex) byPos.push_back({idx, lab});
    std::sort(byPos.begin(), byPos.end());
    for (const auto& [idx, lab] : byPos) {
        std::cout << lab << " -> instr " << idx << "\n";
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: cfg_week2 <file.iloc> [--dump-ir] [--dump-cfg] [--dump-dom]\n";
        return 2;
    }

    const std::string path = argv[1];
    bool do_ir = false;
    bool do_cfg = false;
    bool do_dom = false;

    for (int i = 2; i < argc; ++i) {
        std::string opt = argv[i];
        if (opt == "--dump-ir") do_ir = true;
        else if (opt == "--dump-cfg") do_cfg = true;
        else if (opt == "--dump-dom") do_dom = true;
        else {
            std::cerr << "Unknown option: " << opt << "\n";
            return 2;
        }
    }

    // default behavior if no flags
    if (!do_ir && !do_cfg && !do_dom) do_dom = true;

    try {
        Program p = parse_file(path);

        if (do_ir) {
            dump_ir(p);
            std::cout << "\n";
        }

        CFG cfg;
        if (do_cfg || do_dom) {
            cfg = build_cfg(p);
        }

        if (do_cfg) {
            dump_cfg(p, cfg);
        }

        if (do_dom) {
            auto dom = compute_dominators(cfg);
            dump_dominators(cfg, dom);
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
