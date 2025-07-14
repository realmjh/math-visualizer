#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <iomanip>

class Parser {
private:
    std::string expr;
    size_t pos;
    
    std::string preprocess(std::string input) {
        std::string result;
        for (size_t i = 0; i < input.length(); i++) {
            char curr = input[i];
            result += curr;
            if (i < input.length() - 1) {
                char next = input[i + 1];
                if ((isdigit(curr) && isalpha(next)) ||
                    (isalpha(curr) && isdigit(next)) ||
                    (isalpha(curr) && isalpha(next) && curr != next) ||
                    (curr == ')' && next == '(') ||
                    (curr == ')' && isalpha(next)) ||
                    (isdigit(curr) && next == '(') ||
                    (isalpha(curr) && next == '(')) {
                    result += '*';
                }
            }
        }
        return result;
    }
    
    double getNum() {
        std::string num;
        while (pos < expr.length() && (isdigit(expr[pos]) || expr[pos] == '.')) {
            num += expr[pos++];
        }
        return num.empty() ? 0 : std::stod(num);
    }
    
    double getFactor() {
        if (pos >= expr.length()) return 0;
        
        if (expr[pos] == '(') {
            pos++;
            double result = getExpr();
            if (pos < expr.length() && expr[pos] == ')') pos++;
            return result;
        }
        
        if (expr[pos] == '-') {
            pos++;
            return -getFactor();
        }
        
        if (expr[pos] == '+') {
            pos++;
            return getFactor();
        }
        
        if (isdigit(expr[pos])) {
            return getNum();
        }
        
        if (isalpha(expr[pos])) {
            std::string var;
            while (pos < expr.length() && isalpha(expr[pos])) {
                var += expr[pos++];
            }
            
            if (var == "x") return xVal;
            if (var == "y") return yVal;
            if (var == "pi") return M_PI;
            if (var == "e") return M_E;
            
            if (pos < expr.length() && expr[pos] == '(') {
                pos++;
                double arg = getExpr();
                if (pos < expr.length() && expr[pos] == ')') pos++;
                
                if (var == "sin") return sin(arg);
                if (var == "cos") return cos(arg);
                if (var == "tan") return tan(arg);
                if (var == "log") return arg > 0 ? log(arg) : 0;
                if (var == "sqrt") return arg >= 0 ? sqrt(arg) : 0;
                if (var == "exp") return exp(arg);
                if (var == "abs") return fabs(arg);
            }
            return 0;
        }
        return 0;
    }
    
    double getTerm() {
        double result = getFactor();
        while (pos < expr.length()) {
            if (expr[pos] == '*') {
                pos++;
                result *= getFactor();
            } else if (expr[pos] == '/') {
                pos++;
                double div = getFactor();
                result = (div != 0) ? result / div : 0;
            } else if (expr[pos] == '^') {
                pos++;
                result = pow(result, getFactor());
            } else {
                break;
            }
        }
        return result;
    }
    
    double getExpr() {
        double result = getTerm();
        while (pos < expr.length()) {
            if (expr[pos] == '+') {
                pos++;
                result += getTerm();
            } else if (expr[pos] == '-') {
                pos++;
                result -= getTerm();
            } else {
                break;
            }
        }
        return result;
    }
    
public:
    double xVal, yVal;
    
    double eval(const std::string& expression, double x, double y = 0) {
        try {
            expr = preprocess(expression);
            expr.erase(std::remove(expr.begin(), expr.end(), ' '), expr.end());
            pos = 0;
            xVal = x;
            yVal = y;
            return getExpr();
        } catch (...) {
            return 0;
        }
    }
};

