#include "stdafx.h"
#include "qttest.h"
#include "model.h"

EditCardsDialog::EditCardsDialog( QWidget *parent, CardsStorage &storage, const Settings &st, CardsStorage::TConstSearchIterator itCurrent  )
    : QDialog(parent), storage(storage), m_st(st) 
{
  setWindowFlags( (windowFlags() | Qt::WindowMinMaxButtonsHint) & ~Qt::WindowContextHelpButtonHint );

  ui.setupUi(this);
  ui.toolButton_1->setDefaultAction( ui.actionAdd );
  ui.toolButton_2->setDefaultAction( ui.actionAdjustHeights );
  ui.toolButton_3->setDefaultAction( ui.actionDelete );
  ui.splitter->setStretchFactor(0, 1);

  for( CardsStorage::TConstSearchIterator it = storage.CardsBegin(); it != storage.CardsEnd(); ++it )
  {
    const int newRow = ui.tableFlashcard->rowCount();
    ui.tableFlashcard->insertRow( newRow );
    CardToRow( *it, ui.tableFlashcard, newRow );
  } 
  
  ui.tableFlashcard->resizeRowsToContents();
  ui.tableFlashcard->resizeColumnToContents(foreinLevelColumn);
  ui.tableFlashcard->resizeColumnToContents(nativeLevelColumn);
  ui.tableFlashcard->resizeColumnToContents(foreinAttemptColumn);
  ui.tableFlashcard->resizeColumnToContents(nativeAttemptColumn);
  
  if( ui.tableFlashcard->rowCount() > 0 )
  {
    const int selectedRow = itCurrent != storage.CardsEnd() ? 
        itCurrent - storage.CardsBegin(): storage.GetCardsSize() - 1;
    
    ui.tableFlashcard->selectRow( selectedRow );
  } 
  else
    emit on_tableFlashcard_currentCellChanged( -1, -1, -1, -1 );  
}

void EditCardsDialog::on_btnSave_clicked()
{
  storage.Clear();
  
  const int rowCount = ui.tableFlashcard->rowCount();
  Flashcard fc;
  
  for( int i = 0; i < rowCount; ++i )
  {
    RowToCard( ui.tableFlashcard, i, fc );
    storage.Add(fc);
  }
  
  accept();    
}

void EditCardsDialog::on_actionAdjustHeights_triggered()
{
  ui.tableFlashcard->resizeRowsToContents();
}

int EditCardsDialog::askForClosing()
{
  return QMessageBox::question(
      this,
      QApplication::applicationName(),
      tr("Save changes to cards?"),
      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
  );
}

void EditCardsDialog::closeEvent( QCloseEvent *event ) 
{
  switch( askForClosing() ) 
  {
  case QMessageBox::Save:
    event->ignore();
    on_btnSave_clicked();
    break;
  case QMessageBox::Discard:
    event->accept();
    break;
  case QMessageBox::Cancel:
  default:
    event->ignore();
  };
}

void EditCardsDialog::keyPressEvent(QKeyEvent *e)
{
  if( e->key() != Qt::Key_Escape ) 
    QDialog::keyPressEvent(e);
  else
    switch( askForClosing() ) 
    {
    case QMessageBox::Save:
      e->ignore();
      on_btnSave_clicked();
      break;
    case QMessageBox::Discard:
      QDialog::keyPressEvent(e);
      break;
    case QMessageBox::Cancel:
    default:
      e->ignore();
    };  
}

static QTableWidgetItem *createTWI( const QString &val )
{
  QTableWidgetItem *pRes = new QTableWidgetItem( val );

  return pRes;  
}

static QTableWidgetItem *createTWI( double val )
{
  QTableWidgetItem *pRes = new QTableWidgetItem( QString::number(val) );
  pRes->setTextAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
  
  return pRes;  
}

