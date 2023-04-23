#include "scc/driver/PretentiousUI.h"
#include "scc/driver/ColorOutput.h"
#include "scc/driver/DriverUtils.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

void PretentiousUI::render() {

  const size_t lineLen = 53;
  const winsize w = DriverUtils::getTerminalSize();

  auto centered = [w](std::string s) {
    return std::string((w.ws_col - s.size()) / 2, ' ') + s;
  };

  const std::string paddingLeft = std::string((w.ws_col - lineLen) / 2, ' ');
  const std::string paddingTop = std::string((w.ws_row - 12) / 2, '\n');

  std::string pad = "\n" + paddingLeft;
  std::string text = pad + R"(anon for artifact review)";
  std::string underline =
      paddingLeft + "████████████████████████████████████████████████████";

  using Col = ColorOutput::Color;
  using namespace std::chrono_literals;
  struct ColorScheme {
    Col top;
    Col logo;
    Col bot;
    const char *hint;
  };

  std::vector<ColorScheme> colors = {
      {Col::Blue, Col::White, Col::White, "Ready?"},
      {Col::Blue, Col::White, Col::White, "Ready?"},
      {Col::Blue, Col::White, Col::Blue, "Set."},
      {Col::Blue, Col::White, Col::Blue, "Set."},
      {Col::White, Col::Blue, Col::White, "Go!"},
  };
  for (ColorScheme colors : colors) {
    DriverUtils::clearScreen();
    std::cout << paddingTop;
    ColorOutput::print(colors.top, underline);
    std::cout << "\n";
    ColorOutput::print(colors.logo, text);
    std::cout << "\n";
    ColorOutput::print(colors.bot, underline);
    std::cout << "\n\n";
    std::cout << centered(colors.hint);

    std::cout.flush();
    std::this_thread::sleep_for(700ms);
  }
  std::this_thread::sleep_for(1000ms);
}
