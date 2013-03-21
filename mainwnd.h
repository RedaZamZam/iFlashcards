#ifndef MAINWND_H
#define MAINWND_H

#include "ui_MainScr.h"
#include "model.h"

struct FileData
{
  Settings settings;
  CardsStorage storage;
  
  template<class Archive>
  void serialize(Archive & ar, const unsigned int /* version */)
  {
    ar & BOOST_SERIALIZATION_NVP(settings)
      & BOOST_SERIALIZATION_NVP(storage);
  } 
};

class QMainScr : public QMainWindow
{
  Q_OBJECT

public:
  QMainScr(QWidget *parent = 0, Qt::WFlags flags = 0);
  ~QMainScr();

private slots:
  void on_actionShow_Flashcards_triggered();
  void on_btnNext_clicked();
  void on_btnYes_clicked();
  void on_btnNo_clicked();
  void on_actionTest_triggered();
  void on_actionInvertLng_toggled( bool checked );
  void on_actionExportToCSV_triggered();
  void on_actionStatistics_triggered();
  void on_actionAbout_triggered();
  void SaveSettings();

private:
  void Clear();
  void ReceiveAnswer( Answer::T ans );
  void ShowFullCard();

private:
  Ui::MainWindow ui;
  FileData m_data;
  CardsStorage::TSearchIterator m_iterCard;
};

#endif // QTTEST_H
