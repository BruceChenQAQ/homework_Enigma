#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <random>
#include <unordered_map>
#include <vector>

class Rotor {
private:
  int position = 0;
  std::array<char, 26> rotor_forward;
  std::array<int, 26> rotor_backward;

  void init(std::mt19937 &gen) {
    std::string all = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::shuffle(all.begin(), all.end(), gen);
    for (size_t i = 0; i < 26; ++i) {
      rotor_forward[i] = all[i];
      rotor_backward[all[i] - 'A'] = i;
    }
  }

public:
  explicit Rotor(std::mt19937 &gen) { init(gen); }
  explicit Rotor() {
    auto gen = std::mt19937{std::random_device{}()};
    init(gen);
  }

  Rotor(Rotor const &other) = default;
  Rotor &operator=(Rotor const &other) = default;

  Rotor(Rotor &&other) = default;
  Rotor &operator=(Rotor &&other) = default;

  bool turn_rotor() {
    position = (position + 1) % 26;
    return position == 0;
  }

  void print() {
    for (auto &c : rotor_forward)
      printf("%c", c);
    printf(" pos = %2d (%c)", position, rotor_forward[position]);
    puts("");
  }

  void init_position(char letter) { position = letter - 'A'; }

  // 正向通过
  char encode_forward(char letter) {
    int index = letter - 'A';
    return rotor_forward[(index + position) % 26];
  }

  // 反向通过
  char encode_backward(char letter) {
    int index = rotor_backward[letter - 'A'];
    return 'A' + ((26 + index - position) % 26);
  }
};

class Reflector {
private:
  std::array<int, 26> reflector;

  void init(std::mt19937 &gen) {
    const std::string all = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string part1, part2;
    // 取样一半，与另一半建立映射关系
    std::sample(all.begin(), all.end(), std::back_inserter(part1), 13, gen);
    // 集合相减获取另一半
    std::set_difference(all.begin(), all.end(), part1.begin(), part1.end(),
                        std::back_inserter(part2));
    // 打乱有序的后半部分
    std::shuffle(part2.begin(), part2.end(), gen);
    // 生成完毕，记录映射关系
    for (size_t i = 0; i < 13; ++i) {
      reflector[part1[i] - 'A'] = part2[i];
      reflector[part2[i] - 'A'] = part1[i];
    }
    // std::cout << part1 << part2 << std::endl;
  }

public:
  explicit Reflector(std::mt19937 &gen) { init(gen); }
  explicit Reflector() {
    auto gen = std::mt19937{std::random_device{}()};
    init(gen);
  }

  Reflector(Reflector const &other) = default;
  Reflector &operator=(Reflector const &other) = default;

  Reflector(Reflector &&other) = default;
  Reflector &operator=(Reflector &&other) = default;

  void print() {
    for (size_t i = 0; i < 13; ++i)
      printf("%c<->%c  ", reflector[i], reflector[i + 13]);
    puts("");
  }

  char reflect(char letter) { return reflector[letter - 'A']; }
};

class Plugboard {
private:
  std::array<char, 26> plugboard = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
                                    'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
                                    'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

  void init(std::mt19937 &gen) {
    std::uniform_int_distribution<> distrib(0, 25);
    std::uniform_int_distribution<> cnt_distrib(0, 6);

    // 随机设定插线次数
    int cnt = cnt_distrib(gen);
    while (cnt--) {
      // 每次选择两个字母，将它们互换
      int a = distrib(gen), b = distrib(gen);
      std::swap(plugboard[a], plugboard[b]);
    }
  }

public:
  explicit Plugboard(std::mt19937 &gen) { init(gen); }
  explicit Plugboard() {
    auto gen = std::mt19937{std::random_device{}()};
    init(gen);
  }

  char plugboard_encode(char letter) { return plugboard[letter - 'A']; }

