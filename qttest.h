#ifndef QTTEST_H
#define QTTEST_H

#include "ui_qttest.h"
#include "model.h"

class EditCardsDialog : public QDialog
{
    Q_OBJECT
           
private:
  enum 
  {
    foreinColumn,
    nativeColumn,
    foreinLevelColumn,
    nativeLevelColumn,  
    foreinAttemptColumn,
    nativeAttemptColumn   
  };
         
public:
  EditCardsDialog( QWidget *parent, CardsStorage &storage, const Settings &st, CardsStorage::TConstSearchIterator itCurrent );
  ~EditCardsDialog();

private slots:
  void on_actionAdd_triggered();
  void on_actionDelete_triggered();
  void on_actionAdjustHeights_triggered();
  void on_tableFlashcard_currentCellChanged( int currentRow, int currentColumn, int previousRow, int previousColumn );
  void on_plainTextForeign_textChanged();
  void on_plainTextNative_textChanged();
  void on_btnSave_clicked();
  
  
private:
  static void RowToCard( const QTableWidget *tbl, int row, Flashcard &fc );
  static void CardToRow( const Flashcard &fc, QTableWidget *tbl, int row );
  void closeEvent( QCloseEvent *event );
  void keyPressEvent( QKeyEvent *e );
  int askForClosing();
  
private:
  Ui::QtTestClass ui;
  CardsStorage &storage;
  const Settings &m_st;
};

#endif // QTTEST_H
