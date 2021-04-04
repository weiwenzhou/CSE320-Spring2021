#include "test_common.h"

#define TEST_REF_DIR "tests/rsrc"
#define TEST_OUTPUT_DIR "tests.out"

int exists(const char *fname) {
    FILE *file;
    if ((file = fopen(fname, "r"))) {
        fclose(file);
        return 1;
    } return 0;
}


long getFileLines(char *file) {
    char str[80];
    sprintf(str, "wc -l %s|awk '{print $1}'", file);
    FILE *cmd=popen(str, "r");
    char result[24]={0x0};
    long line_num = 0 ;
    while (fgets(result, sizeof(result), cmd) !=NULL) {
        char *ptr;
        line_num = strtol(result, &ptr, 10);
        break ;
    }
    pclose(cmd);
    return line_num ;
}

int getWLOutput(char *wl_cmd) {
    FILE *cmd=popen(wl_cmd, "r");
    char result[24]={0x0};
    long line_num = 0 ;
    while (fgets(result, sizeof(result), cmd) !=NULL) {
        char *ptr;
        line_num = strtol(result, &ptr, 10);
        break ;
    }
    pclose(cmd);
    return line_num ;
}

/*
 * test segfault if the program encountered without any moves
 *
 */
Test(grading_suite, segfault, .timeout=5) {
    char *name = "segfault";
    sprintf(program_options, "rsrc/no-moves.ntn");
    int err = run_using_system(name, "", "");
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
    assert_errfile_matches(name, "");
}

Test(valgrind_suite, segfault, .timeout=5) {
    char *name = "valgrind_segfault";
    sprintf(program_options, "rsrc/no-moves.ntn");
    int err = run_using_system(name, "", "valgrind --error-exitcode=37");
    assert_no_valgrind_errors(err);
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
    assert_errfile_matches(name, "^==");
}

/*
 * test no_new_move, check the initialization of variable m within notation_main
 *
 * Note: valgrind_uninitialized is serving for more general purpose
 */
Test(grading_suite, no_new_move, .timeout=5) {
    char *name = "no_new_move";
    sprintf(program_options, "rsrc/algebric.ntn");
    run_using_system(name, "", "valgrind --leak-check=no --undef-value-errors=yes --error-exitcode=37");

    // un-initialization error happens inside function notation_main
    if (exists("./tests.out/no_new_move.err")) {
        if (getWLOutput("cat ./tests.out/no_new_move.err | grep 'notation_main' |wc -l") > 0 &&
            getWLOutput("cat ./tests.out/no_new_move.err | grep 'Access not within mapped region at address'|wc -l") > 0) {
            cr_assert(0 > 1) ;
        }
    }
}

/*
 * Tests -o option for bug: omit_break
 */
Test(grading_suite, omit_break, .timeout=5) {
    char *name = "omit_break";
    sprintf(program_options, " -o ./tests.out/omit_break.log rsrc/algebric.ntn");
    int err = run_using_system(name, "", "");
    assert_normal_exit(err);
    assert_files_matches("omit_break.log", "omit_break.log");
}

Test(valgrind_suite, omit_break, .timeout=5) {
    char *name = "valgrind_omit_break";
    sprintf(program_options, " -o ./tests.out/omit_break.log rsrc/algebric.ntn");
    int err = run_using_system(name, "", "valgrind --error-exitcode=37");
    assert_no_valgrind_errors(err);
    assert_normal_exit(err);
    assert_files_matches("omit_break.log", "omit_break.log");
}

/*
 * Tests the basic program operation: shortened notation
 */
Test(base_suite, quick, .timeout=5) {
    char *name = "quick";
    sprintf(program_options, "-s rsrc/algebric.ntn");
    int err = run_using_system(name, "", "");
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
    assert_errfile_matches(name, "");
}

Test(valgrind_suite, quick, .timeout=5) {
    char *name = "valgrind_quick";
    sprintf(program_options, "-s rsrc/algebric.ntn");
    int err = run_using_system(name, "", "valgrind --error-exitcode=37");
    assert_no_valgrind_errors(err);
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
    assert_errfile_matches(name, "^==");
}

/*
 * Tests the basic program operation: algebraic notation
 */
Test(grading_suite, quick_2, .timeout=5) {
    char *name = "quick_2";
    sprintf(program_options, "-a rsrc/algebric.ntn");
    int err = run_using_system(name, "", "");
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
    assert_errfile_matches(name, "");
}

Test(valgrind_suite, quick_2, .timeout=5) {
    char *name = "valgrind_quick_2";
    sprintf(program_options, "-a rsrc/algebric.ntn");
    int err = run_using_system(name, "", "valgrind --error-exitcode=37");
    assert_no_valgrind_errors(err);
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
    assert_errfile_matches(name, "^==");
}