class InputBox {
private:
    sf::RectangleShape bg;
    sf::RectangleShape border;
    std::string txt;
    bool active;
    sf::Font* fnt;
    sf::Text display;
    sf::Text lbl;
    float timer;
    bool cursor;
    
public:
    InputBox(float x, float y, float w, float h, const std::string& label = "") 
        : active(false), fnt(nullptr), timer(0), cursor(false) {
        bg.setPosition(x, y);
        bg.setSize(sf::Vector2f(w, h));
        bg.setFillColor(sf::Color(30, 30, 30));
        
        border.setPosition(x - 2, y - 2);
        border.setSize(sf::Vector2f(w + 4, h + 4));
        border.setFillColor(sf::Color::Transparent);
        border.setOutlineThickness(2);
        border.setOutlineColor(sf::Color(100, 100, 100));
        
        display.setPosition(x + 5, y + 5);
        display.setFillColor(sf::Color::White);
        display.setCharacterSize(16);
        
        lbl.setString(label);
        lbl.setPosition(x, y - 20);
        lbl.setFillColor(sf::Color::White);
        lbl.setCharacterSize(14);
    }
    
    void setFont(sf::Font& f) {
        fnt = &f;
        display.setFont(f);
        lbl.setFont(f);
    }
    
    void setActive(bool a) {
        active = a;
        border.setOutlineColor(a ? sf::Color::Green : sf::Color(100, 100, 100));
    }
    
    bool isActive() const { return active; }
    
    void handleText(sf::Uint32 unicode) {
        if (!active) return;
        if (unicode == 8) {
            if (!txt.empty()) txt.pop_back();
        } else if (unicode >= 32 && unicode < 127) {
            txt += static_cast<char>(unicode);
        }
        refresh();
    }
    
    void refresh() {
        if (fnt) {
            display.setString(txt + (cursor && active ? "|" : ""));
        }
    }
    
    void update(float dt) {
        timer += dt;
        if (timer > 0.5f) {
            cursor = !cursor;
            timer = 0;
            refresh();
        }
    }
    
    void draw(sf::RenderWindow& win) {
        if (fnt) win.draw(lbl);
        win.draw(border);
        win.draw(bg);
        if (fnt) win.draw(display);
    }
    
    std::string getText() const { return txt; }
    void clear() { 
        txt.clear(); 
        refresh();
    }
    
    bool contains(sf::Vector2f pt) {
        return bg.getGlobalBounds().contains(pt);
    }
};

class Plotter {
private:
    sf::RenderWindow& win;
    Parser parser;
    sf::Font font;
    bool hasFont;
    
    double xMin, xMax, yMin, yMax;
    int w, h;
    std::vector<std::string> eqs;
    std::vector<sf::Color> cols;
    
    sf::Vector2f toScreen(double x, double y) {
        float sx = static_cast<float>((x - xMin) / (xMax - xMin) * w);
        float sy = static_cast<float>(h - (y - yMin) / (yMax - yMin) * h);
        return sf::Vector2f(sx, sy);
    }
    
    void toWorld(float sx, float sy, double& wx, double& wy) {
        wx = xMin + (sx / w) * (xMax - xMin);
        wy = yMax - (sy / h) * (yMax - yMin);
    }
    
    std::string fmtNum(double n) {
        if (fabs(n) < 0.001 && n != 0) return "0";
        if (fabs(n) > 9999) {
            std::ostringstream oss;
            oss << std::scientific << std::setprecision(0) << n;
            return oss.str();
        }
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << n;
        std::string result = oss.str();
        result.erase(result.find_last_not_of('0') + 1, std::string::npos);
        result.erase(result.find_last_not_of('.') + 1, std::string::npos);
        return result;
    }
    
public:
    Plotter(sf::RenderWindow& window) : win(window), hasFont(false), xMin(-10), xMax(10), yMin(-10), yMax(10) {
        w = static_cast<int>(win.getSize().x);
        h = static_cast<int>(win.getSize().y - 100);
        
        if (font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") ||
            font.loadFromFile("/System/Library/Fonts/Arial.ttf") ||
            font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
            hasFont = true;
        }
        
        cols = {sf::Color::Red, sf::Color::Blue, sf::Color::Green, sf::Color::Yellow,
                sf::Color::Magenta, sf::Color::Cyan, sf::Color(255, 165, 0), sf::Color(128, 0, 128)};
    }
    
