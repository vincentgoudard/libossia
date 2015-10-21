#include "Editor/JamomaCurve.h"
#include "Editor/CurveSegment/JamomaCurveSegmentLinear.h"

#include <iostream> //! \todo to remove. only here for debug purpose

using namespace OSSIA;
using namespace std;

# pragma mark -
# pragma mark Life cycle

namespace OSSIA
{
  // explicit instantiation for double and bool
  template class Curve<double, bool>;
  
  template <>
  shared_ptr<Curve<double, bool>> Curve<double, bool>::create()
  {
    return make_shared<JamomaCurve<double, bool>>();
  }
  
  // explicit instantiation for double and int
  template class Curve<double, int>;
  
  template <>
  shared_ptr<Curve<double, int>> Curve<double, int>::create()
  {
    return make_shared<JamomaCurve<double, int>>();
  }
  
  // explicit instantiation for double and float
  template class Curve<double, float>;
  
  template <>
  shared_ptr<Curve<double, float>> Curve<double, float>::create()
  {
    return make_shared<JamomaCurve<double, float>>();
  }
}

template <typename X, typename Y>
JamomaCurve<X,Y>::
JamomaCurve() :
mInitialDestination(nullptr)
{
  mInitialDestinationIndex.push_back(0);
}

template <typename X, typename Y>
JamomaCurve<X,Y>::
JamomaCurve(const JamomaCurve * other)
{}

template <typename X, typename Y>
shared_ptr<Curve<X,Y>> JamomaCurve<X,Y>::
clone() const
{
  return make_shared<JamomaCurve>(this);
}

template <typename X, typename Y>
JamomaCurve<X,Y>::
~JamomaCurve()
{}

# pragma mark -
# pragma mark Edition

template <typename X, typename Y>
bool JamomaCurve<X,Y>::
addPoint(X abscissa, Y value, shared_ptr<CurveSegment<Y>> segment)
{
  pair<Y,shared_ptr<CurveSegment<Y>>> p(value, segment);
  
  //! \todo check if there is already a point
  
  mPointsMap.emplace(abscissa, p);
  
  return true;
}

template <typename X, typename Y>
bool JamomaCurve<X,Y>::
removePoint(X abscissa)
{
  return mPointsMap.erase(abscissa) > 0;
}

# pragma mark -
# pragma mark Execution

template <typename X, typename Y>
Y JamomaCurve<X,Y>::
valueAt(X abscissa) const
{
  TimeValue lastAbscissa(0.);
  Y lastValue = getInitialValue();
  
  for (auto it = mPointsMap.begin(); it != mPointsMap.end(); it++)
  {
    if (abscissa > lastAbscissa &&
        abscissa <= it->first)
    {      
      lastValue = it->second.second->valueAt((abscissa - lastAbscissa) / (it->first - lastAbscissa), lastValue , it->second.first);
      break;
    }
    else if (abscissa > it->first)
    {
      lastAbscissa = it->first;
      lastValue = it->second.first;
    }
    else
      break;
  }
  
  return lastValue;
}

# pragma mark -
# pragma mark Accessors

template <typename X, typename Y>
Y JamomaCurve<X,Y>::
getInitialValue() const
{
  if (mInitialDestination == nullptr)
  {
    return mInitialValue;
  }
  else
  {
    auto address = mInitialDestination->value->getAddress();
    
    if (!address)
      throw runtime_error("getting an address value using from a destination without address");
    
    char level = 0;
    return convertToTemplateTypeValue(address->pullValue(), &level);
  }
}

template <typename X, typename Y>
void JamomaCurve<X,Y>::
setInitialValue(Y value)
{
  mInitialValue = value;
}

template <typename X, typename Y>
const Destination* JamomaCurve<X,Y>::
getInitialDestination() const
{
  return mInitialDestination;
}

template <typename X, typename Y>
void JamomaCurve<X,Y>::
setInitialDestination(const Destination* destination)
{
  mInitialDestination = static_cast<Destination*>(destination->clone());
}

template <typename X, typename Y>
vector<char> JamomaCurve<X,Y>::
getInitialDestinationIndex() const
{
  return mInitialDestinationIndex;
}

template <typename X, typename Y>
void JamomaCurve<X,Y>::
setInitialDestinationIndex(std::initializer_list<char> index)
{
  mInitialDestinationIndex.clear();
  
  for (const auto & i : index)
    mInitialDestinationIndex.push_back(i);
}

template <typename X, typename Y>
map<X, pair<Y, shared_ptr<CurveSegment<Y>>>> JamomaCurve<X,Y>::
getPointsMap() const
{
  return mPointsMap;
}

# pragma mark -
# pragma mark Implementation specific

template <typename X, typename Y>
Y JamomaCurve<X,Y>::
convertToTemplateTypeValue(const Value * value, char* level) const
{
  switch (value->getType())
  {
    case Value::Type::BOOL :
    {
      auto b = static_cast<const Bool*>(value);
      return b->value;
    }
      
    case Value::Type::INT :
    {
      auto i = static_cast<const Int*>(value);
      return i->value;
    }
      
    case Value::Type::FLOAT :
    {
      auto f = static_cast<const Float*>(value);
      return f->value;
    }
      
    case Value::Type::CHAR :
    {
      auto c = static_cast<const Char*>(value);
      return c->value;
    }
    
    case Value::Type::TUPLE :
    {
      auto t = static_cast<const Tuple*>(value);
      
      char index = mInitialDestinationIndex[*level];
      (*level)++;

      return convertToTemplateTypeValue(t->value[index], level);
    }
      
    case Value::Type::GENERIC :
    {
      //! \todo GENERIC case
    }
      
    default :
    {
      throw runtime_error("converting none numerical value");
    }
  }
}