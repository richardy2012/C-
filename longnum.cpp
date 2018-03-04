#include <iostream>
#include <numeric>
#include <string>
#include <algorithm>
#include <iomanip>
#include <stdexcept>
#include <iterator>
#include <memory>
#include <functional>
#include "mylist.h"


using IntList = MyList<int>;
auto EmptyIntList = EmptyList<int>;

int base = 1000;
int highest_unit = base - 1;

IntList make_number(IntList);

bool is_negative(IntList);

IntList fill_low_digit(IntList, int, size_t);
IntList fill_high_digit(IntList, int, size_t);
IntList clip_high_digit(IntList, size_t);
IntList clip_low_digit(IntList, size_t);


IntList shift_left(IntList, size_t);
IntList shift_right(IntList, size_t);
IntList raise_digits(IntList, size_t);

IntList invert(IntList);
IntList negate(IntList);
IntList carry(IntList, int);
int calc_carry(int);
int calc_remain(int);

IntList add(IntList, IntList);
IntList sub(IntList, IntList);
IntList mul(IntList, IntList);
IntList div(IntList, IntList);

IntList make_number(IntList lst)
{
  if (Any([](int x) -> bool { return x < 0; }, lst->tail())) {
    throw std::runtime_error("###### 操作数输入错误：符号错误。");
  }
  else if (Any([=](int x) -> bool { return abs(x) >= base; }, lst)) {
    throw std::runtime_error("###### 操作数输入错误：每单元最多包含3位数。");
  }
  else {
    return (lst->head() >= 0) ?
      Reverse(Cons(0, lst)) :
      negate(make_number(Cons(- lst->head(), lst->tail())));
  }
}

bool is_negative(IntList n)
{
  int sign_digit = Reverse(n)->head();
  return sign_digit >= (base / 2);
}

IntList fill_low_digit(IntList n, int filler, size_t count)
{
  return (count == 0) ? n : Cons(filler, fill_low_digit(n, filler, count - 1));
}
IntList fill_high_digit(IntList n, int filler, size_t count)
{
  return Reverse(fill_low_digit(Reverse(n), filler, count));
}
IntList clip_high_digit(IntList n, size_t count)
{
  return ListHead(n, Length(n) - count);
}
IntList clip_low_digit(IntList n, size_t count)
{
  return ListTail(n, count);
}



IntList shift_left(IntList n, size_t count)
{
  return fill_low_digit(clip_high_digit(n, count), 0, count);
}
IntList shift_right(IntList n, size_t count)
{
  return raise_digits(clip_low_digit(n, count), Length(n));
}


IntList raise_digits(IntList n, size_t target_digits_length)
{
  auto pre_len{Length(n)};
  if (pre_len > target_digits_length) {
    return n;
  }
  else {
    auto count{target_digits_length - pre_len};
    return is_negative(n) ?
      fill_high_digit(n, highest_unit, count) :
      fill_high_digit(n, 0, count);
  }
}

IntList invert(IntList n)
{
  return FoldRight([](int x, IntList xs) { return Cons(highest_unit - x, xs); }, 
                       EmptyIntList, n);
}
IntList negate(IntList n)
{
  return carry(invert(n), 1);
}

int calc_carry(int n) { return n / base; }
int calc_remain(int n) { return n % base; }
IntList carry(IntList n, int x)
{
  if (x == 0) {
    return n;
  }
  else if (n == EmptyIntList) {
    return EmptyIntList;
  }
  else {
    auto neW{n->head() + x};
    auto new_last_digit{calc_remain(neW)};
    auto new_carry{calc_carry(neW)};
    return Cons(new_last_digit, carry(n->tail(), new_carry));
  }
}



IntList add(IntList x, IntList y)
{
  auto x_len{Length(x)};
  auto y_len{Length(y)};
  if (x_len != y_len) {
    auto higher_digits_length = x_len > y_len ? x_len : y_len;
    return add(raise_digits(x, higher_digits_length),
               raise_digits(y, higher_digits_length));
  }
  else if (x == EmptyIntList && y == EmptyIntList) {
    return EmptyIntList;
  }
  else {
    auto neW{x->head() + y->head()};
    auto new_last_digit{calc_remain(neW)};
    auto new_carry{calc_carry(neW)};
    return Cons(new_last_digit, add(carry(x->tail(), new_carry), y->tail()));
  } 
}

IntList sub(IntList x, IntList y)
{
  return add(x, negate(y));
}

IntList mul(IntList x, IntList y)
{  
  auto mul_digit_by_all_digits = [](int x, IntList xs) {
      return PoorMap([=](int y) { return y * x; }, xs);
    };

  auto mul_digits_by_all_digits = [&mul_digit_by_all_digits](IntList y, IntList x) {
      return FoldRight([=, &mul_digit_by_all_digits](int term, IntList xs) {
          return add(mul_digit_by_all_digits(term, y),
                     shift_left(xs, 1));
      }, make_number(toList({0})), x);
    };
  
  auto new_digits_length{Length(x) + Length(y) - 1};
  auto pre_mul = [=](IntList x, IntList y) { 
      return mul_digits_by_all_digits( raise_digits(x, new_digits_length), y);
    };

  auto remain_term = [=](IntList t1, IntList t2) {
      return !(is_negative(t2)) ?
        make_number(toList({0})) :
        negate( shift_left( raise_digits(t1, new_digits_length),
                            Length(t2)));
    };

  return add(pre_mul(x, y), remain_term(x, y));
}

