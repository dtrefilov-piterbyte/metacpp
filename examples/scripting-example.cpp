#include <Object.h>
#include <JSScriptEngine.h>
#include <cstdio>

// That is our program inside a program
const char szScriptProgram[] = R"#(
var pt = new Point();
pt.x = pt.y = 5
console.log("Point created { x = %d, y = %d }", [ pt.x, pt.y ]);
pt.addX(12);
pt.addY(-29);
console.log("Point moved { x = %d, y = %d }", [ pt.x, pt.y ]);
// GC objects can be passed to native functions
console.printPoint(pt);
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
    // All properties and methods exposed to the script engine must be public
    int x = 0, y = 0;

    void addX(int v) { x += v; }
    void addY(int v) { y += v; }

    // this macro simply declares service function for the object introspection
    // and also for dynamic object instantiation and destruction
    META_INFO_DECLARE(Point)
};

// Declare exposed object methods
METHOD_INFO_BEGIN(Point)
    METHOD(Point, addX)
    METHOD(Point, addY)
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
