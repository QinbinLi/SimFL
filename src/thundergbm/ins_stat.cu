//
// Created by shijiashuai on 5/7/18.
//
#include "thundergbm/ins_stat.h"

void InsStat::resize(size_t n_instances) {
    this->n_instances = n_instances;
    gh_pair.resize(n_instances);
    nid.resize(n_instances);
    y.resize(n_instances);
    y_predict.resize(n_instances);
}

void InsStat::updateGH() {
    sum_gh = GHPair(0, 0);
    GHPair *gh_pair_data = gh_pair.host_data();
    int *nid_data = nid.host_data();
    float_type *stats_y_data = y.host_data();
    float_type *stats_yp_data = y_predict.host_data();
    LOG(DEBUG) << y_predict;
    LOG(TRACE) << "initializing instance statistics";
    //TODO parallel?
    for (int i = 0; i < n_instances; ++i) {
        nid_data[i] = 0;
        //TODO support other objective function
        gh_pair_data[i].g = stats_yp_data[i] - stats_y_data[i];
        gh_pair_data[i].h = 1;
        sum_gh = sum_gh + gh_pair_data[i];
    }
}

void InsStat::updateGH(SyncArray<bool>& is_multi) {
    sum_gh = GHPair(0, 0);
    GHPair *gh_pair_data = gh_pair.host_data();
    int *nid_data = nid.host_data();
    float_type *stats_y_data = y.host_data();
    float_type *stats_yp_data = y_predict.host_data();
    bool* is_multi_data = is_multi.host_data();
    LOG(DEBUG) << y_predict;
    LOG(TRACE) << "initializing instance statistics";
    //TODO parallel?
    for (int i = 0; i < n_instances; ++i) {
        nid_data[i] = 0;
        //TODO support other objective function
        if(is_multi_data[i]) {
            gh_pair_data[i].g = 2 * stats_yp_data[i] - stats_y_data[i];
            gh_pair_data[i].h = 2;
        }
        else {
            gh_pair_data[i].g = stats_yp_data[i] - stats_y_data[i];
            gh_pair_data[i].h = 1;
        }
        sum_gh = sum_gh + gh_pair_data[i];
    }
}

void InsStat::updateGH(SyncArray<bool>& is_multi, int numP) {
    sum_gh = GHPair(0, 0);
    GHPair *gh_pair_data = gh_pair.host_data();
    int *nid_data = nid.host_data();
    float_type *stats_y_data = y.host_data();
    float_type *stats_yp_data = y_predict.host_data();
    bool* is_multi_data = is_multi.host_data();
    LOG(DEBUG) << y_predict;
    LOG(TRACE) << "initializing instance statistics";
    //TODO parallel?
    for (int i = 0; i < n_instances; ++i) {
        nid_data[i] = 0;
        //TODO support other objective function
        if(is_multi_data[i]) {
            gh_pair_data[i].g = numP * stats_yp_data[i] - stats_y_data[i]; // y is already multipled in similar_ins_bundle
            gh_pair_data[i].h = numP;

//            gh_pair_data[i].g = stats_yp_data[i] - stats_y_data[i];
//            gh_pair_data[i].h = 1;
        }
        else {
            gh_pair_data[i].g = stats_yp_data[i] - stats_y_data[i];
            gh_pair_data[i].h = 1;
        }
        sum_gh = sum_gh + gh_pair_data[i];
    }
}