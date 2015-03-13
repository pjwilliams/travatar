#include <iostream>
#include <fstream>
#include <travatar/global-debug.h>
#include <travatar/tree-io.h>
#include <travatar/tree-converter-runner.h>
#include <travatar/config-tree-converter-runner.h>
#include <travatar/binarizer-directional.h>
#include <travatar/binarizer-cky.h>
#include <travatar/unary-flattener.h>
#include <travatar/word-splitter-compound.h>
#include <travatar/word-splitter-regex.h>
#include <travatar/caser.h>
#include <travatar/hyper-graph.h>
#include <travatar/dict.h>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

using namespace travatar;
using namespace std;
using namespace boost;

// Run the model
void TreeConverterRunner::Run(const ConfigTreeConverterRunner & config) {

    // Set the debugging level
    GlobalVars::debug = config.GetInt("debug");

    // Create the tree input
    scoped_ptr<TreeIO> tree_in, tree_out;
    if(config.GetString("input_format") == "penn")
        tree_in.reset(new PennTreeIO);
    else if(config.GetString("input_format") == "egret") {
        EgretTreeIO *p = new EgretTreeIO();
        if (config.GetBool("no_egret_weight_normalization")) {
          p->SetNormalize(false);
        }
        tree_in.reset(p);
    } else if(config.GetString("input_format") == "json")
        tree_in.reset(new JSONTreeIO);
    else if(config.GetString("input_format") == "rule")
        tree_in.reset(new RuleTreeIO);
    else if(config.GetString("input_format") == "word")
        tree_in.reset(new WordTreeIO);
    else
        THROW_ERROR("Invalid -input_format type: " << config.GetString("input_format"));
    // Create the tree output
    bool phrase_out = config.GetString("output_format") == "phrase";
    if(config.GetString("output_format") == "penn")
        tree_out.reset(new PennTreeIO);
    else if(config.GetString("output_format") == "egret")
        tree_out.reset(new EgretTreeIO);
    else if(config.GetString("output_format") == "json")
        tree_out.reset(new JSONTreeIO);
    else if(config.GetString("output_format") == "mosesxml")
        tree_out.reset(new MosesXMLTreeIO);
    else if(config.GetString("output_format") == "rule")
        tree_out.reset(new RuleTreeIO);
    else if(config.GetString("output_format") == "word")
        tree_out.reset(new WordTreeIO);
    else
        THROW_ERROR("Invalid -output_format type: " << config.GetString("output_format"));

    // Create the splitter
    scoped_ptr<GraphTransformer> splitter;
    if(config.GetString("split").size() != 0) {
        splitter.reset(new WordSplitterRegex(config.GetString("split")));
    }

    // Create the compound splitter
    scoped_ptr<GraphTransformer> compoundsplitter;
    if(config.GetString("compoundsplit").size() != 0) {
      compoundsplitter.reset(new WordSplitterCompound(config.GetString("compoundsplit"),
						      config.GetInt("compoundsplit_minchar"),
						      config.GetReal("compoundsplit_threshold"),
						      config.GetString("compoundsplit_filler")));
    }

    // Create the binarizer
    scoped_ptr<GraphTransformer> binarizer;
    binarizer.reset(Binarizer::CreateBinarizerFromString(config.GetString("binarize")));

    // Create the flattener
    scoped_ptr<GraphTransformer> flattener;
    if(config.GetBool("flatten")) {
        flattener.reset(new UnaryFlattener());
    }

    // Create the caser
    scoped_ptr<GraphTransformer> caser;
    caser.reset(Caser::CreateCaserFromString(config.GetString("case")));

    // Open the file (or cin)
    const vector<string> & argv = config.GetMainArgs();
    istream * src_in;
    if(argv.size() == 1 && argv[0] != "-") {
        src_in = new ifstream(argv[0].c_str());
        if(!src_in) THROW_ERROR("Could not find src file: " << argv[0]);
    } else {
        src_in = &cin;
    }

    // Get the lines
    string src_line;
    int sent = 0;
    cerr << "Transforming trees (.=10,000, !=100,000 sentences)" << endl;
    try {
    while(true) {
        // Parse into the appropriate data structures
        boost::shared_ptr<HyperGraph> src_graph(tree_in->ReadTree(*src_in));
        if(src_graph.get() == NULL) break;
        // { /* DEBUG */ JSONTreeIO io; io.WriteTree(*src_graph, cerr); cerr << endl; }

        // Splitter if necessary
        if(splitter.get() != NULL)
            src_graph.reset(splitter->TransformGraph(*src_graph));
        // Compound Splitter if necessary
        if(compoundsplitter.get() != NULL)
            src_graph.reset(compoundsplitter->TransformGraph(*src_graph));
        // Binarizer if necessary
        if(binarizer.get() != NULL)
            src_graph.reset(binarizer->TransformGraph(*src_graph));
        // Flattener if necessary
        if(flattener.get() != NULL)
            src_graph.reset(flattener->TransformGraph(*src_graph));
        // Caser if necessary
        if(caser.get() != NULL)
            src_graph.reset(caser->TransformGraph(*src_graph));
        // { /* DEBUG */ JSONTreeIO io; io.WriteTree(*src_graph, cerr); cerr << endl; }
        // Write out the tree
        if(tree_out.get() != NULL) {
            tree_out->WriteTree(*src_graph, cout); cout << endl;
        } else if (phrase_out) {
            THROW_ERROR("Phrases not implemented yet");
        } else {
            THROW_ERROR("Unknown output format");
        }
        sent++;
        if(sent % 10000 == 0) {
            cerr << (sent % 100000 == 0 ? '!' : '.'); cerr.flush();
        }
    }
    } catch (std::runtime_error & e) {
        THROW_ERROR("Error reading tree on line " << sent+1 << endl << src_line << endl << e.what());
    }
    cerr << endl;
}
