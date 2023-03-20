
#include <git2.h>
#include <gul14/catch.h>
#include "taskolib/GitWrapper.h"
#include "taskolib/serialize_sequence.h"
#include "taskolib/Step.h"
#include "taskolib/Sequence.h"

using namespace task;



TEST_CASE("Construct LibGit object", "[GitWrapper]")
{
    // prepare first sequence for test
    Step step_1_01{Step::type_while};
    step_1_01.set_label("while");
    step_1_01.set_script("return i < 10");

    Step step_1_02{Step::type_action};
    step_1_02.set_label("action");
    step_1_02.set_script("i = i + 1");

    Sequence seq_1{"test.seq.1"};
    seq_1.push_back(step_1_01);
    seq_1.push_back(step_1_02);

    store_sequence("sequences/unit_test_1", seq_1);


    // Create Git Library
    LibGit gl{"sequences"};
    

    REQUIRE(not gl.get_path().empty());
    REQUIRE(gl.get_path() == "sequences/.git");
    
    
    // Test if repo got initialized
    REQUIRE(gl.get_last_commit_message() == "Initial commit");

}