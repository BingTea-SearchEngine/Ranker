#include "static/google.h"

GoogleMatrix::GoogleMatrix()
  : SparseNetwork() {}

GoogleMatrix::GoogleMatrix(const std::string& filename)
  : SparseNetwork(filename) {}

GoogleMatrix::GoogleMatrix(const unsigned n_in, const unsigned m_in, 
                           unsigned** from_to_in, unsigned* out_degrees_in)
  : SparseNetwork(n_in, m_in, from_to_in, out_degrees_in) {}
  
GoogleMatrix::GoogleMatrix(unsigned const n_in, unsigned const m_in,
                           unsigned* first_in, unsigned* second_in)
  : SparseNetwork(n_in, m_in, first_in, second_in) {}