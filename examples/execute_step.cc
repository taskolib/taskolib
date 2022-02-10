#include <iostream>
#include "avtomat/execute_step.h"

int main()
{
    using namespace task;

    // Set up a step that imports variables "a" and "b" and returns their sum in "sum"
    Step step;
    step.set_imported_variable_names(VariableNames{ "a", "b" });
    step.set_script("sum = a + b");
    step.set_exported_variable_names(VariableNames{ "sum" });

    // Create a context and store values for "a" and "b" in it
    Context context;
    context["a"] = 42LL;
    context["b"] = -41.5;

    // Execute the step with the context
    execute_step(step, context);

    // Retrieve variables from the context with std::get()
    std::cout << "According to LUA, the sum of " << std::get<long long>(context["a"])
              << " and " << std::get<double>(context["b"]) << " is "
              << std::get<double>(context["sum"]) << ".\n";

    return 0;
}
