#ifndef MATRIX_H
#define MATRIX_H

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

///Helper class for matrix operations
struct Matrix
{
  ///Create a n_rows x n_cols sized matrix from a std::vector,
  ///used for easy initialization
  static const boost::numeric::ublas::matrix<double> CreateMatrix(
    const std::size_t n_rows,
    const std::size_t n_cols,
    const std::vector<double>& v);


  ///Create a uBLAS vector from a std::vector,
  ///used for easy initialization
  static const boost::numeric::ublas::vector<double> CreateVector(const std::vector<double>& v);

  ///Calculate the inverse of a matrix
  static const boost::numeric::ublas::matrix<double> Inverse(
    const boost::numeric::ublas::matrix<double>& m);

  ///Test these functions
  static void Test();

};

#endif // MATRIX_H
