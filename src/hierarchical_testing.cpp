// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-

#include <RcppArmadillo.h>
#include <lemon/list_graph.h>
#include <lemon/dfs.h>
#include <lemon/lgf_reader.h>
#include <lemon/adaptors.h>
#include <iostream>
#include <fstream>
#include <string>
#include "dagger.h"

using namespace Rcpp;
using namespace RcppArmadillo;
using namespace lemon;
using namespace std;
// [[Rcpp::depends(RcppArmadillo)]]
// [[Rcpp::plugins(cpp11)]]

// count/print nodes
// [[Rcpp::export]]
arma::mat test_hierarchically(std::string filepath, double alpha, arma::colvec prob_theta_equals_zero) {
    ListDigraph gr;
    ListDigraph::NodeMap<int> layer(gr);
    ListDigraph::NodeMap<int> label(gr);
    ListDigraph::NodeMap<int> depth(gr);
    std::fstream infile;
    infile.open(filepath);
    int max_depth;

    // Read in Directed Graph from lgf.txt
    //  dim gives which "layer" the given node lies in
    //  max_depth is the number of layers in the graph
    digraphReader(gr, infile)
        .nodeMap("label", label)
        .nodeMap("layer", layer)
        .attribute("max_depth", max_depth)
        .run();
    
    // Create a dagger object from the graph and observed probabilites
    Dagger dagger(gr, layer, label, prob_theta_equals_zero, alpha, max_depth);

    // Run the testing procedure
    int ncan;
    int num_currently_rejected = dagger.num_currently_rejected();

    // For each layer, beginning at the root
    for(auto d = 0; d < max_depth; d++) {
        ncan = dagger.num_currently_candidates();
        if(ncan > 0) {
            dagger.test_hypothesis_at_current_layer();
            num_currently_rejected = dagger.num_currently_rejected();
        }
    }

    // Matrix whose first column is node label,
    //  and second column is whether or not that node is rejected
    arma::mat out(dagger.total_nodes(), 2, arma::fill::zeros);
    for(ListDigraph::NodeIt n(gr); n != INVALID; ++n){
        out(dagger.label(n)-1, 0) = dagger.label(n);
        out(dagger.label(n)-1, 1) = dagger.is_rejected(n);
    }

    return out;
}
