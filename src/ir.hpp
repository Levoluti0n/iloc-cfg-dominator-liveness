#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <optional>
#include <stdexcept>
#include <unordered_map>

// ---------- IR model ----------

enum class Kind {
    Label,   // "L0:"
    Cbr,     // "cbr r3 -> L1, L2"
    JumpI,   // "jumpI -> L0"
    Ret,     // "return"
    Other    // everything else (kept as raw)
};

struct Instr {
    Kind kind = Kind::Other;
    std::string raw;
    std::optional<std::string> labelDef;
    std::optional<std::string> t1, t2;
};

struct Program {
    std::vector<Instr> code;
    std::unordered_map<std::string, int> labelToIndex;
};

// ---------- small helpers ----------

inline std::string trim(std::string s) {
    auto notSpace = [](unsigned char c) {return !std::isspace(c);};
    while(!s.empty() && !notSpace((unsigned char)s.front())) s.erase(s.begin());
    while(!s.empty() && !notSpace((unsigned char)s.back())) s.pop_back();
    return s;
} 

inline bool starts_with(const std::string& s, const std::string& pref) {
    return s.size() >= pref.size() && s.compare(0, pref.size(), pref) == 0;
}

inline std::string strip_comment(std::string line) {
    auto pos_hash  = line.find('#');
    auto pos_slash = line.find("//");

    std::size_t pos = std::string::npos;

    if (pos_hash != std::string::npos)
        pos = pos_hash;

    if (pos_slash != std::string::npos)
        pos = (pos == std::string::npos) ? pos_slash : std::min(pos, pos_slash);

    if (pos != std::string::npos)
        line.erase(pos);

    return line;
}

inline std::runtime_error parse_error(int lineNo, const std::string& msg) {
    return std::runtime_error("Parse error at line " + std::to_string(lineNo) + ": " + msg);
}

inline std::optional<std::string> parse_label_def(const std::string& line) {
    if(!line.empty() && line.back() == ':' && line.find(' ') == std::string::npos) {
        return line.substr(0, line.size() - 1);
    }
    return std::nullopt;
}

inline std::string rhs_after_arrow(const std::string& line) {
    auto pos = line.find("->");
    if(pos == std::string::npos) return {}; 
    return trim(line.substr(pos + 2));
}

inline Program parse_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Cannot open file: " + path);

    Program p;
    std::string line;
    int lineNo{};

    while(std::getline(in, line)) {
        ++lineNo;
        line = strip_comment(line);
        line = trim(line);

        if(line.empty()) continue;

        Instr inst;
        inst.raw = line;

        if(auto lab = parse_label_def(line)) {
            inst.kind = Kind::Label;
            inst.labelDef = *lab;

            if(p.labelToIndex.count(*lab)) {
                throw parse_error(lineNo, "duplicate label: " + *lab);
            }
            p.labelToIndex[*lab] = (int)p.code.size();
            p.code.push_back(std::move(inst));
            continue;
        }

        if(starts_with(line, "cbr")) {
            inst.kind = Kind::Cbr;

            std::string rhs = rhs_after_arrow(line);
            if(rhs.empty()) throw parse_error(lineNo, "cbr missing '-> targets'");

            auto comma = rhs.find(',');
            if (comma == std::string::npos) throw parse_error(lineNo, "cbr needs two targets: '-> L1, L2'");

            inst.t1 = trim(rhs.substr(0, comma));
            inst.t2 = trim(rhs.substr(comma + 1));
            if (!inst.t1->empty() && inst.t1->back() == ',') inst.t1 = trim(inst.t1->substr(0, inst.t1->size() - 1));
        } else if(starts_with(line, "jumpI")) {
            inst.kind = Kind::JumpI;

            std::string rhs = rhs_after_arrow(line);
            if(rhs.empty()) throw parse_error(lineNo, "jumpI missing target: '-> Lx'");
            inst.t1 = trim(rhs);
        } else if(starts_with(line, "return")) {
            inst.kind = Kind::Ret;
        } else {
            inst.kind = Kind::Other; 
        }

        p.code.push_back(std::move(inst));
    }
    
    for (int i = 0; i < (int)p.code.size(); ++i) {
        const auto& inst = p.code[i];
        if (inst.kind == Kind::Cbr) {
            if (!inst.t1 || !inst.t2) throw std::runtime_error("Internal: cbr missing targets");
            if (!p.labelToIndex.count(*inst.t1)) throw std::runtime_error("Unknown label target: " + *inst.t1);
            if (!p.labelToIndex.count(*inst.t2)) throw std::runtime_error("Unknown label target: " + *inst.t2);
        } else if (inst.kind == Kind::JumpI) {
            if (!inst.t1) throw std::runtime_error("Internal: jumpI missing target");
            if (!p.labelToIndex.count(*inst.t1)) throw std::runtime_error("Unknown label target: " + *inst.t1);
        }
    }
    return p;
}


inline const char* kind_name(Kind k) {
    switch (k) {
        case Kind::Label: return "Label";
        case Kind::Cbr:   return "Cbr";
        case Kind::JumpI: return "JumpI";
        case Kind::Ret:   return "Return";
        default:          return "Other";
    }
}