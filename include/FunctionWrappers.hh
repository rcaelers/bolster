#include <tuple>

template <int Index, int SkipIndex, class T, class F1, class F2, class... ExtraArgs>
struct TypeGeneratorSkipArgument;

template <class T, class F1, class... ExtraArgs>
struct TypeGeneratorAddExtraArgs;

template <class T, class F1, class F2>
struct TypeGeneratorReverseArguments;

template <int SkipIndex, class T,  class F, class... ExtraArgs>
struct TypeGenerator;

template <int SkipIndex, class T, class Ret, class... Args, class... ExtraArgs>
struct TypeGenerator<SkipIndex, T, Ret (*) (Args...), ExtraArgs...>
  : public TypeGeneratorSkipArgument<sizeof...(Args), sizeof...(Args) - SkipIndex + 1, T, Ret (*) (), Ret (*) (Args...), ExtraArgs...>
{
};

template <int Index, int SkipIndex, class T, class Ret, class... Args1, class Args2Head, class... Args2Tail, class... ExtraArgs>
struct TypeGeneratorSkipArgument<Index, SkipIndex, T, Ret (*) (Args1...), Ret (*) (Args2Head, Args2Tail...), ExtraArgs...>
  : public TypeGeneratorSkipArgument<Index - 1, SkipIndex, T, Ret (*) (Args2Head, Args1...), Ret (*) (Args2Tail...), ExtraArgs...>
{
};

template <int SkipIndex, class T, class Ret, class... Args1, class Arg2Head, class... Args2Tail, class... ExtraArgs>
struct TypeGeneratorSkipArgument<SkipIndex, SkipIndex, T, Ret (*) (Args1...), Ret (*) (Arg2Head, Args2Tail...), ExtraArgs...>
  : public TypeGeneratorSkipArgument<SkipIndex - 1, SkipIndex, T, Ret (*) (Args1...), Ret (*) (Args2Tail...), ExtraArgs...>
{
};

template <int SkipIndex, class T, class Ret, class... Args1, class... ExtraArgs>
struct TypeGeneratorSkipArgument<0, SkipIndex, T, Ret (*) (Args1...), Ret (*) (), ExtraArgs...>
  : TypeGeneratorAddExtraArgs<T, Ret (*) (Args1...), ExtraArgs...>
{
};

template <class T, class Ret, class... Args1, class ExtraArgsHead, class... ExtraArgsTail>
struct TypeGeneratorAddExtraArgs<T, Ret (*) (Args1...), ExtraArgsHead, ExtraArgsTail...>
  : TypeGeneratorAddExtraArgs<T, Ret (*) (ExtraArgsHead, Args1...), ExtraArgsTail...>
{
};

template <class T, class Ret, class... Args1>
struct TypeGeneratorAddExtraArgs<T, Ret (*) (Args1...)>
  : TypeGeneratorReverseArguments<T, Ret (*) (Args1...), Ret (*) ()>
{
};

template <class T, class Ret, class Args1Head, class... Args1Tail, class... Args2>
struct TypeGeneratorReverseArguments<T, Ret (*) (Args1Head, Args1Tail...), Ret (*) (Args2...)>
  : public TypeGeneratorReverseArguments<T, Ret (*) (Args1Tail...), Ret (*) (Args1Head, Args2...)>
{
};

template <class T, class Ret, class... Args2>
struct TypeGeneratorReverseArguments<T, Ret (*) (), Ret (*) (Args2...)>
{
  typedef Ret (T::*Type)(Args2...);
};


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