    sf::Font* getFont() { return hasFont ? &font : nullptr; }
    
    void add(const std::string& eq) {
        if (!eq.empty()) {
            eqs.push_back(eq);
            std::cout << "Added: " << eq << std::endl;
        }
    }
    
    void clear() {
        eqs.clear();
        std::cout << "Cleared" << std::endl;
    }
    
    void setView(double xmin, double xmax, double ymin, double ymax) {
        xMin = xmin; xMax = xmax; yMin = ymin; yMax = ymax;
    }
    
    void zoom(float factor, sf::Vector2f center) {
        double cx, cy;
        toWorld(center.x, center.y, cx, cy);
        
        double rx = (xMax - xMin) * factor;
        double ry = (yMax - yMin) * factor;
        
        xMin = cx - rx / 2;
        xMax = cx + rx / 2;
        yMin = cy - ry / 2;
        yMax = cy + ry / 2;
    }
    
    void pan(float dx, float dy) {
        double wx = dx * (xMax - xMin) / w;
        double wy = -dy * (yMax - yMin) / h;
        
        xMin -= wx; xMax -= wx;
        yMin -= wy; yMax -= wy;
    }
    
    void drawGrid() {
        sf::VertexArray grid(sf::Lines);
        
        double rx = xMax - xMin;
        double ry = yMax - yMin;
        
        double sx = pow(10, floor(log10(rx / 10)));
        double sy = pow(10, floor(log10(ry / 10)));
        
        if (rx / sx < 5) sx /= 2;
        if (ry / sy < 5) sy /= 2;
        
        double startX = ceil(xMin / sx) * sx;
        for (double x = startX; x <= xMax; x += sx) {
            sf::Vector2f top = toScreen(x, yMax);
            sf::Vector2f bot = toScreen(x, yMin);
            sf::Color gc = (fabs(x) < sx / 2) ? sf::Color(80, 80, 80) : sf::Color(40, 40, 40);
            grid.append(sf::Vertex(top, gc));
            grid.append(sf::Vertex(bot, gc));
        }
        
        double startY = ceil(yMin / sy) * sy;
        for (double y = startY; y <= yMax; y += sy) {
            sf::Vector2f left = toScreen(xMin, y);
            sf::Vector2f right = toScreen(xMax, y);
            sf::Color gc = (fabs(y) < sy / 2) ? sf::Color(80, 80, 80) : sf::Color(40, 40, 40);
            grid.append(sf::Vertex(left, gc));
            grid.append(sf::Vertex(right, gc));
        }
        
        win.draw(grid);
    }
    
    void drawAxes() {
        sf::VertexArray axes(sf::Lines);
        
        if (yMin <= 0 && yMax >= 0) {
            sf::Vector2f left = toScreen(xMin, 0);
            sf::Vector2f right = toScreen(xMax, 0);
            axes.append(sf::Vertex(left, sf::Color::White));
            axes.append(sf::Vertex(right, sf::Color::White));
        }
        
        if (xMin <= 0 && xMax >= 0) {
            sf::Vector2f top = toScreen(0, yMax);
            sf::Vector2f bot = toScreen(0, yMin);
            axes.append(sf::Vertex(top, sf::Color::White));
            axes.append(sf::Vertex(bot, sf::Color::White));
        }
        
        win.draw(axes);
    }
    