IntList div(IntList x, IntList y)
{
  if (Length(Filter([](int x) { return x != 0; }, y)) == 0) {
    throw std::runtime_error("###### 除零错误。");
  }

  std::function<int(IntList, IntList)> quo_prime = 
  [&quo_prime](IntList t1, IntList t2) {
    auto remain{sub(t1, t2)};
    return is_negative(remain) ? 
      0 : (1 + quo_prime(remain, t2));
  };
  std::function<IntList(IntList, IntList)> rem_prime = 
  [&rem_prime](IntList t1, IntList t2) {
    auto remain{sub(t1, t2)};
    return is_negative(remain) ?
      add(remain, t2) : rem_prime(remain, t2);
  };


  std::function<int(IntList, IntList)> calc_digits = 
  [&calc_digits](IntList t1, IntList t2) {
    return is_negative(sub(t1, t2))?
      0 : (1 + calc_digits(t1, shift_left(t2, 1)));
  };

  auto div_posi = [&calc_digits, &quo_prime, &rem_prime](IntList t1, IntList t2) {
    auto raised_t2 = raise_digits(t2, Length(t1));
    auto digits_length = calc_digits(t1, raised_t2);

    auto result = make_number(toList({0}));
    for ( auto dividend = t1
        , divisor = shift_left(raised_t2, digits_length - 1)
        ; digits_length != 0
        ; digits_length--)
        {
          result = Cons(quo_prime(dividend, divisor), result);
          dividend = rem_prime(dividend, divisor);
          divisor = shift_right(divisor, 1);
        }
    return result;
  };


  if (is_negative(x) && is_negative(y)) {
    return div_posi(negate(x), negate(y));
  }
  else if (is_negative(x)) {
    return negate(div_posi(negate(x), y));
  }
  else if (is_negative(y)) {
    return negate(div_posi(x, negate(y)));
  }
  else {
    return div_posi(x, y);
  }
}

IntList Parser(std::string s)
{
  std::string delimiter{","};

  IntList result{EmptyIntList};

  size_t pos{0};
  std::string token;
  while ((pos = s.find(delimiter)) != std::string::npos) {
      token = s.substr(0, pos);
      result = Cons(stoi(token), result);
      s.erase(0, pos + delimiter.length());
  }
  
  return Reverse(Cons(stoi(s), result));
}

IntList Eval(char op, IntList x, IntList y)
{
  switch (op) {
    case '+': return add(x, y); break;
    case '-': return sub(x, y); break;
    case '*': return mul(x, y); break;
    case '/': return div(x, y); break;
    default: return EmptyIntList; break;
  }
}

void PrettyPrint(char op, IntList left, IntList right, IntList result)
{
  auto format_num = [](IntList n) -> IntList {
    auto remove_zero = [](IntList n) -> IntList {
      auto a = n;
      for (
          ; (a->head() == 0) && (a->tail() != EmptyIntList)
          ; a = a->tail()
      ) {}
      return a;
    };
    if (is_negative(n)) {
      auto posi = remove_zero(Reverse(negate(n)));
      return Cons(- posi->head(), posi->tail());
    }
    else {
      return remove_zero(Reverse(n));
    }
  };
  auto l = format_num(left);
  auto r = format_num(right);
  auto res = format_num(result);
  auto len = std::max({Length(l), Length(r), Length(res)});

  auto print_num = [](IntList n, size_t len) -> void {
    auto print_digit = [](int n, size_t width) -> void {
      std::cout << std::setfill('0') << std::setw(width) << n;
    };

    std::cout << std::setfill(' ') << std::setw(4 * (len - Length(n)) + 1) << ' ';
    std::cout << std::setfill(' ') << std::setw(4) << n->head();
    while (n->tail() != EmptyIntList) {
      std::cout << " ";
      n = n->tail();
      print_digit(n->head(), 3);
    }
  };

  std::cout << std::endl << " ";  
  print_num(l, len);
  std::cout << std::endl << op;
  print_num(r, len);
  std::cout << std::endl << std::setfill('-') << std::setw(4 * (len + 1)) << ' ';
  std::cout << std::endl << " ";
  print_num(res, len);
  std::cout << std::endl;
}

void Engine()
{
  std::cout << "\n\n请输入运算符(+ - * /)：";
  char op;
  std::cin >> op;
  std::string junk;
  std::getline(std::cin, junk, '\n');
  if (op != '+' && op != '-' && op != '*' && op != '/' ) {
    throw std::runtime_error("###### 操作符解析错误：应为 + 或 - 或 * 或 /。");
  }

  std::cout << "请输入第一个操作数 (示例：-123, 456, 789 )：";
  std::string s1{""};
  std::getline(std::cin, s1, '\n');
  auto n1 = make_number(Parser(s1));

  std::cout << "请输入第二个操作数 (示例：-123, 456, 789 )：";
  std::string s2{""};
  std::getline(std::cin, s2, '\n');  
  auto n2 = make_number(Parser(s2));

  if (Length(n1) <= 0 || Length(n2) <= 0) {
    throw std::runtime_error("###### 操作数解析错误，格式应为：-123, 456, 789");
  }

  auto res = Eval(op, n1, n2);
  PrettyPrint(op, n1, n2, res);
}

int main()
{
  while (1) {
    try {
      Engine();
    }
    catch (std::runtime_error err) {
      std::cerr << "\n" << err.what() << "\n";
    }
    std::cout << "\n\n继续输入？请键入 y 或 n 进行确认：";
    char c;
    if (!(std::cin >> c) || c == 'n') break;
  }
  return 0;
}
