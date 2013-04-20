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
  
  QApplication::instance()->installEventFilter( this );
  
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
  QApplication::instance()->removeEventFilter( this );
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

inline QString TableGap()
{
  return TableDef( "", "" );
}

inline QString ToPercent( double val )
{
  return QString::number(val * 100, 'f', 2) + '%';
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
  if(  m_data.storage.IsCardsEmpty() )
  {
    QMessageBox::information( this, "Statistics", "There are not enough cards number to gather statistics." );
    return; 
  } 

  typedef CardsStorage::TSize TSize;   
  typedef std::vector<double> TWeights;
  
  const Lang::T lang = m_data.settings.Language();
  const TSize cardsCount = m_data.storage.GetCardsSize();
  const double initalWeight = m_data.settings.InitialWeight( cardsCount );
  const double weightAfterSuccessAnswer = m_data.storage.CalcNewFactor( initalWeight, m_data.settings, Answer::Correct ); 
  
  TSize newCardsCount = 0;
  TSize unknownCardsCount = 0;
  TSize totalAttempts = 0;
  TSize attemptsToInitial = 0;  
  TSize attemptsToKnown = 0;
  double newCardsWeightSum = 0;
  double unknownCardsWeightSum = 0;
  double weightSum = 0;
  TWeights weights;
   
  weights.reserve( cardsCount );
   
  for( CardsStorage::TConstSearchIterator it = m_data.storage.CardsBegin(); it != m_data.storage.CardsEnd(); ++it )
  {
    if( it->attempts[lang] == 0 )
    {
      ++newCardsCount;
      newCardsWeightSum += it->factor[lang];
    }
      
    if( it->factor[lang] > weightAfterSuccessAnswer || it->attempts[lang] == 0 )
    {
      ++unknownCardsCount;
      unknownCardsWeightSum += it->factor[lang];
    }  
      
    totalAttempts += it->attempts[lang];   
    attemptsToInitial += m_data.storage.AttempsCountToReachWeight( it->factor[lang], initalWeight, m_data.settings );
    attemptsToKnown += m_data.storage.AttempsCountToReachWeight( it->factor[lang],  weightAfterSuccessAnswer, m_data.settings );
    
    weights.push_back( it->factor[lang] );
    weightSum += it->factor[lang];
  }

  std::sort( weights.begin(), weights.end(), std::greater<TWeights::value_type>() );

  //Вычисляем количество карточек вероятность выбора одной из которых равна каждому значению rgPecentileBorders
  static const double rgPecentileBorders[] = { 0.1, 0.25, 0.5, 0.75, 0.9, 0.99 };
  TSize rgPecentileValues[ ARRAY_SIZE(rgPecentileBorders) ] = {};
  double curWeightSum = 0;
  TSize curPecentile = 0;
  
  for( TWeights::const_iterator it = weights.begin(); it != weights.end() && curPecentile < ARRAY_SIZE(rgPecentileValues); ++it )
  {
    curWeightSum += *it;
    
    //Использование while вместо if здесь позволит учитывать пересечение границ, но не хочется усложнять
    if( curWeightSum / weightSum >= rgPecentileBorders[curPecentile] )
    {
      rgPecentileValues[curPecentile] = it - weights.begin() + 1;
      ++curPecentile;
    }  
  } 

  QString resultText;
  
  resultText +=
      TableDef( "Cards Count:", cardsCount )
    + TableDef( "Total Attempts:", totalAttempts )
    + TableGap()
    + TableDef( "New Cards Count:", newCardsCount )
    + TableDef( "Unknown Cards Count:", unknownCardsCount )
    + TableDef( "Known Cards Count:", cardsCount - unknownCardsCount )
    + TableGap()
    + TableDef( "Successful Attempts To Initial Weight:", attemptsToInitial )
    + TableDef( "Successful Attempts To Known Weight:", attemptsToKnown )
    + TableGap()
    + TableDef( "Initial Card Weight:", initalWeight )
    + TableDef( "Known Card Weight:", weightAfterSuccessAnswer )
    + TableDef( "Average Card Weight:", weightSum / cardsCount )
    + TableDef( "Median Card Weight:", weights[ weights.size() / 2 ] )
    + TableGap()
    + TableDef( "Next added card selection probability: ", ToPercent( initalWeight / weightSum) )
    + TableDef( "One of new cards selection probability: ", ToPercent( newCardsWeightSum / weightSum) )
    + TableDef( "One of unknown cards selection probability: ", ToPercent( unknownCardsWeightSum / weightSum) )
    + TableGap()
  ;
  
  for( int i = 0; i < ARRAY_SIZE(rgPecentileValues); ++i )
    resultText += TableDef( QString("Cards count which are selected with probability %1%:").arg(rgPecentileBorders[i] * 100), rgPecentileValues[i] );

  QMessageBox::information( this, "Statistics", "<table>" + resultText + "</table>"); 
}

void QMainScr::on_actionAbout_triggered()
{
  QMessageBox::about( this, "About iFlashcards",
    "<h1>iFlashcards</h1>"
    "Dmitry Shesterkin 2013<br>"
    "<table>"

    + TableDef( "Version", "1.2.6")
    + TableDef( "Project", "<a href=\"http://smart-flashcards.googlecode.com\">http://smart-flashcards.googlecode.com</a>")
    + TableDef( "Source", "<a href=\"http://github.com/RedaZamZam/iFlashcards\">http://github.com/RedaZamZam/iFlashcards</a>")
    + TableDef( "Email", "<a href=\"mailto:dfb@yandex.ru?subject=iFlashcards Review/Question/Problem&body=Please share your opinion about program!\">dfb@yandex.ru</a>")

    + "</table>"
  );
}

bool QMainScr::eventFilter( QObject *ob, QEvent *e )
{
  (void)ob;

  if( (e->type() != QEvent::KeyPress && e->type() != QEvent::KeyRelease) || !isActiveWindow() )
    return false; 
       
  QKeyEvent * const keyEvent = static_cast<QKeyEvent *>(e);
     
  if( keyEvent->key() == Qt::Key_Space && !keyEvent->isAutoRepeat() ) 
  {
    if( e->type() == QEvent::KeyPress )                                                                                     
    {
      if( !ui.btnNext->isEnabled() )
        ui.plainTextNative->setPlainText( "" );  
      else
        on_btnNext_clicked();
    }
    else if( e->type() == QEvent::KeyRelease )
    {                                                                                                              
      if( !ui.btnNext->isEnabled() )
        on_btnNext_clicked(); 
    } 
  
    e->accept();
    return true;
  }

  return false;
}