#pragma once
#include "ir.hpp"

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <unordered_set>

// ---------------- CFG model ----------------

struct Block {
    int id = -1;
    std::string name;
    int start = -1;
    int end = -1;
    std::vector<int> succ;
    std::vector<int> pred;
};

struct CFG {
    std::vector<Block> blocks;
    int entry = 0;
};

// ---------------- Helpers ----------------

inline bool is_terminator(const Instr& ins) {
    return ins.kind == Kind::Cbr || ins.kind == Kind::JumpI || ins.kind == Kind::Ret;
}

// 1) first instruction
// 2) any label instruction
// 3) instruction after terminator (if exists)
inline std::vector<int> find_leaders(const Program& p) {
    std::unordered_set<int> leaders;
    if (!p.code.empty()) leaders.insert(0);

    for (int i = 0; i < (int)p.code.size(); ++i) {
        if (p.code[i].kind == Kind::Label) leaders.insert(i);
        if (is_terminator(p.code[i]) && i + 1 < (int)p.code.size()) leaders.insert(i + 1);
    }
    std::vector<int> out(leaders.begin(), leaders.end());
    std::sort(out.begin(), out.end());
    return out;
}

inline std::vector<int> build_inst_to_block(const CFG& cfg, int nInst) {
    std::vector<int> instToBlock(nInst, -1);

    for (const auto& b : cfg.blocks) {
        for (int i = b.start; i <= b.end; ++i) instToBlock[i] = b.id;
    }
    return instToBlock;
}

inline void add_edge(CFG& cfg, int from, int to) {
    if (from < 0 || to < 0) return;
    auto& s = cfg.blocks[from].succ;
    if (std::find(s.begin(), s.end(), to) == s.end()) s.push_back(to);
    auto& p = cfg.blocks[to].pred;
    if (std::find(p.begin(), p.end(), from) == p.end()) p.push_back(from);
}

inline int label_to_block(const Program& p,
                          const std::vector<int>& instToBlock,
                          const std::string& label) {
    auto it = p.labelToIndex.find(label);
    if (it == p.labelToIndex.end()) return -1;
    int instIdx = it->second;
    if (instIdx < 0 || instIdx >= (int)instToBlock.size()) return -1;
    return instToBlock[instIdx];
}

inline CFG build_cfg(const Program& p) {
    CFG cfg;
    
    if(p.code.empty()) {
        cfg.entry = -1;
        return cfg;    
    }

    auto leaders = find_leaders(p);
    if (leaders.empty()) {
        cfg.entry = -1;
        return cfg;
    }

    cfg.blocks.reserve(leaders.size());
    for (int bi = 0; bi < (int)leaders.size(); ++bi) {
        int start = leaders[bi];
        int end = (bi + 1 < (int)leaders.size()) ? (leaders[bi + 1] - 1) : ((int)p.code.size() - 1);

        Block b;
        b.id = bi;
        b.start = start;
        b.end = end;

        if (p.code[start].kind == Kind::Label && p.code[start].labelDef) {
            b.name = *p.code[start].labelDef;
        } else { 
            b.name = "B" + std::to_string(bi);
        }
        cfg.blocks.push_back(std::move(b));
    }
    cfg.entry = 0;
    auto instToBlock = build_inst_to_block(cfg, (int)p.code.size());

    for (auto& b : cfg.blocks) {
        int termIdx = b.end;
        while (termIdx >= b.start && p.code[termIdx].kind == Kind::Label) termIdx--;
        if (termIdx < b.start) continue;

        const auto& term = p.code[termIdx];
        if (term.kind == Kind::Cbr) {
            int t1 = label_to_block(p, instToBlock, *term.t1);
            int t2 = label_to_block(p, instToBlock, *term.t2);
            add_edge(cfg, b.id, t1);
            add_edge(cfg, b.id, t2);
        } else if (term.kind == Kind::JumpI) {
            int t1 = label_to_block(p, instToBlock, *term.t1);
            add_edge(cfg, b.id, t1);
        } else if (term.kind == Kind::Ret) {

        } else {}
    }
    return cfg;
}

inline void dump_cfg(const Program& p, const CFG& cfg) {
    std::cout << "== CFG ==\n";
    std::cout << "Blocks: " << cfg.blocks.size() << ", entry: " << cfg.entry << "\n\n";

    if (cfg.blocks.empty()) {
        std::cout << "(empty CFG)\n\n";
        return;
    }
    
    for (const auto& b : cfg.blocks) {
        std::cout << "Block " << b.id << " (" << b.name << ")"
                  << " [" << b.start << ", " << b.end << "]\n";

        for (int i = b.start; i <= b.end; ++i) {
            std::cout << "  " << i << ": " << p.code[i].raw << "\n";
        }

        std::cout << "  succ: ";
        for (int s : b.succ) std::cout << s << "(" << cfg.blocks[s].name << ") ";
        std::cout << "\n  pred: ";
        for (int pr : b.pred) std::cout << pr << "(" << cfg.blocks[pr].name << ") ";
        std::cout << "\n\n";
    }
}