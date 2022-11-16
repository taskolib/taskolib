#include <iostream>
#include "taskolib/Step.h"

int main()
{
    using namespace task;

    // Set up a step that imports variables "a" and "b" and returns their sum in "sum"
    Step step;
    step.set_used_context_variable_names(VariableNames{ "a", "b", "sum" });
    step.set_script("sum = a + b");

    // Create a context and store values for "a" and "b" in it
    Context context;
    context.variables["a"] = LuaInteger{ 42 };
    context.variables["b"] = LuaFloat{ -41.5 };

    // Execute the step with the context
    step.execute(context);

    // Retrieve variables from the context with std::get()
    std::cout << "According to LUA, the sum of "
              << std::get<LuaInteger>(context.variables["a"])
              << " and " << std::get<LuaFloat>(context.variables["b"]) << " is "
              << std::get<LuaFloat>(context.variables["sum"]) << ".\n";

    return 0;
}
