#include "boost/variant.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <time.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/system_error.hpp>
#include <functional>
#include <unordered_map>

// Motivation
// balancing complexity and presentation/usability
// Typical home:
// - Many subsystems (electrical, sanitation etc)
// - Complex internal structure (e.g. floor, layers etc)
// - End user not exposed to internals
// Same with software!
// - Many systems working together provide flexibility, but..
// - API consumbers want it to "just work"!

// Facade: Provides a simple, easy to understand/use interface over a large and sophisticated body of code.
// - Make a library easier to understand, use and test
// - Reduce dependencies of user code on internal API's that may change
// -- Allows more flexibility in developing/refactoring the library
// -- Wrap a poorly designed collection of API's with a single well-designed API

// example: lots of complicated classes that work together:
struct Size
{
    uint32_t width;
    uint32_t height;
};

struct Point
{
    uint32_t x;
    uint32_t y;

    Point() : x(0), y(0) {}
    Point(int x, int y) : x(x), y(y) {}
};

class IBuffer {
public:
    virtual ~IBuffer() {
    }

    virtual Size get_size() const = 0;

    virtual char operator()(uint32_t x, uint32_t y) = 0;

    virtual void add_string(const std::string &s) = 0;
};

#include <boost/circular_buffer.hpp>
#include <boost/none.hpp>
#include <boost/optional.hpp>

class TextBuffer : public IBuffer {
    boost::circular_buffer<std::string> lines;
    const std::string line_break = "\n";
    uint32_t width, height;
public:
    TextBuffer(uint32_t width, uint32_t height) : lines{height}, width{width}, height{height} {
    }

    Size get_size() const override {
        return {width, height};
    }

    char operator()(uint32_t x, uint32_t y) override {
        if (y + 1 > lines.size())
            return 0;
        return x + 1 > lines[y].size() ? 0 : lines[x][y];
    }

    void add_string(const std::string &s) override {
        lines.push_back(s);
    }

    std::string to_string() {
        std::ostringstream oss;
        for (auto &line: lines)
            oss << line << line_break;
        return oss.str();
    }
};

class Viewport {
protected:
    std::shared_ptr<IBuffer> buffer;
    Point buffer_location;
    Point screen_location;
    Size size;
public:
    Viewport(const std::shared_ptr<IBuffer>& buffer, boost::optional<Size> size = boost::none,
            boost::optional<Point> buffer_location = boost::none,
            boost::optional<Point> screen_location = boost::none) :buffer(buffer) {
        this->size = size.value_or(buffer->get_size());
        this->buffer_location = buffer_location.value_or(Point{});
        this->screen_location = screen_location.value_or(Point{});
    }
    // Gets the character the buffer is meant to show, or  0 if out of bounds.
    char operator()(int x, int y) const {
        if ((x>= screen_location.x) && x < (screen_location.x + size.width) &&
                (y>=screen_location.y) && y < (screen_location.y + size.height)) {
            return (*buffer)(x-screen_location.x, y-screen_location.y);
        }
        return 1;
    }
};

struct TextRenderer {
    //...
    //... complex stuff
    //...
    TextRenderer() {
        std::cout << "TextRenderer constructor" << std::endl;
    }
    ~TextRenderer() {
        std::cout << "TextRenderer destructor" << std::endl;
    }
    void DrawText() {
        std::cout << "draw" << std::endl;
    }
};

struct Window
{
    int width, height;
    std::shared_ptr<TextRenderer> text_renderer;

    std::vector<std::shared_ptr<IBuffer>> buffers;
    std::vector<std::shared_ptr<Viewport>> viewports;

    Window(std::string title, int width, int height){
        std::cout << "Window constuctor" << std::endl;
    };

    void DrawEverything(){
        std::cout << "DrawEverything" << std::endl;
    };
    void Show(){
        std::cout << "Show" << std::endl;
    };
    std::pair<int, int> size() const {
        std::cout << "size" << std::endl;
        return {1,2};
    };

    ~Window(){
        std::cout << "Window destuctor" << std::endl;
    };

    // used to get around the callback stupidity with C APIs
    static Window* self;
};

TEST(facade, complicated_way_of_setting_it_up) {
    Window window("Test", 1280, 720);
    auto buffer = std::make_shared<TextBuffer>(40,40);
    window.buffers.push_back(buffer);
    auto viewport = std::make_shared<Viewport>(buffer);
    window.viewports.emplace_back(viewport);

    auto viewport2 = std::make_shared<Viewport>(buffer, Size{40,40}, Point{}, Point{45, 0});
    window.viewports.emplace_back(viewport2);
    auto viewport3 = std::make_shared<Viewport>(buffer, Size{40,40}, Point{}, Point{90, 0});
    window.viewports.emplace_back(viewport3);

    window.Show();
    // everything here can be automated in a facade and simply what is happening in the system.
}

// Facade: this is actually a piece of syntactic sugar
class Console {
public:
    std::vector<std::shared_ptr<Window>> windows;

    static Console& instance(){
        static Console console;
        return console;
    }

    std::shared_ptr<Window> multicolumn(const std::string& title, uint8_t columnCount, uint8_t columnWidth, uint8_t charsHigh){
        auto w = std::make_shared<Window>(title, columnCount * columnWidth * charWidth, charsHigh * charHeigth);
        for (uint8_t c = 0; c < columnCount; ++c) {
            auto buffer = std::make_shared<TextBuffer>(columnWidth, charsHigh);
            w->buffers.push_back(buffer);
            auto viewport = std::make_shared<Viewport>(buffer, buffer->get_size(), Point{}, Point{columnWidth*c, 0});
            w->viewports.push_back(viewport);
            windows.push_back(w);
        }
        return w;
    }

    std::shared_ptr<Window> single(const std::string& title, uint8_t charsWide, uint8_t charsHigh) {
        auto w = std::make_shared<Window>(title, charsWide * charWidth, charsHigh * charHeigth);
        auto buffer = std::make_shared<TextBuffer>(charsWide, charsHigh);
        w->buffers.push_back(buffer);
        auto viewport = std::make_shared<Viewport>(buffer);
        w->viewports.push_back(viewport);
        windows.push_back(w);
        return w;
    }
private:
    const int charWidth = 10, charHeigth = 14;
    Console(){

    }
    Console(Console const&) = delete;
    void operator=(Console const&) = delete;
};

TEST(facade, simple_facade_single) {
    auto window = Console::instance().single("Test", 50, 30);
    window->Show();
}

TEST(facade, simple_facade_multi) {
    auto window = Console::instance().multicolumn("Test", 2, 50, 30);
    window->Show();
}

// Build a Facade to provide a simplified API over a set of classes.
// May wish to (optionally) expose internals through the facade.
// May allow user to "escalate" to use more comples API's of they need to.