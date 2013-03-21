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
    FromXml( szStorageFileName, m_data );
    
  ui.actionInvertLng->setChecked( m_data.settings.Language() == Lang::Native );
  
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
  ui.btnNext->setEnabled( !m_data.storage.IsCardsEmpty() );
  m_iterCard = m_data.storage.CardsEnd();
  
  ui.lblAvgScore->setText( QString("Score: <b>%1</b>").arg( m_data.storage.Score(m_data.settings) ) );
}

QMainScr::~QMainScr()
{

}

void QMainScr::on_actionShow_Flashcards_triggered()
{
  const CardsStorage::TSearchIterator itCurrent( m_iterCard );
  Clear();
  const CardsStorage::TSize cardNum = itCurrent - m_data.storage.CardsBegin();
  EditCardsDialog cards( this, m_data.storage, m_data.settings, itCurrent );
   
  switch( cards.exec() )
  {
    case EditCardsDialog::NotChanged:
    case EditCardsDialog::OnlyEdit:
      m_iterCard = m_data.storage.CardsBegin() + cardNum;
      
      if( m_iterCard != m_data.storage.CardsEnd() )
        ShowFullCard(); 
    break;
    
    case EditCardsDialog::CompleatlyChanged:
      Clear(); 
    break;
  }
  
  SaveSettings();
}

void QMainScr::ShowFullCard()
{
  ui.btnYes->setEnabled( true );
  ui.btnNo->setEnabled( true );
  ui.btnNext->setEnabled( false );
  ui.plainTextNative->setPlainText( m_iterCard->text[!m_data.settings.Language()]);
  ui.lblScoreChange->setText( QString() );
  ui.plainTextForeign->setPlainText( m_iterCard->text[m_data.settings.Language()]);
}

void QMainScr::on_btnNext_clicked()
{
  if( m_iterCard == m_data.storage.CardsEnd() )
  {  
    m_iterCard = m_data.storage.GetRandomElem( m_data.settings.Language() ); 
    ui.plainTextForeign->setPlainText( m_iterCard->text[m_data.settings.Language()]);         
  }
  else
  {
    ShowFullCard();
  }   
}

void QMainScr::ReceiveAnswer( Answer::T ans )
{
  const double prevFactor = m_iterCard->factor[ m_data.settings.Language() ];  
  m_data.storage.ChangeFactor( m_iterCard, m_data.settings, ans );
  const double newFactor = m_iterCard->factor[ m_data.settings.Language() ];
  const double newAttempt = m_iterCard->attempts[ m_data.settings.Language() ]; 
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
    CardsStorage::TSearchIterator it = m_data.storage.GetRandomElem( m_data.settings.Language() );
    ++map[it];
  }

  QString str;

  for( TMap::const_iterator it = map.begin(); it != map.end(); ++it )
    str += QString("%2\t%3\n")
              .arg(it->second)
              .arg(it->first->factor[m_data.settings.Language()]);

  ui.plainTextForeign->setPlainText(str);  
}

void QMainScr::on_actionInvertLng_toggled( bool checked )
{
  m_data.settings.Language( checked ? Lang::Native : Lang::Foreign );
  Clear();
}

void QMainScr::SaveSettings()
{
  ToXml( szStorageFileName, m_data );
}

inline QString ToCSV( const QString &s )
{
  return '"' + QString(s).replace("\"", "\"\"") + '"';
}

inline QString TableDef( const QString &def, const QString &val )
{
  return QString("<tr><td align=right><b>%1</b></td><td width=5></td><td>%2</td></tr>").arg(def).arg(val);
}

inline QString TableDef( const QString &def, CardsStorage::TSize val )
{
  return TableDef( def, QString::number(val) );
}

inline QString TableDef( const QString &def, double val )
{
  return TableDef( def, QString::number(val) );
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
  
  for( CardsStorage::TConstSearchIterator it = m_data.storage.CardsBegin(); it != m_data.storage.CardsEnd(); ++it )
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
  typedef CardsStorage::TSize TSize;   
  
  const Lang::T lang = m_data.settings.Language();
  const TSize cardsCount = m_data.storage.GetCardsSize();
  const double initalWeight = m_data.settings.InitialWeight( cardsCount );
  const double weightAfterSuccessAnswer = m_data.storage.CalcNewFactor( initalWeight, m_data.settings, Answer::Correct ); 
  TSize newCardsCount = 0;
  TSize unknownCardsCount = 0;
  TSize totalAttempts = 0;
  TSize attemptsToInitial = 0;  
  TSize attemptsToKnown = 0;
    
  for( CardsStorage::TConstSearchIterator it = m_data.storage.CardsBegin(); it != m_data.storage.CardsEnd(); ++it )
  {
    if( it->attempts[lang] == 0 )
      ++newCardsCount;
      
    if( it->factor[lang] > weightAfterSuccessAnswer || it->attempts[lang] == 0 )
      ++unknownCardsCount;  
      
    totalAttempts += it->attempts[lang];   
    attemptsToInitial += CardsStorage::AttempsCountToReachWeight( it->factor[lang], initalWeight, m_data.settings.CorrectAnswerFactor() );
    attemptsToKnown += CardsStorage::AttempsCountToReachWeight( it->factor[lang],  weightAfterSuccessAnswer, m_data.settings.CorrectAnswerFactor() );
  }

  QMessageBox::information( this, "Statistics",
    "<table>" 
    + TableDef( "Cards Count:", cardsCount )
    + TableDef( "New Cards Count:", newCardsCount )
    + TableDef( "Unknown Cards Count:", unknownCardsCount )
    + TableDef( "Known Cards Count:", cardsCount - unknownCardsCount )
    + TableDef( "Total Attempts:", totalAttempts )
    + TableDef( "Successful Attempts To Initial Weight:", attemptsToInitial )
    + TableDef( "Successful Attempts To Known Weight:", attemptsToKnown )
    + TableDef( "Known Cards Weight:", weightAfterSuccessAnswer )
    + "</table>"
  ); 
}

void QMainScr::on_actionAbout_triggered()
{
  QMessageBox::about( this, "About iFlashcards",
    "<h1>iFlashcards</h1>"
    "Dmitry Shesterkin 2013<br>"
    "<table>"
    + TableDef( "Version", "1.2.0")
    + TableDef( "Email", "<a href=\"mailto:dfb@yandex.ru?subject=iFlashcards Review/Question/Problem&body=Please share your opinion about program!\">dfb@yandex.ru</a>")
    + TableDef( "Source", "<a href=\"http://github.com/RedaZamZam/iFlashcards\">http://github.com/RedaZamZam/iFlashcards</a>")
    + "</table>"
  );
}