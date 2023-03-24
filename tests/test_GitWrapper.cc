
#include <git2.h>
#include <gul14/catch.h>
#include "taskolib/GitWrapper.h"
#include "taskolib/serialize_sequence.h"
#include "taskolib/Step.h"
#include "taskolib/Sequence.h"
#include <filesystem>

using namespace task;



TEST_CASE("Construct LibGit object", "[GitWrapper]")
{
    // make sure directory is empty
    std::filesystem::remove_all("sequences");


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
    REQUIRE(gl.get_path() == "sequences");
    git_reference *temp;
    REQUIRE(git_repository_head(&temp, gl.get_repo()) == 0);
    
    
    // Test if repo_ got initialized
    REQUIRE(gl.get_last_commit_message() == "Initial commit");

}


TEST_CASE("Commit", "[GitWrapper]")
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
    
    
    // Check if repo_ can be found again
    REQUIRE(gl.get_last_commit_message() == "Initial commit");

    // prepare another sequence for test
    Step step_2_01{Step::type_while};
    step_2_01.set_label("while");
    step_2_01.set_script("return i < 10");

    Step step_2_02{Step::type_action};
    step_2_02.set_label("action");
    step_2_02.set_script("i = i + 1");

    Sequence seq_2{"test.seq.2"};
    seq_2.push_back(step_2_01);
    seq_2.push_back(step_2_02);

    store_sequence("sequences/unit_test_2", seq_2);

    gl.libgit_add(); // add unit_test_2
    gl.libgit_commit("Add second sequence");

    REQUIRE(gl.get_last_commit_message() == "Add second sequence");

}


TEST_CASE("Delete Sequence", "[GitWrapper]")
{

    // Create Git Library
    LibGit gl{"sequences"};
    
    std::filesystem::path mypath = "unit_test_2";

    gl.libgit_remove_sequence(mypath);
    
    // check if path got removed
    //REQUIRE(not std::filesystem::exists("sequences" / mypath));
    for (const auto & entry : std::filesystem::directory_iterator("sequences"))
        //REQUIRE(entry.path() == ".git");
        std::cout << entry.path();

}