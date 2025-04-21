#include "static/google.h"

GoogleMatrix::GoogleMatrix() {}

GoogleMatrix::GoogleMatrix(const std::string& filename) {
    read_bitstream(filename);
    page_ranks = Vector<double>(n);
    page_ranks.shrink_to_fit();
    for (unsigned i = 0; i < n; ++i) {
        page_ranks[i] = 1. / n;
    }
    unsigned map_size = (n + (8 * sizeof(char)) - 1) / (8 * sizeof(char));
    delete_map = new char[map_size];
    for (unsigned i = 0; i < map_size; ++i)
        delete_map[i] = 0;
    full_row = new unsigned[n];
    for (unsigned i = 0; i < n; ++i)
        full_row[i] = i;
    convert_google();
}

GoogleMatrix::GoogleMatrix(const unsigned n_in, unsigned** from_to_in, 
                           unsigned* out_degrees_in)
  : n(n_in), from_to(from_to_in), out_degrees(out_degrees_in), page_ranks(n_in), delete_from_to(false) {
    page_ranks.shrink_to_fit();
    unsigned map_size = (n + (8 * sizeof(char)) - 1) / (8 * sizeof(char));
    delete_map = new char[map_size];
    for (unsigned i = 0; i < map_size; ++i) {
        delete_map[i] = 0;
    }
    full_row = new unsigned[n];
    for (unsigned i = 0; i < n; ++i) {
        full_row[i] = i;
        quicksort(from_to[i], 0, out_degrees[i] - 1);
        page_ranks[i] = 1. / n;
    }
    convert_google();
}

GoogleMatrix::GoogleMatrix(unsigned const n_in, unsigned const m,
                           unsigned* first_in, unsigned* second_in)
  : n(n_in), page_ranks(n_in) {
    page_ranks.shrink_to_fit();
    unsigned map_size = (n + (8 * sizeof(char)) - 1) / (8 * sizeof(char));
    delete_map = new char[map_size];
    for (unsigned i = 0; i < map_size; ++i) {
        delete_map[i] = 0;
    }
    full_row = new unsigned[n];
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

GoogleMatrix::~GoogleMatrix() {
    if (delete_from_to) {
        for (unsigned i = 0; i < n; ++i) {
            if (delete_map[i / (8 * sizeof(char))] & (1 << (i % (8 * sizeof(char)))))
                continue;
            delete[] from_to[i];
        }
        delete[] from_to;
        delete[] out_degrees;
    }
    delete[] full_row;
    delete[] delete_map;
}

void GoogleMatrix::convert_google() {
    for (unsigned i = 0; i < n; ++i) {
        if (out_degrees[i] == 0) {
            delete[] from_to[i];
            if (delete_from_to)
                from_to[i] = full_row;
            else {
                from_to[i] = new unsigned[n];
                for (unsigned j = 0; j < n; ++j)
                    from_to[i][j] = j;
            }
            out_degrees[i] = n;
            auto asdf = i / (8 * sizeof(char));
            delete_map[asdf] += 1 << (i % (8 * sizeof(char)));
            // Find a way to bitmap each row that uses full_row to
            // prevent double frees in the destructor
        }
    }
}

void GoogleMatrix::iteration(double damping) {
    Vector<double> original_page_ranks = page_ranks;
    for (unsigned i = 0; i < n; ++i) {
        page_ranks[i] = 0;
    }

    double global_contribution = 0;
    for (unsigned i = 0; i < n; ++i) {
        if (out_degrees[i] == n) {
            global_contribution += original_page_ranks[i];
        }
        else {
            double contribution = original_page_ranks[i] / out_degrees[i];
            for (unsigned j = 0; j < out_degrees[i]; ++j) {
                unsigned node = from_to[i][j];
                page_ranks[node] += contribution;
            }
        }
        if (i % 1000000 == 0)
            std::cout << i << std::endl;
    }

    global_contribution /= n;
    for (unsigned i = 0; i < n; ++i) {
        page_ranks[i] += global_contribution;
    }

    for (unsigned i = 0; i < n; ++i) {
        page_ranks[i] *= damping;
        page_ranks[i] += (1 - damping) / n;
    }
}

Vector<double> GoogleMatrix::pagerank(double damping) {
    std::cout << "Starting" << std::endl;
    for (unsigned i = 0; i < 75; ++i) {
        iteration(damping);
        std::cout << i << std::endl;
    }
    return page_ranks;
}

void GoogleMatrix::print(bool adjacency) {
    if (adjacency) {
        std::cout << "adjacency matrix:\n";
        for (unsigned i = 0; i < n; ++i) {
            unsigned next = -1;
            unsigned counter = 0;
            if (out_degrees[i] != 0)
                next = from_to[i][counter];
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

void GoogleMatrix::dump(const std::string& filename) {
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