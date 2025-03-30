class SparseNetwork {
    private:
        int n;
        int* first;
        int* second;
    public:
        SparseNetwork();
        SparseNetwork(int num_edges, int* first, int* second);
};