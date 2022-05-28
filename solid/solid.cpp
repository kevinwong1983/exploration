//#include "boost/variant.hpp"
//#include <iostream>
//#include <gtest/gtest.h>
//#include <boost/asio.hpp>
//#include <time.h>
//#include <boost/date_time/posix_time/posix_time.hpp>
//#include <boost/system/system_error.hpp>
//#include <functional>
//#include <unordered_map>
//#include <vector>
//
//template<typename T, typename... Args>
//std::unique_ptr<T> make_unique(Args&&... args)
//{
//    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
//}
//
////////////////////////////////////////////////////////////////////////////////////////////
////      SRP - Single Responsibility Principle
////      A class should only have a single responsibility
//// -> We gather the things that are going to change for the same reason, and we separate
////    the things that are going to change for different reasons.
//// -> When we keep our responsibility separate, we can change them with minimum impact.
//// -> Definition of a responsibility: a family of function that serves a specific actor.
//// -> Classes have responsibilities to their users. These users can be classified as actors
////    by the role that they play.
//// -> A responsibility is a family of functions that serves one particular actor.
//// -> There are two values of software.
//// -> -> primary value: Keep it flexible so it can continue to meet its requirements
////      throughout its lifetime.
//// -> -> Secondary value: software should serve the users need.
//// -> Carefully allocating responsibility to classes and modules is one of the ways to
////    keep the primary value of software high.
//// -> When modules have more than one responsibility, the system become fragile due to unintended
////    interactions between the responsibilities.
//// -> Separate ways to solve violations of the SRP.
//// -> -> Separations
//// -> -> Facades
//// -> -> Interface segregation
//// -> Non of the ways are perfect: welcome to software engineering
//
//#include <fstream>
//
////class Todo {
////public:
////    Todo (const std::string& title):title_(title) {
////    }
////    void addTask (const std::string& task) {
////        tasks_.push_back(task);
////    }
////    void save(std::string file_name){
////        std::ofstream stream(file_name);
////        stream << title_ << std::endl;
////        for (auto& e: tasks_) {
////            stream << e << std::endl;
////        }
////    }
////private:
////    std::string title_;
////    std::vector<std::string> tasks_;
////};
//
//class Todo {
//public:
//    Todo (const std::string& title):title_(title) {
//    }
//    void addTask (const std::string& task) {
//        tasks_.push_back(task);
//    }
//    std::string getTitle() const {
//        return title_;
//    }
//    std::vector<std::string> getTasks() const {
//        return tasks_;
//    }
//private:
//    std::string title_;
//    std::vector<std::string> tasks_;
//};
//
//class PersistenceManager {
//public:
//    void save(const Todo& todo, std::string file_name){
//        std::ofstream stream(file_name);
//        stream << todo.getTitle() << std::endl;
//        for (auto& e: todo.getTasks()) {
//            stream << e << std::endl;
//        }
//    }
//};
//
//
//TEST(solid, SRP) {
//    Todo td("wedding");
//    td.addTask("order wedding cake");
//    td.addTask("send out inviatations");
//    td.addTask("plan rehersal");
//
//    auto pm = PersistenceManager();
//    pm.save(td, "wedding plan.txt" );
//}
//
//struct Journal {
//    std::string title_;
//    std::vector<std::string> entries_;
//
//    explicit Journal(const std::string& title):title_(title){
//    }
//    void add(const std::string& entry) {
//        entries_.push_back(entry);
//    }
//    // it should not be the responsibility for Journal to save itself. should be handled by separate class.
//    // -> class should only have one reason to change:
//    // -> -> e.g. in case the persistence implementation changes fram file to database,
//    // -> -> e.g. format in which the journal should be saved
//    // Journal class should not be changed.
////    void save(const std::string& filename) {
////        std::ofstream ofs(filename);
////        for (auto& e: entries_) {
////            ofs << e << std::endl;
////        }
////    }
//};
//
//// add a persistence manager to take the "save" responsibility
////struct PersistenceManager {
////    void save(const Journal& j, const std::string& filename) {
////        std::ofstream ofs(filename);
////        for (auto& e: j.entries_) {
////            ofs << e << std::endl;
////        }
////    }
////};
//
////////////////////////////////////////////////////////////////////////////////////////////
////      OCP - Open close principle
////      Module should be open for extension and closed for modification
//// -> Open for extentions: it should be very simple to change the behavoir of that module
//// -> Closed for modification means the source code should not change
//// -> How do we do this? Abstraction & Inversion
//// -> When ever you have a module with behavior you want to extend without modifying it, you separate
////    the extensive behavior behind an abstract interface and then turn the dependencies around.
//// -> If you design your system in conformance to the Open-Close Principle. When you modify them, you
////    can do so by adding new code. Not by changing old code.
//// -> If you old code never gets modified, it will never rot…
//// -> When violation of the OCP, will lead to design smell like rigidity, fragility and immobility.
//// -> To create a system that perfectly conform to the OCP, one must be able to predict the future.
//// -> However, by using a iterative process, with lots of feedback and refactoring, we can infect develop
////    systems that conform well enough to the OCP
//
//enum class Color {Red, Green, Blue};
//enum class Size {Small, Medium, Large};
//
//struct Product {
//    std::string name;
//    Color color;
//    Size size;
//};
//
//// example bad: next to opening class up for extention, we are opening it up for modification -> violate OCP
//struct ProductFilter {
//    typedef std::vector<Product*> Items;
//    // first filter by color
//    static Items by_color(Items items, Color color) {
//        Items result;
//        for (auto& i: items) {
//            if (i->color == color) {
//                result.push_back(i);
//            }
//        }
//        return result;
//    }
//    // later filter by size
//    static Items by_size(Items items, Size size) {
//        Items result;
//        for (auto& i: items) {
//            if (i->size == size) {
//                result.push_back(i);
//            }
//        }
//        return result;
//    }
//    // even later filter by color and size
//    static Items by_color_and_size(Items items, Color color, Size size) {
//        Items result;
//        for (auto& i: items) {
//            if (i->color == color && i->size == size) {
//                result.push_back(i);
//            }
//        }
//        return result;
//    }
//
//};
//
//// specification pattern: used in data access to apply OCP
//template <typename T>
//struct ICriteria {
//    virtual bool is_satisfied(T* item) = 0;
//};
//
//template <typename T>
//struct IFilter {
//    virtual std::vector<T*> filter(std::vector<T*> items, ICriteria<T>& spec) = 0;
//};
//
//struct BetterFilter: IFilter<Product> {
//    typedef std::vector<Product*> Items;
//    std::vector<Product*> filter(std::vector<Product*> items, ICriteria<Product>& spec){
//        Items result;
//        for (auto& p : items) {
//            if (spec.is_satisfied(p)) {
//                result.push_back(p);
//            }
//        }
//        return result;
//    }
//};
//
//struct ColorCriteria: ICriteria<Product> {
//    Color color_;
//    explicit ColorCriteria(const Color color) : color_(color) {
//    }
//    bool is_satisfied(Product* item) override {
//        return item->color == color_;
//    }
//};
//
//struct SizeCriteria: ICriteria<Product> {
//    Size size_;
//    explicit SizeCriteria(const Size size) : size_(size) {
//    }
//    bool is_satisfied(Product* item) override {
//        return item->size == size_;
//    }
//};
//
//TEST(solid, ocp) { // never cast away const, instead use mutable
//    Product apple {"Apple", Color::Green, Size::Small};
//    Product tree {"Tree", Color::Green, Size::Large};
//    Product house {"House", Color::Blue, Size::Large};
//
//    std::vector<Product*> all {&apple, &tree, &house};
//    BetterFilter bf;
//    ColorCriteria green(Color::Green);
//
//    auto green_things = bf.filter(all, green);
//
//    EXPECT_EQ(green_things.size(), 2);
//    for (auto && x : green_things){
//        std::cout << x->name << " is green " << std::endl;
//    }
//}
//
//template <typename T> struct AndSpecification : ICriteria<T> {
//    ICriteria<T> &first;
//    ICriteria<T> &second;
//    AndSpecification(ICriteria<T>& first, ICriteria<T>& second) : first(first), second(second) {
//    }
//    bool is_satisfied(Product* item) override {
//        return first.is_satisfied(item) && second.is_satisfied(item);
//    }
//};
//
//TEST(solid, ocp2) { // never cast away const, instead use mutable
//    Product apple {"Apple", Color::Green, Size::Small};
//    Product tree {"Tree", Color::Green, Size::Large};
//    Product house {"House", Color::Blue, Size::Large};
//
//    std::vector<Product*> all {&apple, &tree, &house};
//    BetterFilter bf;
//    ColorCriteria green(Color::Green);
//    SizeCriteria big(Size::Large);
//    AndSpecification<Product> green_and_big(big, green);
//    auto green_and_big_things = bf.filter(all, green_and_big);
//
//    EXPECT_EQ(green_and_big_things.size(), 1);
//}
//
////////////////////////////////////////////////////////////////////////////////////////////
////      LSP - Liskov’s Substitution Principle
////      Object should be replaceable with instances of their subtypes without altering program correctness.
////History and theory of types
////- it does not matter what is inside a type, all that matters are the operation that can be performed on that type
////- e.g. as long as integer that represents 1, plus integer that represents 2 equals an integer that represents 3
////- thus a type is just a bag of operations (there might be data inside it, but those are hidden behind the operations)
////- that is precisely what a class is looking from outside in.
////Subtypes (e.g. typedef point{ double x,y; } and typedef describedPoint{double x,y; char* description}
////- describedPoint can be cast to point, but not the other way around
////- relationship between the point and describePoint is asymmetrical
////- describe point is a SUBPOINT of point
////        Subtypes - definition of Liskov
////- subtypes be used as there parent types, without the user knowing it
////- subtypes can be substituted for its parent, subtypes must have same methods as its parents, even though it has different implementations
////Both for static and dynamic
////- static: use inheritance
////- dynamic: use same methods name
////Square paradox: what can go wrong if you use inheritance intuitively instead of according to Liskovs definition
////- a square is a subtype of rectangle…?
////- however when you change height of the square, the with also changes
////- this is something that users don’t expect when they change a rectangle, but a square was passed
////- only way is to use “isInstanceOf(recatangle)” to check… this creates a dependency and violated the OCP
////- Best way of avoid these problems is by keeping square and rectangles as two completely different types, and never pass a square to a function that expect a rectangle
////- the “is a” relationship holds… but the problem is, this is not a rectangle, it is a piece of code that just represents a rectangle.
////- Representatives do not share the relationships of the things that they represent.
////Heuristics, examples and rules to detect violation of LSP
////- if the base class does something, the derived class must do it too. And it must do so in such a way, that it does not violate the expectation of the callers
////- you cannot take expected behaviours away from a subtype, it can do more, but never do less!
////- So if you have empty function (degenerative implementation), probably you have violated LSP
////- So if a derived function just throws an exception, only way to solve it again is using “isInstanceOf()”
////- what you see “if" IsInstanceOf()… this violate LSP when there is also a “else”
////- when you see a typecast
////        Easy to violate the LSP and how to repair them
////- use adepter pattern instead of inheritance
//
//class Rectangle {
//protected:
//    int width_;
//    int height_;
//public:
//    Rectangle(const int width, const int height) : width_(width), height_(height) {
//    }
//
//    virtual int GetWidth() const {
//        return width_;
//    }
//    virtual void SetWidth(const int width) {
//        this->width_ = width;
//    }
//    virtual int GetHeight() const {
//        return height_;
//    }
//    virtual void SetHeight(const int height) {
//        this->height_ = height;
//    }
//    int Area() const {
//        return width_ * height_;
//    }
//};
//
//int process (Rectangle& r) {
//    int w = r.GetWidth();
//    r.SetHeight(10);
//
//    return r.Area();
//}
//
//TEST(solid, LSP) {
//    Rectangle r{5,5};
//    int a = process (r);
//    EXPECT_EQ(a, 50);
//    // this all works fine
//}
//
//// now we violate LSP and see how it all breaks down!
//class Square : public Rectangle {
//public:
//    Square(int size) : Rectangle {size, size} {
//    }
//
//    void SetWidth(const int width) override {
//        this->width_ = height_ = width;
//    }
//    void SetHeight(const int height) override {
//        this->height_ = width_ = height;
//    }
//
//    // How do mitigate LSP violations? 1.in terms of assignmnts
//    // do not override SetHeight and SetWidth
//    // void SetSize(int size)
//    // however we cannot use this in the polymorphic function anymore...
//};
//
//TEST(solid, LSP_violation) {
//    Rectangle r{5,5};
//    int a = process (r);
//    EXPECT_EQ(a, 50);
//
//    Square s{5};
//    a = process(s);
//    EXPECT_NE(a, 50); // we expected this to be 50... however is 100:
//    // LSP: Object should be replaceable with instances of their subtypes without altering program correctness.
//}
//
//// How do mitigate LSP violations? 2. in terms of initialization
//// Do not use Square Class
//// For initialization: Use a Rectangular Factory instead
//struct RectangleFactory {
//    static Rectangle MakeRectangle(int w, int h);
//    static Rectangle MakeSquare(int size);
//};
//
////////////////////////////////////////////////////////////////////////////////////////////
////      ISP - Interface Segregation Principle
////      Many client-specific interfaces better than one general-purpose interface.
//// -> Name of Interfaces have more to do with its the classes that use them, than with the classes that implements them.
//// -> Fat classes problems:
//// -> -> classes that have lots of methods and used by many other classes with each there own use cases.
//// -> -> has lots of fan-in, which results in a major bottle-neck.
//// -> -> coupling involved in fat classes can be so severe -> make it impossible to take the clients and put them in
////       separate components (dll, jar) and independently deploy them.
//// -> When faces with a fat class, we isolate the class with its clients, by creating interfaces that those clients can use.
//// -> -> since interfaces are more logically coupled to the client that calls them than to the classes that implements
////       them, we make sure that those interfaces contain only the method that the clients wishes to call.
//// -> -> then we multiple inherent those interfaces into the original fat class.
//// -> -> changes made in function signatures in certain clients, will not cause the other clients to change. This
////       prevent them to be re-compiled and redeployed.
//// -> -> that means we can put those clients in separate components (dll, jar) and deploy them separately.
//// -> Fat classes creates a weird backwards coupling that comes from to much knowledge, changing one method in its
////    interfaces needs recompile and redeployment of all its client, that uses that interface.
//// -> -> goal if ISP is to prevent that backwards coupling, by insuring you don’t depend on methods that you do not call.
//// -> -> generalised: don’t depend on things that you do not need. Otherwise you will create coupling that will make
////       your software rigid and fragile.
//// -> Detect ISP violation: don’t depend on thing that you do not need.
//// -> -> create an instance of an object and pass in constructor arguments that you have no use for? You are passing in
////       arguments that you do not need, simply to satisfy someone else his needs.
//// -> -> build up a complex data structure to run a simple test.
//// -> -> fire up a web server and connect to a database just to test a simple business rule.
//// -> -> write a test that walks though the logging process simply to test a business rule.
//// -> -> need to call a function, but before you can, you need to call two more function, that you do not know or case
////       what they did.
//// -> Don’t force your "users" to depend on thing s they don’t need, whether your "users" are modules, people or tests.
//
//struct Document;
//
//// we create multifucntion device
//// from the ISP we agree to use abscractions rather than concrete implementations
//struct IMachine {
//    virtual void print(std::vector<Document*> docs) = 0;
//    virtual void scan(std::vector<Document*> docs) = 0;
//    virtual void fax(std::vector<Document*> docs) = 0;
//};
//
//struct MutifunctionalPheriperal : IMachine {
//    void print(std::vector<Document*> docs) override;
//    void scan(std::vector<Document*> docs) override;
//    void fax(std::vector<Document*> docs) override;
//    // according to ISP this is a bad idea..
//    // 1. everytime you change just a part of the functionality e.g. print mechanism, without
//    //      touching scan and fax mechanism, you would have to recompile because its part of
//    //      a single file.
//    // 2. it could be that the user just want device to print, and does not care about scan and fax
//    //      or does not know how to implement scan or fax yet. Your are forcing the implementer to
//    //      implement too much!
//
//    // No client should be forced to depend on methods it does not use.
//    // ISP is all about breaking up these monolithic monstrocities and doing it in a piece-wise fashion.
//};
//
//// how do we do it better?
//struct Printable {
//    virtual void print(std::vector<Document*> docs) = 0;
//};
//struct IScanner {
//    virtual void scan(std::vector<Document*> docs) = 0;
//};
//struct IFaxer {
//    virtual void fax(std::vector<Document*> docs) = 0;
//};
//
//struct Printer : Printable {
//    void print (std::vector<Document*> docs) override {
//        std::cout << "print something" << std::endl;
//    }
//};
//struct Scanner : IScanner {
//    void scan (std::vector<Document*> docs) override {
//        std::cout << "scan something" << std::endl;
//    }
//};
//struct Faxer : IFaxer {
//    void fax (std::vector<Document*> docs) override {
//        std::cout << "fax something" << std::endl;
//    }
//};
//
//struct IBetterMachine : Printable, IScanner {};
//
//struct BetterMachine : IBetterMachine {
//    Printable& printer_;  // decorator pattern: machines aggregates functionalities of both printer and scanner
//    IScanner& scanner_;
//
//    BetterMachine(Printable &printer, IScanner scanner)
//    // use dependency injection to give functionality to machine.
//    // so if later we have a color printer, we can inject that one instead.
//            : printer_{ printer }, scanner_{ scanner } {
//    }
//    void print (std::vector<Document*> docs) override {
//        printer_.print(docs);
//    }
//    void scan (std::vector<Document*> docs) override {
//        scanner_.scan(docs);
//    }
//};
//
////////////////////////////////////////////////////////////////////////////////////////////
////      DIP - Dependency Inversion Principle
////      Dependencies should be abstract rather than concrete. In other words, dependencies on interfaces
////      and supertypes is better than dependencies on concrete types.
////
//// -> What dependencies there are, what there cost are and how to arrange them into a good architecture
//// -> -> Runtime dependency: whenever two modules interact at runtime
//// -> -> ->  when flow of control leaves one module and enters another
//// -> -> -> when one module access the variables of another
//// -> -> Compile time dependency: when a name is defined in one module, but appears in another module
////       You cannot just compile one module, you need to first compile the modules that that module depend upon.
//// -> -> -> e.g. Dependency of A->B->C. When compiling A, need to first compile C and B.
////          Structure design
//// -> -> topdown design, starting with main, until you specify every methods
//// -> -> compile time dependencies (source code dependencies) run in the same direction as the runtime dependencies
//// -> -> when source code dependencies are based on this structure, then we have a difficult time of keeping those
////      dependencies of crossing team boundaries
//// -> -> we need to make the runtime dependencies run different than the source code dependencies structure!
////
//// -> Dependency Inversion/Inversion of Control(IoC)
//// -> -> the actual process of creating abstractions and getting them to replace dependencies.
//// -> -> use polymorphism
//// -> - A -> B with f() (both runtime and completive dependency)
//// -> - polymorphism: A -> Interface with f() <|- B with f(), A would use interface, B would implement interface
//// -> - now A still has runtime dependency on B, it does not have the compile time dependencies
//// -> - Both A and B have source code dependencies on the interface!
//// -> - note that the source code dependencies of B upon the interface, points in the opposite direction of the runtime dependency of A upon B
//// -> - dependencies are inverted whenever the source code dependencies opose the direction of the flow of control
//// -> - this is the way how we create boundaries in our software modules
//// -> - whenever we want boundaries to exist, we careful choose which dependencies to invert against the flow of control, so that all the dependencies point to the same direction across the boundary
//// -> - boundary like this is how we create plug-ins:
//// -> -plugin is a module, that is anonymously called by another module, caller has no idea who is calling
//// -> - that way you create an independently deployable and developable architecture is to compose it using plugins
//// -> - thus devide the system with boundaries and invert the dependencies that cross this boundaries
////        Architectural Implications: High level policies(e.g. use cases) should not depend on low level details (e.g. databases, web formatting), low level details should depend on high level policy
//// -> -  a good architecture is a plugin architecture using the DIP
//
//struct Engine {
//    float volume = 5;
//    int horse_power = 400;
//
//    friend std::ostream & operator << (std::ostream& os, const Engine& obj) {
//        return os
//            << "volume: " << obj.volume
//            << " horse_power: " << obj.horse_power;
//    }
//};
//
//struct Car {
//    std::shared_ptr<Engine> engine;
//    Car(std::shared_ptr<Engine> e):engine(e) {
//    }
//
//    friend std::ostream& operator << (std::ostream& os, const Car& obj) {
//        return os << "car with Engine: " << *obj.engine;
//    }
//};
//
//TEST(solid, DIP) {
//    auto e = std::make_shared<Engine>();
//    Car c(e);
//    std::cout << c << std::endl;
//}