static QTableWidgetItem *createTWI( int val )
{
  QTableWidgetItem *pRes = new QTableWidgetItem( QString::number(val) );
  pRes->setTextAlignment( Qt::AlignHCenter | Qt::AlignVCenter );

  return pRes;  
}

void EditCardsDialog::RowToCard( const QTableWidget *tbl, int row, Flashcard &fc )
{
  fc.text[Lang::Foreign] = tbl->item(row, foreinColumn)->text(); 
  fc.text[Lang::Native] = tbl->item(row, nativeColumn)->text();
  fc.factor[Lang::Foreign] = tbl->item(row, foreinLevelColumn)->text().toDouble();
  fc.factor[Lang::Native] = tbl->item(row, nativeLevelColumn)->text().toDouble();
  fc.attempts[Lang::Foreign] = tbl->item(row, foreinAttemptColumn)->text().toInt();
  fc.attempts[Lang::Native] = tbl->item(row, nativeAttemptColumn)->text().toInt();
}

void EditCardsDialog::CardToRow( const Flashcard &fc, QTableWidget *tbl, int row )
{
  tbl->setItem( row, foreinColumn, createTWI(fc.text[Lang::Foreign]) );
  tbl->setItem( row, nativeColumn, createTWI(fc.text[Lang::Native]) );
  tbl->setItem( row, foreinLevelColumn, createTWI(fc.factor[Lang::Foreign]) );
  tbl->setItem( row, nativeLevelColumn, createTWI(fc.factor[Lang::Native]) );
  tbl->setItem( row, foreinAttemptColumn, createTWI(fc.attempts[Lang::Foreign]) );
  tbl->setItem( row, nativeAttemptColumn, createTWI(fc.attempts[Lang::Native]) );
}

EditCardsDialog::~EditCardsDialog()
{

}


void EditCardsDialog::on_actionAdd_triggered()
{                                             
  const int newRow = ui.tableFlashcard->rowCount();
  ui.tableFlashcard->insertRow( newRow );
  
  Flashcard fc( m_st.InitialWeight(newRow) );
  CardToRow( fc, ui.tableFlashcard, newRow ); 
  
  ui.tableFlashcard->selectRow( newRow );
  ui.plainTextForeign->setFocus();
}

void EditCardsDialog::on_tableFlashcard_currentCellChanged( int currentRow, int currentColumn, int previousRow, int previousColumn )
{
  (void)currentColumn;
  (void)previousColumn;
  
  if( previousRow >= 0 )
    ui.tableFlashcard->resizeRowToContents(previousRow); 
  
  bool isEnabled = false;
  QString foreignText;
  QString nativeText;
    
  if( currentRow >= 0 )
  {  
    isEnabled = true;
    foreignText = ui.tableFlashcard->item(currentRow, foreinColumn)->text();
    nativeText = ui.tableFlashcard->item(currentRow, nativeColumn)->text();
  }

  ui.plainTextForeign->setEnabled(isEnabled);
  ui.plainTextForeign->setPlainText(foreignText);

  ui.plainTextNative->setEnabled(isEnabled);
  ui.plainTextNative->setPlainText(nativeText);
}

void EditCardsDialog::on_plainTextForeign_textChanged()
{                             
  const int currentRow = ui.tableFlashcard->currentRow();
  
  if( currentRow >= 0 )
    ui.tableFlashcard->item(currentRow, foreinColumn)->setText( ui.plainTextForeign->toPlainText() ); 
}

void EditCardsDialog::on_plainTextNative_textChanged()
{
  const int currentRow = ui.tableFlashcard->currentRow();
    
  if( currentRow >= 0 )
    ui.tableFlashcard->item(ui.tableFlashcard->currentRow(), nativeColumn)->setText( ui.plainTextNative->toPlainText() );
}

void EditCardsDialog::on_actionDelete_triggered()
{   
  const int currentRow = ui.tableFlashcard->currentRow();

  if( currentRow >= 0 ) 
    ui.tableFlashcard->removeRow( ui.tableFlashcard->currentRow() );
}