/*
 * Tests the basic program operation: output: english
 */
Test(grading_suite, quick_3, .timeout=5) {
    char *name = "quick_3";
    sprintf(program_options, "-t french rsrc/algebric.ntn");
    int err = run_using_system(name, "", "");
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
    assert_errfile_matches(name, "");
}

Test(valgrind_suite, quick_3, .timeout=5) {
    char *name = "valgrind_quick_3";
    sprintf(program_options, "-t french rsrc/algebric.ntn");
    int err = run_using_system(name, "", "valgrind --error-exitcode=37");
    assert_no_valgrind_errors(err);
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
    assert_errfile_matches(name, "^==");
}

/*
 * Tests the basic program operation: test flag -e
 */
Test(grading_suite, quick_4, .timeout=5) {
    char *name = "quick_4";
    sprintf(program_options, "-e 10 rsrc/algebric.ntn");
    int err = run_using_system(name, "", "");
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
    assert_errfile_matches(name, "");
}

Test(valgrind_suite, quick_4, .timeout=5) {
    char *name = "valgrind_quick_4";
    sprintf(program_options, "-e 10 rsrc/algebric.ntn");
    int err = run_using_system(name, "", "valgrind --error-exitcode=37");
    assert_no_valgrind_errors(err);
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
    assert_errfile_matches(name, "^==");
}

/*
 * Tests the basic program operation: test flag -d
 */
Test(grading_suite, quick_d, .timeout=5) {
    char *name = "quick_d";
    sprintf(program_options, "-d tex rsrc/algebric.ntn");
    int err = run_using_system(name, "", "");
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
    assert_errfile_matches(name, "");
}

Test(valgrind_suite, quick_d, .timeout=5) {
    char *name = "valgrind_quick_d";
    sprintf(program_options, "-d tex rsrc/algebric.ntn");
    int err = run_using_system(name, "", "valgrind --error-exitcode=37");
    assert_no_valgrind_errors(err);
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
    assert_errfile_matches(name, "^==");
}

Test(grading_suite, long_arg, .timeout=5) {
    char *name = "long_arg";
    sprintf(program_options, "--long-algebraic rsrc/algebric.ntn");
    int err = run_using_system(name, "", "");
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
    assert_errfile_matches(name, "");
}

Test(valgrind_suite, long_arg, .timeout=5) {
    char *name = "valgrind_long_arg";
    sprintf(program_options, "--long-algebraic rsrc/algebric.ntn");
    int err = run_using_system(name, "", "valgrind --error-exitcode=37");
    assert_no_valgrind_errors(err);
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
    assert_errfile_matches(name, "^==");
}

Test(grading_suite, arg_prefix, .timeout=5) {
    char *name = "arg_prefix";
    sprintf(program_options, "--long rsrc/algebric.ntn");
    int err = run_using_system(name, "", "");
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
    assert_errfile_matches(name, "");
}

Test(valgrind_suite, arg_prefix, .timeout=5) {
    char *name = "valgrind_arg_prefix";
    sprintf(program_options, "--long rsrc/algebric.ntn");
    int err = run_using_system(name, "", "valgrind --error-exitcode=37");
    assert_no_valgrind_errors(err);
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
    assert_errfile_matches(name, "^==");
}

/*
 * Tests the basic program operation: file name argument anywhere in command line
 */
Test(grading_suite, quick_location, .timeout=5) {
    char *name = "quick_location";
    sprintf(program_options, "rsrc/algebric.ntn -e 100");
    int err = run_using_system(name, "", "");
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
    assert_errfile_matches(name, "");
}

Test(valgrind_suite, quick_location, .timeout=5) {
    char *name = "valgrind_quick_location";
    sprintf(program_options, "rsrc/algebric.ntn -e 100");
    int err = run_using_system(name, "", "valgrind --error-exitcode=37");
    assert_no_valgrind_errors(err);
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
    assert_errfile_matches(name, "^==");
}


Test(base_suite, variations, .timeout=5) {
    char *name = "variations";
    sprintf(program_options, "rsrc/boudy.ntn");
    int err = run_using_system(name, "", "");
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
    assert_errfile_matches(name, "");
}

Test(valgrind_suite, variations, .timeout=5) {
    char *name = "valgrind_variations";
    sprintf(program_options, "rsrc/boudy.ntn");
    int err = run_using_system(name, "", "valgrind --error-exitcode=37");
    assert_no_valgrind_errors(err);
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
    assert_errfile_matches(name, "^==");
}

/*
 * Tests "garbage input".  Should exit with error status, but not crash.
 */
