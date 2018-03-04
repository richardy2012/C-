#ifndef _FUNC_H_
#define _FUNC_H_
#include <memory>
#include <iterator>

template <typename T>
class MyNode
{
public:
  MyNode(T a, std::shared_ptr<MyNode> b) : n(a), next(b){};
  T head() { return n; }
  std::shared_ptr<MyNode> tail() { return next; }

private:
  T n;
  std::shared_ptr<MyNode> next;
};

template <typename T>
using MyList = std::shared_ptr<MyNode<T>>;

template <typename T>
MyList<T> EmptyList = nullptr;

template <typename T>
MyList<T> Cons(T x, MyList<T> xs);

template <typename T>
MyList<T> toList(const std::initializer_list<T> &il);

template <typename F, typename Z, typename T>
Z FoldRight(F fn, Z zero, MyList<T> xs);

template <typename F, typename Z, typename T>
Z FoldLeft(F fn, Z zero, MyList<T> xs);

template <typename T>
MyList<T> Reverse(MyList<T> xs);

template <typename T>
MyList<T> Append(MyList<T> l, MyList<T> r);

template <typename T, typename F>
MyList<T> Map(F fn, MyList<T> xs);

template <typename F, typename T>
MyList<T> Filter(F fn, MyList<T> xs);

template <typename T>
size_t Length(MyList<T> xs);

template <typename T>
MyList<T> ListTail(MyList<T> xs, size_t n);

template <typename T>
MyList<T> ListHead(MyList<T> xs, size_t n);

template <typename F, typename T>
bool Any(F pred, MyList<T> lst);




// * 构造List的基础函数
template <typename T>
MyList<T> Cons(T x, MyList<T> xs)
{
  return std::make_shared<MyNode<T>>(x, xs);
}
// * 构造List的语法糖
template <typename T>
MyList<T> toList(const std::initializer_list<T> &il)
{
  MyList<T> res = EmptyList<T>;
  for (auto it = std::rbegin(il); it != std::rend(il); ++it)
    res = Cons(*it, res);
  return res;
}

// * 左右折叠函数
// * https://en.wikipedia.org/wiki/Fold_(higher-order_function)
template <typename F, typename Z, typename T>
Z FoldRight(F fn, Z zero, MyList<T> xs)
{
  if (xs == EmptyList<T>)
    return zero;
  else
    return fn(xs->head(), FoldRight(fn, zero, xs->tail()));
}
template <typename F, typename Z, typename T>
Z FoldLeft(F fn, Z zero, MyList<T> xs)
{
  if (xs == EmptyList<T>)
    return zero;
  else
    return FoldLeft(fn, fn(zero, xs->head()), xs->tail());
}

// * 反转
template <typename T>
MyList<T> Reverse(MyList<T> xs)
{
  return FoldLeft([](MyList<T> xs, T x) -> MyList<T> { return Cons<T>(x, xs); } , 
                  EmptyList<T>, xs);
}

// * 连接
template <typename T>
MyList<T> Append(MyList<T> l, MyList<T> r)
{
	return FoldRight([](T x, MyList<T> xs) -> MyList<T> { return Cons<T>(x, xs); },
		r, l);
}

// * Map
template <typename T, typename F>
MyList<T> PoorMap(F fn, MyList<T> xs)
{
  return FoldRight([&fn](T x, MyList<T> xs) -> MyList<T> { return Cons<T>(fn(x), xs); },
                   EmptyList<T>, xs);
}

// * Filter
template <typename F, typename T>
MyList<T> Filter(F fn, MyList<T> xs)
{
  return FoldRight([&fn](T x, MyList<T> xs) -> MyList<T> { return (fn(x) ? Cons<T>(x, xs) : xs); },
                   EmptyList<T>, xs);
}

// * Length
template <typename T>
size_t Length(MyList<T> xs)
{
  return FoldLeft( [](size_t x, T rest) { return (x + 1); },
                   0, xs);
}

// * list-tail
template <typename T>
MyList<T> ListTail(MyList<T> xs, size_t n)
{
  return ( n == 0 ) ? xs : ListTail(xs->tail(), n - 1) ;
}

// * list-head
template <typename T>
MyList<T> ListHead(MyList<T> xs, size_t n)
{
  return ( n == 0 ) ? EmptyList<T> : Cons(xs->head(), ListHead(xs->tail(), n - 1)) ;
}

// * any
template <typename F, typename T>
bool Any(F pred, MyList<T> lst)
{
  return FoldLeft( [&pred](bool x, T rest) { return pred(rest) || x; }, 
                   false, lst);
}

#endif