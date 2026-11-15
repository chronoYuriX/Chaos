template <size_t CAPACITY>
struct R_TREE {
    static constexpr size_t LEAF_CAPACITY = CAPACITY * sizeof(NODE*) / sizeof(RECT);

    struct NODE {
        RECT  MBR;
        bool  is_leaf;
        NODE* parent;

        union {
            RECT  items[LEAF_CAPACITY];  // 叶子节点用
            NODE* sub_nodes[CAPACITY];   // 分支节点用
        };

        // 叶子节点：用空RECT作为终止符
        static bool is_empty(RECT r) {
            return r.right == 0 && r.bottom == 0;
        }

        // 获取叶子节点实际数量
        int leaf_count() const {
            int n = 0;
            while (n < LEAF_CAPACITY && !is_empty(items[n])) n++;
            return n;
        }

        // 获取分支节点实际数量
        int branch_count() const {
            int n = 0;
            while (n < CAPACITY && sub_nodes[n] != nullptr) n++;
            return n;
        }

        NODE(RECT box, bool leaf = true, NODE* p = nullptr)
            : MBR(box), is_leaf(leaf), parent(p) {
            ZeroMemory(&items, sizeof(items));
        }

        // 插入，返回新分裂的节点（如果有）
        NODE* insert(RECT box) {
            if (is_leaf) {
                int n = leaf_count();
                if (n < LEAF_CAPACITY) {
                    items[n] = box;
                    MBR = RECTOPS::getMBR(MBR, box);
                    return nullptr;
                }
                return split_leaf(box);
            }

            // 分支节点
            int n = branch_count();
            int best = 0;
            int best_gain = INT_MAX;
            for (int i = 0; i < n; i++) {
                int gain = RECTOPS::enlargement(sub_nodes[i]->MBR, box);
                if (gain < best_gain) {
                    best_gain = gain;
                    best = i;
                }
            }

            NODE* new_node = sub_nodes[best]->insert(box);
            if (new_node) {
                // 子节点分裂了
                if (n < CAPACITY) {
                    sub_nodes[n] = new_node;
                    new_node->parent = this;
                    update_mbr();
                    return nullptr;
                }
                return split_branch(new_node);
            }

            MBR = RECTOPS::getMBR(MBR, box);
            return nullptr;
        }

        NODE* split_leaf(RECT new_box) {
            NODE* new_leaf = new NODE(RECT{0}, true, parent);

            // 简单分裂：把后一半移到新叶子
            int n = leaf_count();
            int half = n / 2;
            for (int i = half; i < n; i++) {
                new_leaf->items[i - half] = items[i];
                items[i] = RECT{0};  // 清空
            }
            n = half;

            // 新矩形放入扩张小的
            int gain1 = RECTOPS::enlargement(MBR, new_box);
            int gain2 = RECTOPS::enlargement(new_leaf->MBR, new_box);
            if (gain1 <= gain2) {
                items[n] = new_box;
            } else {
                int m = new_leaf->leaf_count();
                new_leaf->items[m] = new_box;
            }

            update_mbr();
            new_leaf->update_mbr();

            return new_leaf;
        }

        NODE* split_branch(NODE* extra_child) {
            NODE* new_branch = new NODE(RECT{0}, false, parent);

            // 简单分裂：把后一半移到新分支
            int n = branch_count();
            int half = n / 2;
            for (int i = half; i < n; i++) {
                new_branch->sub_nodes[i - half] = sub_nodes[i];
                sub_nodes[i]->parent = new_branch;
                sub_nodes[i] = nullptr;
            }
            n = half;

            // 把 extra_child 放入扩张小的
            int gain1 = RECTOPS::enlargement(MBR, extra_child->MBR);
            int gain2 = RECTOPS::enlargement(new_branch->MBR, extra_child->MBR);
            if (gain1 <= gain2) {
                sub_nodes[n] = extra_child;
                extra_child->parent = this;
            } else {
                int m = new_branch->branch_count();
                new_branch->sub_nodes[m] = extra_child;
                extra_child->parent = new_branch;
            }

            update_mbr();
            new_branch->update_mbr();

            return new_branch;
        }

        void update_mbr() {
            if (is_leaf) {
                int n = leaf_count();
                if (n > 0) {
                    MBR = items[0];
                    for (int i = 1; i < n; i++)
                        MBR = RECTOPS::getMBR(MBR, items[i]);
                }
            } else {
                int n = branch_count();
                if (n > 0) {
                    MBR = sub_nodes[0]->MBR;
                    for (int i = 1; i < n; i++)
                        MBR = RECTOPS::getMBR(MBR, sub_nodes[i]->MBR);
                }
            }
        }

        // 查询
        void query(RECT q, RECT* results, int* count, int max) {
            if (!RECTOPS::intersect(MBR, q)) return;

            if (is_leaf) {
                int n = leaf_count();
                for (int i = 0; i < n && *count < max; i++) {
                    if (RECTOPS::intersect(items[i], q))
                        results[(*count)++] = items[i];
                }
            } else {
                int n = branch_count();
                for (int i = 0; i < n; i++)
                    sub_nodes[i]->query(q, results, count, max);
            }
        }
    };

    NODE* root;

    R_TREE() {
        root = new NODE(RECT{0}, true);
    }

    void insert(RECT box) {
        NODE* new_root = root->insert(box);
        if (new_root) {
            // 根节点分裂了，创建新根
            NODE* old_root = root;
            root = new NODE(RECT{0}, false);
            root->sub_nodes[0] = old_root;
            root->sub_nodes[1] = new_root;
            root->MBR = RECTOPS::getMBR(old_root->MBR, new_root->MBR);
            old_root->parent = root;
            new_root->parent = root;
        }
    }

    void query(RECT q, RECT* results, int* count, int max) {
        *count = 0;
        root->query(q, results, count, max);
    }
};
