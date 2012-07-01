#ifndef TRAVATAR_RULE_EXTRACTOR__
#define TRAVATAR_RULE_EXTRACTOR__

#include <boost/shared_ptr.hpp>
#include <travatar/hyper-graph.h>
#include <travatar/alignment.h>
#include <set>
#include <list>

namespace travatar {

// A virtual class to overload that converts a source parse, a target sentence
// and an alignment into a forest of matched rules
class RuleExtractor {
public:
    // Create the minimal graph of rules
    virtual HyperGraph * ExtractMinimalRules(
        HyperGraph & src_parse, 
        const Alignment & align) const = 0;

    // Take an edge that represents a rule and the corresponding target sentence
    // and return a string version of the rule
    std::string RuleToString(
        const HyperEdge & rule,
        const Sentence & src_sent,
        const Sentence & trg_sent) const;

private:
    // A function to help print rules recursively
    void PrintRuleSurface(const HyperNode & node,
                          const Sentence & src_sent,
                          std::list<HyperEdge*> & remaining_fragments,
                          int & tail_num,
                          std::ostream & oss) const;

};

// A rule extractor using forest based rule extraction
//  "Forest-based Translation Rule Extraction"
//  Haitao Mi and Liang Huang
class ForestExtractor : public RuleExtractor{
public:
    // Extract the very minimal set of rules (no nulls, etc)
    virtual HyperGraph * ExtractMinimalRules(
        HyperGraph & src_parse, 
        const Alignment & align) const;

    HyperGraph * AttachNullsTop(const HyperGraph & rule_graph,
                                const Alignment & align,
                                int trg_len);

protected:

    void AttachNullsTop(std::vector<bool> & nulls,
                        HyperNode & node);

};

}

#endif