  void print() {
    std::array<bool, 26> st;
    for (size_t i = 0; i < 26; ++i) {
      if (st[(plugboard[i] - 'A')] || (plugboard[i] - 'A') == i)
        continue;
      st[plugboard[i] - 'A'] = true;
      printf("%c <-> %c  ", plugboard[i], char('A' + i));
    }
    puts("");
  }
};

class Enigma {
private:
  std::vector<Rotor> rotors;
  Reflector reflector;
  Plugboard plugboard;

public:
  explicit Enigma(const std::string &_keys, const std::vector<Rotor> &_rotors,
                  const Reflector &_reflector, const Plugboard &_plugboard)
      : reflector(std::move(_reflector)), plugboard(std::move(_plugboard)) {

    if (_rotors.size() == 0) {
      std::cout << "转子数不能为零!" << std::endl;
      throw;
    }

    if (_keys.size() != _rotors.size()) {
      std::cout << "密钥长度不匹配!" << std::endl;
      throw;
    }

    // 根据密钥设置每个转子的初始位置
    rotors = std::move(_rotors);
    for (size_t i = 0; i < _keys.size(); ++i)
      rotors[i].init_position(toupper(_keys[i]));
  }

  Enigma(Enigma const &other) = default;
  Enigma &operator=(Enigma const &other) = default;

  Enigma(Enigma &&other) = default;
  Enigma &operator=(Enigma &&other) = default;

  void encode(std::string &input) {
    // 依次加密每个字符
    for (auto &c : input) {
      if (!std::isalpha(c))
        continue;
      c = toupper(c);
      // 通过插线板
      c = plugboard.plugboard_encode(c);
      // 依次正向通过转子
      for (auto it = rotors.begin(); it != rotors.end(); ++it)
        c = it->encode_forward(c);
      // 通过反射器
      c = reflector.reflect(c);
      // 依次反向通过转子
      for (auto it = rotors.rbegin(); it != rotors.rend(); ++it)
        c = it->encode_backward(c);
      // 通过插线板
      c = plugboard.plugboard_encode(c);
      //转子转动
      bool has_trun = true;
      for (auto &rotor : rotors) {
        has_trun = rotor.turn_rotor();
        // 如果转满一圈，则继续带动下一个转子转动，否则结束
        if (!has_trun)
          break;
      }
    }
  }

  void print_status() {
    printf("插线板: ");
    plugboard.print();
    for (size_t i = 0; i < rotors.size(); ++i) {
      printf("第%2lu个转子: ", i);
      rotors[i].print();
    }
    printf("反射器: ");
    reflector.print();
  }
};

int main() {
  // 随机数生成器，可固定种子以获得相同结果
  auto gen = std::mt19937{91021234};
  // auto gen = std::mt19937{std::random_device{}()};

  // 转子的初始位置
  std::string initial_key = "CTR";

  // 三个转子
  std::vector<Rotor> rotors;
  for (size_t i = 0; i < 3; ++i)
    rotors.push_back(Rotor(gen));

  // 反射器
  Reflector reflector = Reflector(gen);

  // 插线板
  Plugboard plugboard = Plugboard(gen);

  // 一台恩格玛机，用于加密
  Enigma e = Enigma(initial_key, rotors, reflector, plugboard);
  // 复制一台用于相同配置的恩格玛机，用于解密
  Enigma e2 = e;

  std::string text = "HELLO ENIGMA!_QWERTYUIOPASDFGHJKLZXCVBNM";
  std::cout << "原文: " << text << std::endl;

  std::cout << std::endl << "恩格玛机1加密前: ";
  e.print_status();
  std::cout << std::endl;

  e.encode(text);
  std::cout << "密文: " << text << std::endl;

  std::cout << std::endl << "恩格玛机1加密后: ";
  e.print_status();
  std::cout << std::endl;

  std::cout << std::endl << "恩格玛机2解密前: ";
  e2.print_status();
  std::cout << std::endl;

  e2.encode(text);
  std::cout << "解密: " << text << std::endl;

  std::cout << std::endl << "恩格玛机2解密后: ";
  e2.print_status();
  std::cout << std::endl;
  return 0;
}