#ifndef MODEL_H
#define MODEL_H

#include <fstream>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/array.hpp>  
#include <boost/random/mersenne_twister.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <QString>


//Правильный способ для сериализации QString - это определить:
//  BOOST_CLASS_IMPLEMENTATION(QString, boost::serialization::primitive_type)
//но уже не хочется терять обратную совместимость
namespace boost {
namespace serialization {

  template<class Archive>
  inline void save( Archive& ar, const QString& s, const unsigned int /*version*/ )
  {
    using boost::serialization::make_nvp;
    std::wstring stdStr( s.toStdWString() );
    ar << make_nvp("value", stdStr);
  }

  template<class Archive>
  inline void load( Archive& ar, QString& s, const unsigned int /*version*/ )
  {
    using boost::serialization::make_nvp;

    std::wstring stdStr;
    ar >> make_nvp("value", stdStr);
    s = QString::fromStdWString(stdStr);
  }

  template<class Archive>
  inline void serialize( Archive& ar, QString& s, const unsigned int file_version )
  {
    boost::serialization::split_free(ar, s, file_version);
  }

 } // namespace serialization
} // namespace boost

template<class T>
static void ToXml( const char *szFileName, const T &ob )
{
  std::wofstream outStream(szFileName);
  boost::archive::xml_woarchive oa(outStream);
  oa << BOOST_SERIALIZATION_NVP(ob);
}

template<class T>
static void FromXml( const char *szFileName, T &ob )
{                 
  std::wifstream inStream(szFileName);
  boost::archive::xml_wiarchive oa(inStream);
  oa >> BOOST_SERIALIZATION_NVP(ob);
}

namespace Lang
{
  enum T
  {
    Foreign,
    Native,
    Count
  };
};

namespace Answer
{
  enum T
  {
    Correct,
    Incorrect
  };
};

class Settings
{
  Lang::T language;
  double maxWeightFactor;
  double correctAnswerFactor;
  double incorrectAnswerFactor;
  
public:  
  Settings(): 
    language(Lang::Native), maxWeightFactor(1), 
    correctAnswerFactor( 0.5 ), incorrectAnswerFactor( 0.5 )  
  {}
  
  Lang::T Language() const { return language; }
  void Language(Lang::T val) { language = val; }
  double CorrectAnswerFactor() const { return correctAnswerFactor; }  
  double IncorrectAnswerFactor() const { return incorrectAnswerFactor; }
  
  double MaxWeight( size_t cardsCount ) const 
  { 
    //Максимальный вес карты должен зависеть от количества карт, иначе
    //возможна ситуация, при которой у пользователя есть множество выученных карт, 
    //т.е. их вес примерно равен 1 и он добавляет несколько новых карт. 
    //При этом суммарный вес старых карт превышает вес добавленных карт и 
    //у пользователя слишком часто появляются старые карты, а не новые.
    //Ограничим вес значением 2, для упрощения расчётов
    return std::max( maxWeightFactor * cardsCount, 2.0 ); 
  }
  
  double InitialWeight( size_t cardsCount ) const 
  { 
    //Вес новый карты равен половине максимального веса для того,
    //чтобы была возможность двигаться в обе стороны
    return MaxWeight(cardsCount) / 2; 
  }
  
  template<class Archive>
  void serialize(Archive & ar, const unsigned int /* version */)
  {
    ar & BOOST_SERIALIZATION_NVP(language)
       & BOOST_SERIALIZATION_NVP(maxWeightFactor)
       & BOOST_SERIALIZATION_NVP(correctAnswerFactor)
       & BOOST_SERIALIZATION_NVP(incorrectAnswerFactor);
  } 
};

struct Flashcard
{
  QString text[Lang::Count];
  double factor[Lang::Count];
  int attempts[Lang::Count];
    
  explicit Flashcard( double initialWeight = 1 )
  {
    for( int i = 0; i < Lang::Count; ++i )
    {
      factor[i] = initialWeight;
      attempts[i] = 0;
    }
  }
  
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version )
  {
    ar & BOOST_SERIALIZATION_NVP(text)
       & BOOST_SERIALIZATION_NVP(factor);
       
    if( version >= 1 )
      ar & BOOST_SERIALIZATION_NVP(attempts);
  } 
};

BOOST_CLASS_VERSION(Flashcard, 1)

class CardsStorage
{
public:
  typedef std::vector<Flashcard> TFlashcads;
  typedef TFlashcads::iterator TIterator;  
  typedef TFlashcads::const_iterator TConstIterator;
  typedef TFlashcads::size_type TSize;
  
  struct AddConst
  {
    const Flashcard &operator()( const Flashcard &fc ) const { return fc; }
  };
  
  typedef boost::transform_iterator<AddConst, TIterator, const Flashcard &> TSearchIterator;
  typedef boost::transform_iterator<AddConst, TConstIterator, const Flashcard &> TConstSearchIterator;
  
public:
  CardsStorage();
  
  void Clear()
  {
    m_flashCards.clear();
    
    for( int i = 0; i < Lang::Count; ++i )
      factorSum[i] = 0;
  }
  
  void Add( const Flashcard& fc )
  {
    m_flashCards.push_back(fc);
    
    for( int i = 0; i < Lang::Count; ++i )
      factorSum[i] += fc.factor[i]; 
  }
  template<class Archive>
  void save(Archive &/*ar*/, const unsigned int /*version*/) const
  {
    //Nothing
  }

  template<class Archive>
  void load(Archive &/*ar*/, const unsigned int /*version*/)
  {
    for( int i = 0; i < Lang::Count; ++i )
      factorSum[i] = 0;
  
    for( TConstIterator it = m_flashCards.begin(); it != m_flashCards.end(); ++it )
      for( int i = 0; i < Lang::Count; ++i )
        factorSum[i] += it->factor[i]; 
  }
  
  template<class Archive>
  void serialize(Archive & ar, const unsigned int file_version)
  {
    ar & BOOST_SERIALIZATION_NVP(m_flashCards);
    boost::serialization::split_member(ar, *this, file_version);
  } 
  
  TConstSearchIterator CardsBegin() const { return TConstSearchIterator(m_flashCards.begin()); }
  TConstSearchIterator CardsEnd() const { return TConstSearchIterator(m_flashCards.end()); }
  TSearchIterator CardsBegin() { return TSearchIterator(m_flashCards.begin()); }
  TSearchIterator CardsEnd() { return TSearchIterator(m_flashCards.end()); }
  
  bool IsCardsEmpty() const { return m_flashCards.empty(); }
  TSize GetCardsSize() const { return m_flashCards.size(); }
  
  TSearchIterator GetRandomElem( Lang::T lng );
  
  void ChangeFactor( TIterator it, const Settings &st, Answer::T answer ); 
  double CalcNewFactor( double oldFactor, const Settings &st, Answer::T answer ) const;
  TSize CardsStorage::AttempsCountToReachWeight( double curWeight, double destWeight, const Settings &st ) const; 
  
  void ChangeFactor( TSearchIterator it, const Settings &st, Answer::T answer )
  {
    ChangeFactor( it.base(), st, answer );  
  }
  
  double Score( const Settings &st ) const; 
  
private:
  TFlashcads m_flashCards;
  double factorSum[Lang::Count];
  mutable boost::random::mt19937 m_rng;
};

#endif