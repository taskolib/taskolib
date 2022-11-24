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
    context.variables["a"] = StepVariable::Integer{ 42 };
    context.variables["b"] = StepVariable::Float{ -41.5 };

    // Execute the step with the context
    step.execute(context);

    // Retrieve variables from the context with std::get()
    std::cout << "According to LUA, the sum of "
              << std::get<StepVariable::Integer>(context.variables["a"])
              << " and " << std::get<StepVariable::Float>(context.variables["b"]) << " is "
              << std::get<StepVariable::Float>(context.variables["sum"]) << ".\n";

    return 0;
}
