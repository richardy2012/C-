#include <iostream>
#include <memory>
#include <vector>
#include <random>
#include <string>
#include <vector>
#include <utility>
#include <stdexcept>
#include <unordered_set>
#include "mylist.h"

std::random_device rd;
std::mt19937 gen(rd());
int rand_int(int from, int to)
{
    return std::uniform_int_distribution<int>(from, to - 1)(gen);
}
struct pair_hash { 
  inline size_t operator()(const std::pair<int, int> &v) const {
    std::hash<int> int_hasher;
    return int_hasher(v.first) ^ int_hasher(v.second);
  }
};


using Cor = std::pair<int, int>;
using CorP = std::shared_ptr<std::pair<int, int>>;

using Way = MyList<CorP>;
auto EmptyCorList = EmptyList<CorP>;
using Way = Way;

using MyWayList = MyList<Way>;
auto EmptyWayList = EmptyList<Way>;

auto C = [](int x, int y) {
  return std::make_shared<Cor>(x, y);
};



class Maze
{
public:
  Maze() = delete;
  Maze(size_t width, size_t height) : width(width)
                                    , height(height)
                                    , xbound(width * 2 + 1)
                                    , ybound(height * 2 + 1)
                                    { GenMaze(); }

  void Run() {
    auto Render = [&](CorP n) {
      maze[n->second][n->first] = '0';
    };
    for ( auto x = find_way(toList({ toList({ start }) }))
        ; x != EmptyCorList
        ; x = x->tail()) { Render(x->head()); }
    Display();
  }

  void Display() {
    for (const auto& y : maze) {
      for (const auto& x : y) {
        std::cout << x;
      }
      std::cout << std::endl;
    }
  }

private:
  const int width;
  const int height;
  const int xbound;
  const int ybound;
  CorP destination;
  CorP start;
  std::vector<std::vector<char>> maze;
  std::vector<std::vector<char>> been;

  Way find_way(MyWayList ways) {
    auto shortest_way{find_shortest_way(ways)};
    if (!(shortest_way == EmptyCorList)) {
      return shortest_way;
    }
    else {
      auto poor_new_ways{EmptyWayList};
      for( auto old_ways = ways
          ; old_ways != EmptyWayList
          ; poor_new_ways = Append(next_steps(old_ways->head()), poor_new_ways)
          , old_ways = old_ways->tail()) {}

      return find_way(Filter([this](Way way) { return is_forwardable(way->head()); },
                      poor_new_ways));    
    }
  }
  

  MyWayList next_steps(Way way) {
    auto last_pstion{way->head()};

    auto d{Cons(C(last_pstion->first + 1, last_pstion->second), way)};
    auto u{Cons(C(last_pstion->first - 1, last_pstion->second), way)};
    auto l{Cons(C(last_pstion->first, last_pstion->second - 1), way)};
    auto r{Cons(C(last_pstion->first, last_pstion->second + 1), way)};

    return toList({ d, u, l, r });
  }

  Way find_shortest_way(MyWayList ways) {
    auto is_reached = [=](Way way) -> bool {
      auto step = way->head();
      return (step->first == destination->first &&
        step->second == destination->second);
    };

    auto reachs{Filter(is_reached, ways)};
    return (reachs == EmptyWayList) ?
      EmptyCorList : reachs->head();
  }


  bool is_forwardable(CorP step) {
    auto not_out_of_bound = [=]() -> bool {
      auto x_{step->first};
      auto y_{step->second};
      return (x_ < xbound) && (y_ < ybound) && (x_ >= 0) && (y_ >= 0);
    };

    auto not_wall = [=]() -> bool {
      return maze[step->second][step->first] == ' ';
    };

    auto not_been = [=]() -> bool {
      auto a = been[step->second][step->first] == 'f';
      if (a) { been[step->second][step->first] = 't'; }
      return a;
    };

    return not_out_of_bound() && not_wall() && not_been();
  }

  
  void GenMaze() {
    
    maze.resize(height * 2 + 1);
    been.resize(height * 2 + 1);
    for (auto& x : maze) { x.resize(width * 2 + 1, '#'); }
    for (auto& x : been) { x.resize(width * 2 + 1, 'f'); }		
    destination = C(width * 2, rand_int(0, height) * 2 + 1);
    start = C(0, rand_int(0, height) * 2 + 1);
    
    for (int i = 1; i < height * 2 + 1; i = i + 2) {
      for (int j = 0; j < width * 2 - 1; j = j + 2) {
        maze[i][j] = '|';
        maze[i][j+1] = ' '; 
      }
      maze[i][width*2] = '|';
    }
    for (int i = 0; i < height * 2 + 1; i = i + 2) {
      for (int j = 0; j < width * 2 - 1; j = j + 2) {
        maze[i][j] = '+';
        maze[i][j+1] = '-'; 
      }
      maze[i][width*2] = '+';
    }

    maze[destination->second][destination->first] = ' ';
    maze[start->second][start->first] = '0';
    
    using CorSet = std::unordered_set<Cor, pair_hash>;

    auto filterAround = [=](const Cor& a, CorSet& tbl) {
      auto res = std::make_shared<std::vector<Cor>>();
      if (a.first - 1 >= 0) {
        auto left = Cor(a.first - 1, a.second);
        if (tbl.find(left) != tbl.end()) {  res->push_back(left); }
      }
      if (a.first + 1 < width) {
        auto right = Cor(a.first + 1, a.second);
        if (tbl.find(right) != tbl.end()) res->push_back(right);
      }
      if (a.second - 1 >= 0) {
        auto up = Cor(a.first, a.second - 1);
        if (tbl.find(up) != tbl.end()) res->push_back(up);
      }
      if (a.second + 1 < height) {
        auto down = Cor(a.first, a.second + 1);
        if (tbl.find(down) != tbl.end()) res->push_back(down);
      }
      return res;
    };

    auto pullDown = [this](const Cor& a, const Cor& b) -> void {
      auto a_c = a.first * 2 + 1 + (b.first - a.first);
      auto b_c = a.second * 2 + 1 + (b.second - a.second);
      maze[b_c][a_c] = ' ';
    };

    CorSet un;
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        un.insert( Cor(x, y) );
      }
    }
    CorSet ed;
    CorSet ing;


    Cor a = *(std::next(un.begin(), rand_int(0, un.size())));
    ed.insert(a);
    un.erase(a);

    auto b{filterAround(a, un)};
    for ( const auto& x : *b ) { 
      ing.insert(x);
      un.erase(x);
    }

    while (!ing.empty()) {
      Cor c = *(std::next(ing.begin(), rand_int(0, ing.size())));
      auto d =  filterAround(c, ed);

      Cor e = d->at(rand_int(0, d->size()));
      pullDown(c, e);
      ing.erase(c);
      ed.insert(c);

      auto f{filterAround(c, un)};
      for ( const auto& x : *f ) { 
        ing.insert(x);
        un.erase(x);
      }
    }
  }
};

int GetXnY() {
  int x{};
  std::cin >> x;
  std::string junk;
  std::getline(std::cin, junk, '\n');

  if (x <= 0) 
    throw std::runtime_error("### 输入必须大于0");
  else
    return x;
}

int main()
{
  while (1) {
    try {
      std::cout << "请输入迷宫宽度，建议宽度为20: ";
      auto x{GetXnY()};

      std::cout << "请输入迷宫宽度，建议高度为10: ";
      auto y{GetXnY()};
      

      auto a{std::make_unique<Maze>(x, y)};
      a->Display();
      std::cout << std::endl << "请按回车键查看最短路径……";
      std::cin.get();
      a->Run();
      std::cout << std::endl;
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
