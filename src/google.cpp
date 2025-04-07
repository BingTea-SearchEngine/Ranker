#include "static/google.h"

GoogleMatrix::GoogleMatrix() {}

GoogleMatrix::GoogleMatrix(const std::string& filename) {}

GoogleMatrix::GoogleMatrix(const unsigned n_in, unsigned** from_to_in, 
                           unsigned* out_degrees_in)
  : n(n_in), from_to(from_to_in), page_ranks(n_in) {
    full_row = new unsigned[n];
    page_ranks.shrink_to_fit();
    for (unsigned i = 0; i < n; ++i) {
        full_row[i] = i;
        quicksort(from_to[i], 0, out_degrees[i] - 1);
        page_ranks[i] = 1 / n;
    }
    convert_google();
}

GoogleMatrix::GoogleMatrix(unsigned const n_in, unsigned const m,
                           unsigned* first_in, unsigned* second_in)
  : n(n_in), page_ranks(n_in) {
    full_row = new unsigned[n];
    page_ranks.shrink_to_fit();
    for (unsigned i = 0; i < n; ++i)
        full_row[i] = i;
    
    from_to = new unsigned*[n];
    out_degrees = new unsigned[n];

    for (unsigned i = 0; i < n; ++i) {
        out_degrees[i] = 0;
    }

    for (unsigned i = 0; i < m; ++i) {
        out_degrees[first_in[i]]++;
    }

    for (unsigned i = 0; i < n; ++i) {
        from_to[i] = new unsigned[out_degrees[i]];
        out_degrees[i] = 0;
    }

    for (unsigned i = 0; i < m; ++i) {
        unsigned node1 = first_in[i];
        unsigned node2 = second_in[i];
        from_to[node1][out_degrees[node1]++] = node2;
    }

    for (unsigned i = 0; i < n; ++i) {
        // This is multithreadable. No overlapping memory accesses.
        quicksort(from_to[i], 0, out_degrees[i] - 1);
        page_ranks[i] = 1. / n;
    }
    convert_google();
    std::cout << "INITIAL" << std::endl;
    for (unsigned i = 0; i < page_ranks.size(); ++i) {
        std::cout << page_ranks[i] << ' ';
    }
    std::cout << std::endl;
}

void GoogleMatrix::convert_google() {
    for (unsigned i = 0; i < n; ++i) {
        if (out_degrees[i] == 0) {
            delete[] from_to[i];
            out_degrees[i] = n;
            from_to[i] = full_row;
            // Find a way to bitmap each row that uses full_row to
            // prevent double frees in the destructor
        }
    }
}

void GoogleMatrix::iteration(double damping) {
    std::cout << "START" << std::endl;
    for (unsigned i = 0; i < page_ranks.size(); ++i) {
        std::cout << page_ranks[i] << ' ';
    }
    std::cout << std::endl;
    Vector<double> original_page_ranks = page_ranks;
    for (unsigned i = 0; i < n; ++i) {
        page_ranks[i] = 0;
    }

    for (unsigned i = 0; i < n; ++i) {
        double contribution = original_page_ranks[i] / out_degrees[i];
        for (unsigned j = 0; j < out_degrees[i]; ++j) {
            unsigned node = from_to[i][j];
            page_ranks[node] += contribution;
        }
    }

    for (unsigned i = 0; i < n; ++i) {
        page_ranks[i] *= damping;
        page_ranks[i] += (1 - damping) / n;
    }
    std::cout << "END" << std::endl;
    for (unsigned i = 0; i < page_ranks.size(); ++i) {
        std::cout << page_ranks[i] << ' ';
    }
    std::cout << std::endl;
}

Vector<double> GoogleMatrix::pagerank(double damping) {
    print(true);
    for (unsigned i = 0; i < 50; ++i) {
        iteration(damping);
    }
    return page_ranks;
}

void GoogleMatrix::print(bool adjacency) {
    if (adjacency) {
        std::cout << "adjacency matrix:\n";
        for (unsigned i = 0; i < n; ++i) {
            unsigned next = -1;
            unsigned counter = 0;
            if (out_degrees[i] != 0) {
                next = from_to[i][counter];
            }
            for (unsigned j = 0; j < n; ++j) {
                if (j == next) {
                    std::cout << 1. / out_degrees[i] << ' ';
                    counter++;
                    if (counter < out_degrees[i])
                        next = from_to[i][counter];
                }
                else
                    std::cout << "0 ";
            }
            std::cout << '\n';
        }
    }
}

void GoogleMatrix::read_bitstream(const std::string& filename) {
    unsigned** raw_rows = nullptr;
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) {
        // handle error
        throw std::runtime_error("file does not exist, is empty, or is badly formatted");
    }

    Vector<unsigned*> rows;
    Vector<unsigned> degrees;
    while (ifs.peek() != EOF) {
        Vector<unsigned> row;
        unsigned value;
        // Read until sentinel is encountered
        while (ifs.read(reinterpret_cast<char*>(&value), sizeof(value))) {
            if (value == -1) {
                break;
            }
            row.push_back(value);
            
        }
        degrees.push_back(row.size());
        unsigned* raw_row = new unsigned[row.size()];
        for (unsigned i = 0; i < row.size(); ++i) {
            raw_row[i] = row[i];
        }
        rows.push_back(raw_row);
    }
    
    n = rows.size();
    from_to = new unsigned*[n];
    out_degrees = new unsigned[n];
    for (unsigned i = 0; i < n; ++i) {
        from_to[i] = rows[i];
        out_degrees[i] = degrees[i];
    }
}

void GoogleMatrix::read_txt(const std::string& filename) {

}

void GoogleMatrix::save(const std::string& filename) {
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        // handle error
        return;
    }
    for (unsigned i = 0; i < n; ++i) {
        auto& row = from_to[i];
        // Write each integer in the row
        for (unsigned j = 0; j < out_degrees[i]; ++j) {
            const auto& value = row[j];
            ofs.write(reinterpret_cast<const char*>(&value), sizeof(value));
        }
        // Write sentinel
        unsigned sentinel = -1;
        ofs.write(reinterpret_cast<const char*>(&sentinel), sizeof(sentinel));
    }
}