    void drawLabels() {
        if (!hasFont) return;
        
        double rx = xMax - xMin;
        double ry = yMax - yMin;
        
        double lx = pow(10, floor(log10(rx / 6)));
        double ly = pow(10, floor(log10(ry / 6)));
        
        if (rx / lx < 4) lx /= 2;
        if (ry / ly < 4) ly /= 2;
        
        double startX = ceil(xMin / lx) * lx;
        int cnt = 0;
        for (double x = startX; x <= xMax && cnt < 8; x += lx, cnt++) {
            if (fabs(x) > lx / 10) {
                sf::Vector2f pos = toScreen(x, 0);
                if (pos.y > h - 20) pos.y = h - 20;
                if (pos.y < 15) pos.y = 15;
                
                sf::Text label(fmtNum(x), font, 12);
                label.setPosition(pos.x - 15, pos.y + 5);
                label.setFillColor(sf::Color::White);
                win.draw(label);
            }
        }
        
        double startY = ceil(yMin / ly) * ly;
        cnt = 0;
        for (double y = startY; y <= yMax && cnt < 8; y += ly, cnt++) {
            if (fabs(y) > ly / 10) {
                sf::Vector2f pos = toScreen(0, y);
                if (pos.x > w - 50) pos.x = w - 50;
                if (pos.x < 5) pos.x = 5;
                
                sf::Text label(fmtNum(y), font, 12);
                label.setPosition(pos.x + 5, pos.y - 6);
                label.setFillColor(sf::Color::White);
                win.draw(label);
            }
        }
    }
    
    void drawEqs() {
        for (size_t i = 0; i < eqs.size(); i++) {
            sf::Color col = cols[i % cols.size()];
            
            if (eqs[i].find('y') != std::string::npos) {
                plotImplicit(eqs[i], col);
            } else {
                plotFunc(eqs[i], col);
            }
        }
    }
    
    void plotFunc(const std::string& eq, sf::Color col) {
        sf::VertexArray curve(sf::LineStrip);
        
        int pts = std::min(w * 2, 1600);
        double step = (xMax - xMin) / pts;
        
        double lastY = 0;
        bool lastValid = false;
        
        for (double x = xMin; x <= xMax; x += step) {
            try {
                double y = parser.eval(eq, x);
                
                if (!std::isnan(y) && !std::isinf(y)) {
                    if (y >= yMin && y <= yMax) {
                        sf::Vector2f pt = toScreen(x, y);
                        if (pt.x >= 0 && pt.x <= w && pt.y >= 0 && pt.y <= h) {
                            if (lastValid && fabs(y - lastY) > (yMax - yMin) * 0.1) {
                                if (curve.getVertexCount() > 1) {
                                    win.draw(curve);
                                }
                                curve.clear();
                            }
                            curve.append(sf::Vertex(pt, col));
                            lastY = y;
                            lastValid = true;
                        }
                    } else {
                        if (curve.getVertexCount() > 1) {
                            win.draw(curve);
                        }
                        curve.clear();
                        lastValid = false;
                    }
                } else {
                    if (curve.getVertexCount() > 1) {
                        win.draw(curve);
                    }
                    curve.clear();
                    lastValid = false;
                }
            } catch (...) {
                if (curve.getVertexCount() > 1) {
                    win.draw(curve);
                }
                curve.clear();
                lastValid = false;
            }
        }
        
        if (curve.getVertexCount() > 1) {
            win.draw(curve);
        }
    }
    
    void plotImplicit(const std::string& eq, sf::Color col) {
        int res = std::min(150, w / 3);
        double sx = (xMax - xMin) / res;
        double sy = (yMax - yMin) / res;
        
        sf::VertexArray pts(sf::Points);
        
        for (int i = 0; i < res; i++) {
            for (int j = 0; j < res; j++) {
                double x = xMin + i * sx;
                double y = yMin + j * sy;
                
                try {
                    double val = parser.eval(eq, x, y);
                    
                    if (fabs(val) < 0.5) {
                        sf::Vector2f pt = toScreen(x, y);
                        if (pt.x >= 0 && pt.x <= w && pt.y >= 0 && pt.y <= h) {
                            pts.append(sf::Vertex(pt, col));
                        }
                    }
                } catch (...) {}
            }
        }
        
        if (pts.getVertexCount() > 0) {
            win.draw(pts);
        }
    }
    
