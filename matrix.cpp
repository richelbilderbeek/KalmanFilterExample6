#ifdef _WIN32
#undef __STRICT_ANSI__
#endif

#include "matrix.h"

#include <iostream>
#include <boost/numeric/ublas/io.hpp>

const boost::numeric::ublas::matrix<double> Matrix::CreateMatrix(
  const std::size_t n_rows,
  const std::size_t n_cols,
  const std::vector<double>& v)
{
  assert(n_rows * n_cols == v.size());
  boost::numeric::ublas::matrix<double> m(n_rows,n_cols);
  for (std::size_t row = 0; row!=n_rows; ++row)
  {
    for (std::size_t col = 0; col!=n_cols; ++col)
    {
      m(row,col) = v[ (col * n_rows) + row];
    }
  }
  return m;
}

const boost::numeric::ublas::vector<double> Matrix::CreateVector(const std::vector<double>& v)
{
  boost::numeric::ublas::vector<double> w(v.size());
  std::copy(v.begin(),v.end(),w.begin());
  return w;
}

const boost::numeric::ublas::matrix<double> Matrix::Inverse(
  const boost::numeric::ublas::matrix<double>& m)
{
  assert(m.size1() == m.size2() && "Can only calculate the inverse of square matrices");
  switch(m.size1())
  {
    case 1:
    {
      assert(m.size1() == 1 && m.size2() == 1 && "Only for 1x1 matrices");
      assert(m(0,0) != 0.0 && "Cannot take the inverse of matrix [0]");
      boost::numeric::ublas::matrix<double> n(1,1);
      n(0,0) =  1.0 / m(0,0);
      return n;
    }
    case 2:
    {
      assert(m.size1() == 2 && m.size2() == 2 && "Only for 2x2 matrices");
      const double a = m(0,0);
      const double b = m(0,1);
      const double c = m(1,0);
      const double d = m(1,1);
      const double determinant = (a * d) - (b * c);
      boost::numeric::ublas::matrix<double> n(2,2);
      n(0,0) =  d / determinant;
      n(0,1) = -b / determinant;
      n(1,0) = -c / determinant;
      n(1,1) =  a / determinant;
      return n;
    }
    default:
      assert(!"Should not get here: unsupported matrix size");
      throw std::runtime_error("Unsupported matrix size");
  }
}

void Matrix::Test()
{
  {
    static bool is_tested = false;
    if (is_tested) return;
    is_tested = true;
  }
  using boost::numeric::ublas::matrix;
  using boost::numeric::ublas::vector;
  {
    ///[1,4]
    ///[2,5]
    ///[3,6]
    const matrix<int> m = CreateMatrix(3,2, {1,2,3,4,5,6} );
    assert(m(0,0) == 1);
    assert(m(1,0) == 2);
    assert(m(2,0) == 3);
    assert(m(0,1) == 4);
    assert(m(1,1) == 5);
    assert(m(2,1) == 6);
  }
  {
    /// [ 1.0 2.0 ] -1    [ -2.0   1.0 ]
    /// [ 3.0 4.0 ]     = [  1.5  -0.5 ]
    const matrix<double> m = CreateMatrix(2,2, {1.0,3.0,2.0,4.0} );
    assert(m(0,0) == 1.0);
    assert(m(1,0) == 3.0);
    assert(m(0,1) == 2.0);
    assert(m(1,1) == 4.0);
    const matrix<double> n = Inverse(m);
    const double epsilon = 0.0000001; //Rounding error
    assert(n(0,0) > -2.0 - epsilon && n(0,0) < -2.0 + epsilon);
    assert(n(1,0) >  1.5 - epsilon && n(1,0) <  1.5 + epsilon);
    assert(n(0,1) >  1.0 - epsilon && n(0,1) <  1.0 + epsilon);
    assert(n(1,1) > -0.5 - epsilon && n(1,1) < -0.5 + epsilon);
  }
}
