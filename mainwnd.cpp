#include "stdafx.h"
#include "mainwnd.h"
#include "qttest.h"
#include <QFile>
#include <QApplication>
#include <QTextStream>

const char * const szStorageFileName = "settings.xml";

QMainScr::QMainScr(QWidget *parent, Qt::WFlags flags)
: QMainWindow(parent, flags)
{
  ui.setupUi(this);
  QObject::connect( QApplication::instance() , SIGNAL(aboutToQuit()), this, SLOT(SaveSettings()) );
  
  if( QFile::exists(szStorageFileName) )
    FromXml( szStorageFileName, data );
    
  ui.actionInvertLng->setChecked( data.settings.Language() == Lang::Native );
  
  Clear();
}

void QMainScr::Clear()
{
  ui.lblScoreChange->setText("");
  ui.lblAvgScore->setText("");
  ui.plainTextForeign->setPlainText("");
  ui.plainTextNative->setPlainText("");
  ui.btnYes->setEnabled( false );
  ui.btnNo->setEnabled( false );
  ui.btnNext->setEnabled( !data.storage.IsCardsEmpty() );
  m_iterCard = data.storage.CardsEnd();
  
  ui.lblAvgScore->setText( QString("Score: <b>%1</b>").arg( data.storage.Score(data.settings) ) );
}

QMainScr::~QMainScr()
{

}

void QMainScr::on_actionShow_Flashcards_triggered()
{
  const CardsStorage::TSearchIterator itCurrent( m_iterCard );
  Clear();
  EditCardsDialog cards( this, data.storage, data.settings, itCurrent ); 
  cards.exec();
  SaveSettings();
  Clear();
}

void QMainScr::on_btnNext_clicked()
{
  if( m_iterCard == data.storage.CardsEnd() )
  {  
    m_iterCard = data.storage.GetRandomElem( data.settings.Language() );    
    ui.plainTextForeign->setPlainText( m_iterCard->text[data.settings.Language()]);  
  }
  else
  {
    ui.btnYes->setEnabled( true );
    ui.btnNo->setEnabled( true );
    ui.btnNext->setEnabled( false );
    ui.plainTextNative->setPlainText( m_iterCard->text[!data.settings.Language()]);
    ui.lblScoreChange->setText( QString() );
  }  
}

void QMainScr::ReceiveAnswer( Answer::T ans )
{
  const double prevFactor = m_iterCard->factor[ data.settings.Language() ];  
  data.storage.ChangeFactor( m_iterCard, data.settings, ans );
  const double newFactor = m_iterCard->factor[ data.settings.Language() ];
  const double newAttempt = m_iterCard->attempts[ data.settings.Language() ]; 
  Clear();
  
  ui.lblScoreChange->setText( QString("Last phrase weight has been changed: %1 -> %2. Attempt: %3").arg(prevFactor).arg(newFactor).arg(newAttempt) );
  on_btnNext_clicked();
}

void QMainScr::on_btnYes_clicked()
{
  ReceiveAnswer( Answer::Correct );
}

void QMainScr::on_btnNo_clicked()
{
  ReceiveAnswer( Answer::Incorrect );
}

void QMainScr::on_actionTest_triggered()
{
  Clear();
  
  typedef std::map<CardsStorage::TSearchIterator, unsigned> TMap;
  TMap map;

  for( int i = 0; i < 10000000; ++i )
  {
    CardsStorage::TSearchIterator it = data.storage.GetRandomElem( data.settings.Language() );
    ++map[it];
  }

  QString str;

  for( TMap::const_iterator it = map.begin(); it != map.end(); ++it )
    str += QString("%2\t%3\n")
              .arg(it->second)
              .arg(it->first->factor[data.settings.Language()]);

  ui.plainTextForeign->setPlainText(str);  
}

void QMainScr::on_actionInvertLng_toggled( bool checked )
{
  data.settings.Language( checked ? Lang::Native : Lang::Foreign );
  Clear();
}

void QMainScr::SaveSettings()
{
  ToXml( szStorageFileName, data );
}

inline QString ToCSV( const QString &s )
{
  return '"' + QString(s).replace("\"", "\"\"") + '"';
}

inline QString TableDef( const QString &def, const QString &val  )
{
  return QString("<tr><td><b>%1</b></td><td width=10></td><td>%2</td></tr>").arg(def).arg(val);
}

void QMainScr::on_actionExportToCSV_triggered()
{
  const QString fileName = QFileDialog::getSaveFileName(this, QString(),
    QString("iflashcards.csv"),
    tr("CSV files (*.csv)"));
    
  if( fileName.isEmpty() )
    return;
  
  QFile file(fileName);
  
  if( !file.open( QIODevice::WriteOnly | QIODevice::Truncate) )
    return;
      
  QTextStream out( &file );
  out.setCodec( "UTF-8" );
  out.setGenerateByteOrderMark(true);
  
  out << "Foreign;Native;F->N Weight;N->F Weight;F->N Attempts;N->F Attempts\n";
  
  for( CardsStorage::TConstSearchIterator it = data.storage.CardsBegin(); it != data.storage.CardsEnd(); ++it )
  {
    out 
      << ToCSV( it->text[Lang::Foreign] ) << ';'
      << ToCSV( it->text[Lang::Native] ) << ';'
      << it->factor[Lang::Foreign] << ';'
      << it->factor[Lang::Native] << ';' 
      << it->attempts[Lang::Foreign] << ';'
      << it->attempts[Lang::Native] << ';'
      << '\n';       
  }
}

void QMainScr::on_actionStatistics_triggered()
{
  QMessageBox::information( this, QApplication::applicationName(),
    "<table>" 
    + TableDef( "Version", "12")
    + TableDef( "Version", "12")
    + "</table>"
  ); 
}

void QMainScr::on_actionAbout_triggered()
{
  QMessageBox::about( this, QApplication::applicationName(),
    "<h1>iFlashcards</h1>"
    "Dmitry Shesterkin 2013"
    "<table>"
    + TableDef( "Version", "1.0.0")
    + TableDef( "Email", "<a href=\"mailto:dfb@yandex.ru?subject=iFlashcards Review/Question/Problem&body=Please share your opinion about program!\">dfb@yandex.ru</a>")
    + TableDef( "Source", "<a href=\"http://github.com/RedaZamZam/iFlashcards\">http://github.com/RedaZamZam/iFlashcards</a>")
    + "</table>"
  );
}