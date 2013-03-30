#include "stdafx.h"
#include "model.h"
#include <boost/random/uniform_real_distribution.hpp>

CardsStorage::TSearchIterator CardsStorage::GetRandomElem( Lang::T lng )
{
  boost::random::uniform_real_distribution<> dist( 0.0, factorSum[lng] );
  double randValue = dist(m_rng);
  TIterator it = m_flashCards.begin();
  
  for( ; it != m_flashCards.end() && it->factor[lng] <= randValue; ++it )
    randValue -= it->factor[lng];  
  
  return TSearchIterator(it);
}
double CardsStorage::CalcNewFactor( double oldFactor, const Settings &st, Answer::T answer ) const
{
  // Maple:
  //   restart;
  //   Digits := 20:
  //   x1 := 3:
  //   x2 := 10:
  //   m := 50:
  //   for i while i < 100 do
  //   print( i, x1, x2 );
  //     x1 := x1 + (m - x1) * 0.5:
  //     x2 := x2 - 0.2 * (x2 - 1):  
  //   end do:
  return answer == Answer::Incorrect?
    //Если был дан неправильный ответ, то нужно сделать скачок в сторону увеличения вероятности
    //в зависимости от того, насколько эта вероятность мала в данный момент.
    //Т.е. мы моделируем ситуацию, когда пользователь выучил слово затем оно долго не попадалось и
    //он его забыл 
    oldFactor + ( st.MaxWeight(GetCardsSize()) - oldFactor ) * st.IncorrectAnswerFactor():
    
    //Если был дан правильный ответ, то мы должны постепенно двигаться к единице и 
    //при этом, нам очень важно то, что количество шагов, которые мы сделаем при стремлении
    //к единице должно быть приемлемым для пользователя как при значении oldFactor 
    //равном 2 так и при 10000
    oldFactor - st.CorrectAnswerFactor() * ( oldFactor - 1 );
}

void CardsStorage::ChangeFactor( TIterator it, const Settings &st, Answer::T answer )
{
  const double oldFactor = it->factor[st.Language()];
  const double newFactor = CalcNewFactor( oldFactor, st, answer );
   
  factorSum[st.Language()] += newFactor - oldFactor;
  it->factor[st.Language()] = newFactor;
  it->attempts[st.Language()] += 1;
}

CardsStorage::TSize CardsStorage::AttempsCountToReachWeight( double curWeight, double destWeight, const Settings &st ) const
{
#if 0  
  // Maple:
  // F:=oldFactor - CorrectFactor * (oldFactor);
  // FP := unapply( collect(F, oldFactor ), oldFactor );
  // (FP@@6)(X);
  // 
  // (1 - CorrectFactor)^times*cur = dest;
  // solve( {%}, {times} );
  // evalf(subs( {dest=40, cur=300, CorrectFactor=0.75}, %));
  const double times = std::log( destWeight / curWeight ) / std::log(1 - st.CorrectAnswerFactor());
  
  return times > 0 ? (TSize) std::ceil(times) : 0;

#else //Так намного проще и точнее
  
  TSize result = 0;
  
  while( curWeight > destWeight )
  {
    curWeight = CalcNewFactor( curWeight, st, Answer::Correct );
    ++result;
  }
  
  return result; 
  
#endif
}                                                          

CardsStorage::CardsStorage():
  m_rng( static_cast<unsigned int>(std::time(0)) ) 
{
  Clear();
}

double CardsStorage::Score( const Settings &st ) const
{
  if( IsCardsEmpty() )
    return 0.0;

  const double avg = factorSum[st.Language()] / GetCardsSize();
  return std::floor( (st.MaxWeight( GetCardsSize() ) - avg) * 100 + 0.5 ); 
}