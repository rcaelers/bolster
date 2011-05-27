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


template<typename F, typename G, int index, typename... ExtraArgs>
struct FunctionForwarder;

template<typename T, typename Ret, typename... Args, typename CRet, typename... CArgs, int index, typename... ExtraArgs>
class FunctionForwarder<Ret (T::*) (Args...), CRet (*) (CArgs...), index, ExtraArgs...>
{
public:
  typedef Ret (T::*FuncType)(Args...);
  
  FunctionForwarder(T *object, FuncType func, ExtraArgs... extra_args) : object(object), func(func), once(false)
  {
    std::tuple<ExtraArgs...> tuple(extra_args...);
    extra = tuple;
  }

  void set_once(bool once = true)
  {
    this->once = once;
  }
  
  static CRet dispatch(CArgs... args)
  {
    std::tuple<CArgs...> tuple(args...);
    FunctionForwarder *t = (FunctionForwarder *) std::get<index - 1>(tuple);
    std::tuple<CArgs..., ExtraArgs...> xtuple = tuple_cat(tuple, t->extra);

    try
      {
        T *object = t->object;
        FuncType func = t->func;

        if (t->once)
          {
            delete t;
          }
        
        return TupleDispatcher<sizeof...(CArgs) + sizeof...(ExtraArgs), index>::dispatch(object, func, xtuple);
      }
    catch (std::exception &e)
      {
      }
    
    return CRet();
  }
  
private:
  T *object;
  FuncType func;
  std::tuple<ExtraArgs...> extra;
  bool once;
};
