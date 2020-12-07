//
// Created by jiashuai on 18-1-18.
//

#ifndef THUNDERGBM_TREE_H
#define THUNDERGBM_TREE_H

#include "thundergbm/thundergbm.h"
#include "syncarray.h"
#include "sstream"
#include "ins_stat.h"


class Tree {
public:
    struct TreeNode {
        int final_id;// node id after pruning, may not equal to node index
        int lch_index;// index of left child
        int rch_index;// index of right child
        int parent_index;// index of parent node
        float_type gain;// gain of splitting this node
        float_type base_weight;
        int split_feature_id;
        float_type split_value;
        bool default_right;
        bool is_leaf;
        bool is_valid;// non-valid nodes are those that are "children" of leaf nodes
        bool is_pruned;// pruned after pruning

        GHPair sum_gh_pair;

        friend std::ostream &operator<<(std::ostream &os,
                                        const TreeNode &node);

        HOST_DEVICE void calc_weight(float_type lambda) {
            this->base_weight = -sum_gh_pair.g / (sum_gh_pair.h + lambda);
        }

        HOST_DEVICE bool splittable() const {
            return !is_leaf && is_valid;
        }

    };

    explicit Tree(int depth);

    Tree() = default;

    Tree(const Tree &tree) {
        nodes.resize(tree.nodes.size());
        nodes.copy_from(tree.nodes);
    }

    Tree &operator=(const Tree &tree) {
        nodes.resize(tree.nodes.size());
        nodes.copy_from(tree.nodes);
        return *this;
    }

    void init(int depth);

    string dump(int depth) const;

    SyncArray<Tree::TreeNode> nodes;

    void prune_self(float_type gamma);

    void shrink(float_type learning_rate);

private:
    void preorder_traversal(int nid, int max_depth, int depth, string &s) const;

    int try_prune_leaf(int nid, int np, float_type gamma, vector<int> &leaf_child_count);

    void reorder_nid();
};

#endif //THUNDERGBM_TREE_H