    void drawList() {
        if (!hasFont) return;
        
        for (size_t i = 0; i < eqs.size(); i++) {
            sf::Text txt(eqs[i], font, 14);
            txt.setPosition(10, 10 + i * 20);
            txt.setFillColor(cols[i % cols.size()]);
            win.draw(txt);
        }
    }
    
    void draw() {
        drawGrid();
        drawAxes();
        drawLabels();
        drawEqs();
        drawList();
    }
};

class App {
private:
    sf::RenderWindow win;
    Plotter plot;
    InputBox input;
    bool dragging;
    sf::Vector2i lastMouse;
    sf::Clock clk;
    
public:
    App() : win(sf::VideoMode(1200, 800), "Graph Calculator"),
            plot(win),
            input(10, 720, 600, 30, "Enter equation:"),
            dragging(false) {
        
        win.setFramerateLimit(60);
        
        if (plot.getFont()) {
            input.setFont(*plot.getFont());
        }
        
        plot.add("sin(x)");
        plot.add("x^2 + y^2 - 25");
    }
    
    void events() {
        sf::Event e;
        while (win.pollEvent(e)) {
            if (e.type == sf::Event::Closed) {
                win.close();
            }
            else if (e.type == sf::Event::MouseButtonPressed) {
                if (e.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f mouse(static_cast<float>(e.mouseButton.x), 
                                     static_cast<float>(e.mouseButton.y));
                    
                    if (input.contains(mouse)) {
                        input.setActive(true);
                    } else {
                        input.setActive(false);
                        dragging = true;
                        lastMouse = sf::Vector2i(e.mouseButton.x, e.mouseButton.y);
                    }
                }
            }
            else if (e.type == sf::Event::MouseButtonReleased) {
                if (e.mouseButton.button == sf::Mouse::Left) {
                    dragging = false;
                }
            }
            else if (e.type == sf::Event::MouseMoved) {
                if (dragging) {
                    sf::Vector2i curr(e.mouseMove.x, e.mouseMove.y);
                    sf::Vector2i delta = curr - lastMouse;
                    plot.pan(static_cast<float>(delta.x), static_cast<float>(delta.y));
                    lastMouse = curr;
                }
            }
            else if (e.type == sf::Event::MouseWheelScrolled) {
                float zoom = e.mouseWheelScroll.delta > 0 ? 0.9f : 1.1f;
                sf::Vector2f mouse(static_cast<float>(e.mouseWheelScroll.x), 
                                 static_cast<float>(e.mouseWheelScroll.y));
                plot.zoom(zoom, mouse);
            }
            else if (e.type == sf::Event::TextEntered) {
                if (input.isActive()) {
                    input.handleText(e.text.unicode);
                }
            }
            else if (e.type == sf::Event::KeyPressed) {
                if (input.isActive() && e.key.code == sf::Keyboard::Enter) {
                    std::string eq = input.getText();
                    if (!eq.empty()) {
                        plot.add(eq);
                        input.clear();
                    }
                } else if (input.isActive() && e.key.code == sf::Keyboard::Escape) {
                    input.setActive(false);
                } else if (!input.isActive()) {
                    if (e.key.code == sf::Keyboard::R) {
                        plot.setView(-10, 10, -10, 10);
                    } else if (e.key.code == sf::Keyboard::C) {
                        plot.clear();
                    } else if (e.key.code == sf::Keyboard::I) {
                        input.setActive(true);
                    }
                }
            }
        }
    }
    
    void update() {
        float dt = clk.restart().asSeconds();
        input.update(dt);
    }
    
    void render() {
        win.clear(sf::Color::Black);
        plot.draw();
        input.draw(win);
        win.display();
    }
    
    void run() {
        while (win.isOpen()) {
            events();
            update();
            render();
        }
    }
};

int main() {
    App app;
    app.run();
    return 0;
}
