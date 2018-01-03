#ifdef _WIN32
#undef __STRICT_ANSI__
#endif

#include "maindialog.h"

#include <vector>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include "kalmanfilter.h"
#include "matrix.h"
#include "whitenoisesystem.h"

MainDialog::MainDialog(
  const int time,
  const double acceleration,
  const boost::numeric::ublas::matrix<double>& control,
  const boost::numeric::ublas::matrix<double>& measurement_noise,
  const boost::numeric::ublas::matrix<double>& observation,
  const boost::numeric::ublas::matrix<double>& p_first_guess,
  const boost::numeric::ublas::matrix<double>& process_noise,
  const boost::numeric::ublas::matrix<double>& state_transition,
  const boost::numeric::ublas::vector<double>& init_x_real,
  const boost::numeric::ublas::vector<double>& real_process_noise,
  const boost::numeric::ublas::vector<double>& x_first_guess,
  const boost::numeric::ublas::vector<double>& x_real_measurement_noise)
  : m_data(
    CreateData(
      time,
      acceleration,
      control,
      measurement_noise,
      observation,
      p_first_guess,
      process_noise,
      state_transition,
      init_x_real,
      real_process_noise,
      x_first_guess,
      x_real_measurement_noise
      )
    )
{



}

const boost::numeric::ublas::matrix<double> MainDialog::CreateData(
  const int time,
  const double acceleration,
  const boost::numeric::ublas::matrix<double>& control,
  const boost::numeric::ublas::matrix<double>& measurement_noise,
  const boost::numeric::ublas::matrix<double>& observation,
  const boost::numeric::ublas::matrix<double>& p_first_guess,
  const boost::numeric::ublas::matrix<double>& process_noise,
  const boost::numeric::ublas::matrix<double>& state_transition,
  const boost::numeric::ublas::vector<double>& init_x_real,
  const boost::numeric::ublas::vector<double>& real_process_noise,
  const boost::numeric::ublas::vector<double>& x_first_guess,
  const boost::numeric::ublas::vector<double>& x_real_measurement_noise)

{
  Matrix::Test();

  using boost::numeric::ublas::matrix;
  using boost::numeric::ublas::vector;

  //The resulting matrix, has time rows and 6 columns
  matrix<double> data(time,6);
  assert(time == static_cast<int>(data.size1()));
  assert(data.size2() == 6);
  assert(GetHeader().size() == data.size2());

  WhiteNoiseSystem s(control,init_x_real,x_real_measurement_noise,real_process_noise,state_transition);

  KalmanFilter k(control,x_first_guess,p_first_guess,measurement_noise,observation,process_noise,state_transition);

  //std::cout << "x_real,x_measured,x_Kalman,v_real,v_measured,v_Kalman\n";
  for (int i=0;i!=time;++i)
  {
    //A constant push the gas pedal, which results in a constant acceleration
    const vector<double> input = Matrix::CreateVector( { 0.0, acceleration } );
    //Update reality, that is, let the real system (i.e. reality) go to its next state
    s.GoToNextState(input);
    //Perform a noisy measurement
    const vector<double> z_measured = s.Measure();
    //Pass this measurement to the filter
    k.SupplyMeasurementAndInput(z_measured,input);
    //Display what the filter predicts
    const vector<double> x_est_last = k.Predict();

    assert(i < static_cast<int>(data.size1()));
    assert(5 < data.size2());
    data(i,0) = s.PeekAtRealState()(0);
    data(i,1) = z_measured(0);
    data(i,2) = x_est_last(0);
    data(i,3) = s.PeekAtRealState()(1);
    data(i,4) = z_measured(1);
    data(i,5) = x_est_last(1);
  }
  return data;
}

const boost::numeric::ublas::vector<std::string> MainDialog::GetHeader()
{
  boost::numeric::ublas::vector<std::string> v(6);
  assert(5 < v.size());
  v(0) = "x_real";
  v(1) = "x_measured";
  v(2) = "x_Kalman";
  v(3) = "v_real";
  v(4) = "v_measured";
  v(5) = "v_Kalman";
  return v;
}
