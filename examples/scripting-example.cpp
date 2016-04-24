#ifdef HAVE_SPIDERMONKEY

#include <Object.h>
#include <JSScriptEngine.h>
#include <cstdio>
#include <iostream>

// That is our program inside a program
const char szScriptProgram[] = R"#(
var pt = new Point();
pt.x = pt.y = 5
console.printPoint(pt);
var pt2 = new Point(2, 3)
console.printPoint(pt2);
pt.addX(12);
pt.addY(-29);
console.log("Point moved { x = %d, y = %d }", [ pt.x, pt.y ]);
console.log("Distance between points now: %g", [ pt.distanceTo(pt2) ])
// attempt to call a function with incompatible parameters will raise an exception
try
{
    pt.addX('12');  // function can't be called with string parameter
}
catch (err)
{
    console.error("Exception raised: %s", [ err.toString() ]);
}
)#";

using namespace metacpp;
using namespace metacpp::scripting::js;

// Define our reflectible classes.

struct Point : public Object {
    explicit Point(int x = 0, int y = 0)
        : x(x), y(y)
    {
    }

    // All properties and methods exposed to the script engine must be public
    int x, y;

    void addX(int v) { x += v; }
    void addY(int v) { y += v; }
    // Euclidean distance between two points
    double distanceTo(Point *other) const
    {
        double dx = x - other->x;
        double dy = y - other->y;
        return sqrt(pow(dx, 2) + pow(dy, 2));
    }

    // this macro simply declares service function for the object introspection
    // and also for dynamic object instantiation and destruction
    META_INFO_DECLARE(Point)
};

// Declare exposed object methods
METHOD_INFO_BEGIN(Point)
    CONSTRUCTOR(Point)
    CONSTRUCTOR(Point, int, int)
    METHOD(Point, addX)
    METHOD(Point, addY)
    METHOD(Point, distanceTo)
METHOD_INFO_END(Point)

// And properties
STRUCT_INFO_BEGIN(Point)
    FIELD(Point, x)
    FIELD(Point, y)
STRUCT_INFO_END(Point)

// REFLECTIBLE_FM means that class have both the reflected fields and methods
// Use REFLECTIBLE_F to expose only fields
// and REFLECTIBLE_M to expose only methods
REFLECTIBLE_FM(Point)
// define service methods
META_INFO(Point)

// The same for another reflectible class
struct console : public Object {
    // VariantArray can be used to pass variable number of arguments to the script
    static void log(const String& fmt, const VariantArray& args)
    {
        std::cerr << "Log: " << String::format(fmt.c_str(), args) << std::endl;
    }

    static void info(const String& fmt, const VariantArray& args)
    {
        std::cerr << "Info: " << String::format(fmt.c_str(), args) << std::endl;
    }

    static void warn(const String& fmt, const VariantArray& args)
    {
        std::cerr << "Warn: " << String::format(fmt.c_str(), args) << std::endl;
    }

    static void error(const String& fmt, const VariantArray& args)
    {
        std::cerr << "Error: " << String::format(fmt.c_str(), args) << std::endl;
    }

    static void printPoint(const Point *pt) {
        std::cout << "Point { x = " << pt->x << ", y = " << pt->y << " }" << std::endl;
    }

    META_INFO_DECLARE(console)
};

METHOD_INFO_BEGIN(console)
    METHOD(console, log)
    METHOD(console, info)
    METHOD(console, warn)
    METHOD(console, error)
    METHOD(console, printPoint)
METHOD_INFO_END(console)

REFLECTIBLE_M(console)

META_INFO(console)

int main(int argc, char **argv)
{
    try
    {
        // Instantiate the engine first
        JSScriptEngine engine;
        // Register bindings to our reflectible classes via their MetaObject
        engine.registerClass(Point::staticMetaObject());
        engine.registerClass(console::staticMetaObject());
        // Create a script program
        auto program = engine.createProgram();
        // Compile it program
        std::istringstream ss(szScriptProgram);
        program->compile(ss, "example.js");
        // Create a script thread and run it
        auto thread = program->createThread();
        thread->run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}

#else

#include <iostream>

int main(int argc, char **argv)
{
    std::cerr << "The metacpp has been built with no JS support" << std::endl;
    return 1;
}

#endif