Test(base_suite, garbage_input, .timeout=5) {
    char *name = "garbage_input";
    sprintf(program_options, "src/main.c");
    int err = run_using_system(name, "", "");
    assert_expected_status(1, err);
}

/*
 * Tests "garbage input".  Should exit with error status, but not crash.
 */
Test(valgrind_suite, garbage_input, .timeout=5) {
    char *name = "valgrind_garbage_input";
    sprintf(program_options, "src/main.c");
    int err = run_using_system(name, "", "valgrind --error-exitcode=37");
    assert_no_valgrind_errors(err);
    assert_expected_status(1, err);
}

/*
 * This test runs valgrind to check for the use of uninitialized variables.
 */
Test(valgrind_suite, uninitialized, .timeout=5) {
    char *name = "valgrind_uninitialized";
    sprintf(program_options, "rsrc/algebric.ntn");
    int err = run_using_system(name, "", "valgrind --leak-check=no --undef-value-errors=yes --error-exitcode=37");
    assert_no_valgrind_errors(err);
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
}

/*
 * This test runs valgrind to check for memory leaks.
 * test on bug: no_free_tos
 * --errors-for-leak-kinds=definite,indirect (default value)
 */
Test(valgrind_suite, leak, .timeout=5) {
    char *name = "valgrind_leak";
    sprintf(program_options, "rsrc/boudy.ntn");
    int err = run_using_system(name, "", "valgrind --leak-check=full --undef-value-errors=no --error-exitcode=37");
    assert_no_valgrind_errors(err);
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
}

/*
 * This test runs valgrind to check for memory leaks.
 * test on general memory leak and catch all kinds of errors
 */
Test(valgrind_suite, leak_1, .timeout=5) {
    char *name = "valgrind_leak_1";
    sprintf(program_options, "rsrc/algebric.ntn");
    int err = run_using_system(name, "", "valgrind --leak-check=full --errors-for-leak-kinds=definite,indirect,possible,reachable --undef-value-errors=no --error-exitcode=37");
    assert_no_valgrind_errors(err);
    assert_normal_exit(err);
    assert_outfile_matches(name, NULL);
}

/*
 * test sizeof_error
 *
 * check if initialization of (depl *) malloc (sizeof(depl *)) exists
 */
Test(grading_suite, sizeof_error, .timeout=5) {
    FILE *cmd=popen("ulimit -t 10; grep -r -E '\\(\\s*depl\\s*\\*\\s*\\)\\s*malloc\\s*\\(\\s*sizeof\\s*\\(\\s*depl\\s*\\*\\s*\\)\\s*\\)' ./src/notation.c |wc -l", "r");
    char result[24]={0x0};
    long line_num = 0 ;
    int new_move_err = 0 ;
    while (fgets(result, sizeof(result), cmd) !=NULL) {
        char *ptr;
        line_num = strtol(result, &ptr, 10);
        break ;
    }
    pclose(cmd);

    // avoid negative false, for example, commented line
    char *name = "sizeof_error";
    sprintf(program_options, "rsrc/boudy.ntn");
    run_using_system(name, "", "valgrind --leak-check=full --errors-for-leak-kinds=reachable --undef-value-errors=no --error-exitcode=37");
    if (exists("./tests.out/sizeof_error.err")) {
        if (getWLOutput("cat ./tests.out/sizeof_error.err | grep 'new_move' |wc -l") > 0 ) {
            new_move_err = 1 ;
        }
    }

    if (line_num > 0 && new_move_err > 0) {
        cr_assert(0 > 1) ;
    }
}

/*
 * test omit_string_init
 *
 * if without debcol[0]  = '\0', the output of the program will mess up and have strange characters
 */
Test(grading_suite, omit_string_init, .timeout=5) {
    FILE *cmd=popen("ulimit -t 10; grep -r -E 'debcol\\s*\\[\\s*0\\s*\\]\\s*=\\s*' ./src/drivers.c|wc -l", "r");
    char result[24]={0x0};
    long line_num = 0 ;
    int output_err = 0 ;
    while (fgets(result, sizeof(result), cmd) !=NULL) {
        char *ptr;
        line_num = strtol(result, &ptr, 10);
        break ;
    }
    pclose(cmd);

    // avoid negative false, for example
    //       for (int i=0; i<16; i++) debcol[i] = '\0' ;
    char *name = "omit_string_init";
    sprintf(program_options, "rsrc/boudy.ntn");
    run_using_system(name, "", "");
    output_err = outfile_matches_errcode(name, NULL);

    if (line_num < 2 && output_err > 0) {
        cr_assert(0 > 1) ;
    }
}
