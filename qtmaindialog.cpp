#ifdef _WIN32
#undef __STRICT_ANSI__
#endif

#include "qtmaindialog.h"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include "qwt_plot_curve.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_zoomer.h"

#include "maindialog.h"
#include "matrix.h"
#include "ui_qtmaindialog.h"

QtMainDialog::QtMainDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::QtMainDialog),
  m_curve_v_estimate(new QwtPlotCurve),
  m_curve_v_measured(new QwtPlotCurve),
  m_curve_v_real(new QwtPlotCurve),
  m_curve_x_estimate(new QwtPlotCurve),
  m_curve_x_measured(new QwtPlotCurve),
  m_curve_x_real(new QwtPlotCurve)

{
  ui->setupUi(this);

  ui->plot_x->setTitle("Position");
  ui->plot_x->setAxisTitle(QwtPlot::xBottom,"Time");
  ui->plot_x->setAxisTitle(QwtPlot::yLeft,"Position");
  m_curve_x_estimate->setTitle("Estimated");
  m_curve_x_estimate->attach(ui->plot_x);
  m_curve_x_estimate->setStyle(QwtPlotCurve::Lines);
  m_curve_x_estimate->setPen(QPen(QColor(255,0,0)));
  m_curve_x_measured->setTitle("Measured");
  m_curve_x_measured->attach(ui->plot_x);
  m_curve_x_measured->setStyle(QwtPlotCurve::Lines);
  m_curve_x_measured->setPen(QPen(QColor(0,255,0)));
  m_curve_x_real->setTitle("Real");
  m_curve_x_real->attach(ui->plot_x);
  m_curve_x_real->setStyle(QwtPlotCurve::Lines);
  m_curve_x_real->setPen(QPen(QColor(0,0,255)));

  ui->plot_v->setTitle("Velocity");
  ui->plot_v->setAxisTitle(QwtPlot::xBottom,"Time");
  ui->plot_v->setAxisTitle(QwtPlot::yLeft,"Velocity");
  m_curve_v_estimate->setTitle("Estimated");
  m_curve_v_estimate->attach(ui->plot_v);
  m_curve_v_estimate->setStyle(QwtPlotCurve::Lines);
  m_curve_v_estimate->setPen(QPen(QColor(255,0,0)));
  m_curve_v_measured->setTitle("Measured");
  m_curve_v_measured->attach(ui->plot_v);
  m_curve_v_measured->setStyle(QwtPlotCurve::Lines);
  m_curve_v_measured->setPen(QPen(QColor(0,255,0)));
  m_curve_v_real->setTitle("Real");
  m_curve_v_real->attach(ui->plot_v);
  m_curve_v_real->setStyle(QwtPlotCurve::Lines);
  m_curve_v_real->setPen(QPen(QColor(0,0,255)));

  //Add grids
  { QwtPlotGrid * const grid = new QwtPlotGrid; grid->setPen(QPen(QColor(196,196,196))); grid->attach(ui->plot_x); }
  { QwtPlotGrid * const grid = new QwtPlotGrid; grid->setPen(QPen(QColor(196,196,196))); grid->attach(ui->plot_v); }
  //Add zoomers
  {
    new QwtPlotZoomer(ui->plot_x->canvas());
    new QwtPlotZoomer(ui->plot_v->canvas());
  }

  //Do the sim
  const double t = 0.1;
  const double acceleration = 1.0;

  //The real state vector
  //[ position ]
  //[ velocity ]
  const boost::numeric::ublas::vector<double> init_x_real = Matrix::CreateVector( { 0.0, 0.0 } );

  //Real measurement noise
  //[ standard deviation of noise in position ]   [ standard deviation of noise in GPS                       ]
  //[ standard deviation of noise in velocity ] = [ standard deviation of noise in defect/unused speedometer ]
  const boost::numeric::ublas::vector<double> x_real_measurement_noise = Matrix::CreateVector( { 10.0, 10000000.0 } );

  //Guess of the state matrix
  //Position and velocity guess is way off on purpose
  //[ position ]
  //[ velocity ]
  const boost::numeric::ublas::vector<double> x_first_guess = Matrix::CreateVector( { 100.0, 10.0 } );

  //Guess of the covariances
  //[ 1.0   0.0 ]
  //[ 0.0   1.0 ]
  const boost::numeric::ublas::matrix<double> p_first_guess = Matrix::CreateMatrix(2,2, { 1.0, 0.0, 0.0, 1.0 } );

  //Effect of inputs on state
  //Input = gas pedal, which gives acceleration 'a'
  //[ 1.0   0.5 * t * t ]   [teleportation (not used)                 x = 0.5 * a * t * t ]
  //[ 0.0   t           ] = [no effect of teleportation on velocity   v = a * t           ]
  const boost::numeric::ublas::matrix<double> control = Matrix::CreateMatrix(2,2, { 1.0, 0.0, 0.5 * t * t, t } );

  //Estimated measurement noise
  //[ 10.0          0.0 ]   [ Estimated noise in GPS   ?                                                     ]
  //[  0.0   10000000.0 ] = [ ?                        Estimated noise in speedometer (absent in this setup) ]
  const boost::numeric::ublas::matrix<double> measurement_noise = Matrix::CreateMatrix(2,2, { 10.0, 0.0, 0.0, 10000000.0 } );

  //Observational matrix
  //[ 1.0   0.0 ]   [GPS measurement   ?                                         ]
  //[ 0.0   0.0 ] = [?                 Speedometer (absent/unused in this setup) ]
  const boost::numeric::ublas::matrix<double> observation = Matrix::CreateMatrix(2,2, { 1.0, 0.0, 0.0, 0.0 } );

  //Real process noise
  //[ 0.001 ]   [ noise in position ]
  //[ 0.001 ] = [ noise in velocity ]
  const boost::numeric::ublas::vector<double> real_process_noise = Matrix::CreateVector( {0.01,0.01} );

  //Estimated process noise covariance
  //[ 0.01   0.01 ]
  //[ 0.01   0.01 ]
  const boost::numeric::ublas::matrix<double> process_noise = Matrix::CreateMatrix(2,2,{0.01,0.01,0.01,0.01});

  //State transition matrix, the effect of the current state on the next
  //[ 1.0     t ]   [ position keeps its value             a velocity changes the position ]
  //[ 0.0   1.0 ] = [ position has no effect on velocity   a velocity keeps its value      ]
  const boost::numeric::ublas::matrix<double> state_transition = Matrix::CreateMatrix(2,2,{1.0,0.0,t,1.0});

  const int time = 250;
  const MainDialog d(
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
    x_real_measurement_noise);

  //Display data
  {
    const boost::numeric::ublas::matrix<double>& data = d.GetData();
    std::vector<double> time_series(time);
    std::vector<double> v_estimate(time);
    std::vector<double> v_measured(time);
    std::vector<double> v_real(time);
    std::vector<double> x_estimate(time);
    std::vector<double> x_measured(time);
    std::vector<double> x_real(time);

    for (int row=0; row!=time; ++row)
    {
      time_series[row] = static_cast<double>(row);
      x_real[     row] = data(row,0);
      x_measured[ row] = data(row,1);
      x_estimate[ row] = data(row,2);
      v_real[     row] = data(row,3);
      v_measured[ row] = data(row,4);
      v_estimate[ row] = data(row,5);
    }
    #ifdef _WIN32
    m_curve_v_estimate->setData(new QwtPointArrayData(&time_series[0],&v_estimate[0],time_series.size()));
    m_curve_v_measured->setData(new QwtPointArrayData(&time_series[0],&v_measured[0],time_series.size()));
    m_curve_v_real->setData(new QwtPointArrayData(&time_series[0],&v_real[0],time_series.size()));
    m_curve_x_estimate->setData(new QwtPointArrayData(&time_series[0],&x_estimate[0],time_series.size()));
    m_curve_x_measured->setData(new QwtPointArrayData(&time_series[0],&x_measured[0],time_series.size()));
    m_curve_x_real->setData(new QwtPointArrayData(&time_series[0],&x_real[0],time_series.size()));
    #else
    m_curve_v_estimate->setData(&time_series[0],&v_estimate[0],time_series.size());
    m_curve_v_measured->setData(&time_series[0],&v_measured[0],time_series.size());
    m_curve_v_real->setData(&time_series[0],&v_real[0],time_series.size());
    m_curve_x_estimate->setData(&time_series[0],&x_estimate[0],time_series.size());
    m_curve_x_measured->setData(&time_series[0],&x_measured[0],time_series.size());
    m_curve_x_real->setData(&time_series[0],&x_real[0],time_series.size());
    #endif
  }
  ui->plot_v->replot();
  ui->plot_x->replot();

}

QtMainDialog::~QtMainDialog()
{
  delete m_curve_v_estimate;
  delete m_curve_v_measured;
  delete m_curve_v_real;
  delete m_curve_x_estimate;
  delete m_curve_x_measured;
  delete m_curve_x_real;
  delete ui;
}
