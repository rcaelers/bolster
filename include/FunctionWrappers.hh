#include <tuple>

template <int Index, int SkipIndex>
struct TupleDispatcher
{
  template <typename T, typename Ret, typename... FuncArgs, typename... TupleArgs, typename... Args>
  static Ret dispatch(T *object,
                       Ret (T::*func)(FuncArgs...),
                       const std::tuple<TupleArgs...> &tuple,
                       Args... args)
  {
    return TupleDispatcher<Index-1, SkipIndex>::dispatch(object, func, tuple, std::get<Index-1>(tuple), args...);
  }
};

template <int SkipIndex>
struct TupleDispatcher<SkipIndex, SkipIndex>
{
  template <typename T, typename Ret, typename... FuncArgs, typename... TupleArgs, typename... Args>
  static Ret dispatch(T* object,
                      Ret (T::*func)(FuncArgs...),
                      const std::tuple<TupleArgs...> &tuple,
                      Args... args)
  {
    return TupleDispatcher<SkipIndex-1, SkipIndex>::dispatch(object, func, tuple, args...);
  }
};

template <int SkipIndex>
struct TupleDispatcher<0, SkipIndex>
{
  template <typename T, typename Ret, typename... FuncArgs, typename... TupleArgs, typename... Args>
  static Ret dispatch(T* object,
                      Ret (T::*func)(FuncArgs...),
                      const std::tuple<TupleArgs...> &,
                      Args... args)
  {
    return (object->*func)(args...);
  }
};


template <typename Ret, typename... Args>
class FunctionWrapper
{
public:
  template<int index, typename F>
  struct Dispatch;

  template<int index, typename CRet, typename... CArgs>
  struct Dispatch<index,  CRet (*) (CArgs...)>
  {
    static CRet dispatch(CArgs... args)
    {
    try
      {
        std::tuple<CArgs...> tuple(args...);
        FunctionWrapper * t = (FunctionWrapper *) std::get<index - 1>(tuple);
        return TupleDispatcher<sizeof...(CArgs), index>::dispatch(t, &FunctionWrapper::operator(), tuple);
      }
    catch (std::exception &e)
      {
      }
    return CRet();
    }
  };
  
  virtual Ret operator()(Args... args) = 0;
};


template<typename F, typename... ExtraArgs>
struct FunctionForwarder;

template<typename T, typename Ret, typename... Args, typename... ExtraArgs>
class FunctionForwarder<Ret (T::*) (Args...), ExtraArgs...>
{
public:
  typedef Ret (T::*FuncType)(Args...);
  
  FunctionForwarder(T *object, FuncType func, ExtraArgs... extra_args) : object(object), func(func)
  {
    std::tuple<ExtraArgs...> tuple(extra_args...);
    extra = tuple;
  }
  
  template<int index, typename F>
  struct Dispatch;

  template<int index, typename CRet, typename... CArgs>
  struct Dispatch<index,  CRet (*) (CArgs...)>
  {
    static CRet dispatch(CArgs... args)
    {
    try
      {
        std::tuple<CArgs...> tuple(args...);
        FunctionForwarder * t = (FunctionForwarder *) std::get<index - 1>(tuple);

        std::tuple<CArgs..., ExtraArgs...> xtuple = tuple_cat(tuple, t->extra);

        return TupleDispatcher<sizeof...(CArgs) + sizeof...(ExtraArgs), index>::dispatch(t->object, t->func, xtuple);
      }
    catch (std::exception &e)
      {
      }

    return CRet();
    }
  };
  
private:
  T *object;
  FuncType func;
  std::tuple<ExtraArgs...> extra;
};
