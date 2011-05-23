#include <tuple>

// TODO: add return type to TupleDispatcher

template <int Index, int SkipIndex>
struct TupleDispatcher
{
  template <typename T, typename... FuncArgs, typename... TupleArgs, typename... Args>
  static void dispatch(T *object,
                   void (T::*func)(FuncArgs...),
                   const std::tuple<TupleArgs...> &tuple,
                   Args... args)
  {
    TupleDispatcher<Index-1, SkipIndex>::dispatch(object, func, tuple, std::get<Index-1>(tuple), args...);
  }
};

template <int SkipIndex>
struct TupleDispatcher<SkipIndex, SkipIndex>
{
  template <typename T, typename... FuncArgs, typename... TupleArgs, typename... Args>
  static void dispatch(T* object,
                   void (T::*func)(FuncArgs...),
                   const std::tuple<TupleArgs...> &tuple,
                   Args... args)
  {
    TupleDispatcher<SkipIndex-1, SkipIndex>::dispatch(object, func, tuple, args...);
  }
};

template <const int SkipIndex>
struct TupleDispatcher<0, SkipIndex>
{
  template <typename T, typename... FuncArgs, typename... TupleArgs, typename... Args>
  static void dispatch(T* object,
                   void (T::*func)(FuncArgs...),
                   const std::tuple<TupleArgs...> &,
                   Args... args)
  {
    (object->*func)(args...);
  }
};


template <typename... Args>
class CWrapper
{
public:

  template<int index, typename T>
  struct Dispatch;
  
  template<int index, typename Ret, typename... CArgs>
  struct Dispatch<index,  Ret (*) (CArgs...)>
  {
    static void dispatch(CArgs... args)
    {
    try
      {
        std::tuple<CArgs...> tuple(args...);
        CWrapper * t = (CWrapper *) std::get<index - 1>(tuple);
        TupleDispatcher<sizeof...(CArgs), index>::dispatch(t, &CWrapper::operator(), tuple);
      }
    catch (std::exception &e)
      {
      }
    }
  };

  virtual void operator()(Args... args) = 0;
};
