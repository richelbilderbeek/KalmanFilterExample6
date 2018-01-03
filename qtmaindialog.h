#ifndef QTMAINDIALOG_H
#define QTMAINDIALOG_H

#ifdef _WIN32
#undef __STRICT_ANSI__
#endif

#include <QDialog>

namespace Ui {
  class QtMainDialog;
}

struct QwtPlotCurve;

class QtMainDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit QtMainDialog(QWidget *parent = 0);
  ~QtMainDialog();
  
private:
  Ui::QtMainDialog *ui;

  QwtPlotCurve * const m_curve_v_estimate;
  QwtPlotCurve * const m_curve_v_measured;
  QwtPlotCurve * const m_curve_v_real;
  QwtPlotCurve * const m_curve_x_estimate;
  QwtPlotCurve * const m_curve_x_measured;
  QwtPlotCurve * const m_curve_x_real;

};

#endif // QTMAINDIALOG_H
