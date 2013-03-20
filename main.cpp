#include "stdafx.h"
#include "mainwnd.h"
#include <QtGui/QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  
  try
  {
    QMainScr w;
    w.show();
    return a.exec();
  }
  catch ( const std::exception &e )
  {
    QMessageBox::critical( 0, QApplication::applicationName(), QObject::tr("Critical error: %1").arg(e.what()) );
  }
  catch (...)
  {
    QMessageBox::critical( 0, QApplication::applicationName(), QObject::tr("Unknown error") );
  }
}
