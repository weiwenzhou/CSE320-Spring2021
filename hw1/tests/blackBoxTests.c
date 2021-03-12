#include<criterion/criterion.h>
#include<criterion/logging.h>
#include "const.h"

/* Test given to the students */
Test(basecode_tests_suite, birp_basic_test, .timeout=10) {

	system("mkdir -p test_output");
	char *cmd = "ulimit -t 10; bin/birp -i pgm < rsrc/M.pgm > test_output/M.birp";
	char *cmp = "diff -B test_output/M.birp rsrc/M.birp";

	int return_code = WEXITSTATUS(system(cmd));
	cr_assert_eq(return_code, EXIT_SUCCESS,
			"Program exited with %d instead of EXIT_SUCCESS",
			return_code);
	return_code = WEXITSTATUS(system(cmp));
	cr_assert_eq(return_code, EXIT_SUCCESS,
			"Program output did not match reference output.");
}

Test(blackbox_tests, pgm_2_birp1, .timeout=10){
	system("mkdir -p test_output");


	char *cmd = "ulimit -t 10; bin/birp -i pgm < rsrc/M.pgm > test_output/M.birp";
	char *cmp = "cmp test_output/M.birp rsrc/M.birp";

	int return_code = WEXITSTATUS(system(cmd));
	cr_assert_eq(return_code, EXIT_SUCCESS,
			"Program exited with %d instead of EXIT_SUCCESS",
			return_code);
	return_code = WEXITSTATUS(system(cmp));
	cr_assert_eq(return_code, EXIT_SUCCESS,
			"Program output did not match reference output.");
}

Test(blackbox_tests, birp_2_pgm1, .timeout=10){
	char* cmd = "ulimit -t 10; bin/birp -o pgm < rsrc/M.birp > test_output/M.pgm";
	char* cmp = "cmp test_output/M.pgm rsrc/newImages/test_M.pgm";


	int ret_code = WEXITSTATUS(system(cmd));
	cr_assert_eq(ret_code, EXIT_SUCCESS, "Program exited with return code %d instead of EXIT_SUCCESS", ret_code);

	ret_code = WEXITSTATUS(system(cmp));
	cr_assert_eq(ret_code, EXIT_SUCCESS, "M.pgm cmp did NOT match reference output.");
}

Test(blackbox_tests, birp_2_pgm_2, .timeout=10){

	system("mkdir -p test_output");


	char *cmd = "ulimit -t 10; bin/birp -o pgm < rsrc/checker.birp > test_output/checker.pgm";
	char *cmp = "cmp test_output/checker.pgm rsrc/newImages/test_checker.pgm";


	int ret_code = WEXITSTATUS(system(cmd));
	cr_assert_eq(ret_code, EXIT_SUCCESS, "Program exited with return code %d instead of EXIT_SUCCESS", ret_code);

	ret_code = WEXITSTATUS(system(cmp));
	cr_assert_eq(ret_code, EXIT_SUCCESS, "checker.pgm did NOT match reference output.");
}

Test(blackbox_tests, birp_2_pgm_3, .timeout=10){

	system("mkdir -p test_output");

	char *cmd = "ulimit -t 10; bin/birp -o pgm < rsrc/cour25.birp > test_output/cour25.pgm";
	char *cmp = "cmp test_output/cour25.pgm rsrc/newImages/test_cour25.pgm";


	int ret_code = WEXITSTATUS(system(cmd));
	cr_assert_eq(ret_code, EXIT_SUCCESS, "Program exited with return code %d instead of EXIT_SUCCESS", ret_code);

	ret_code = WEXITSTATUS(system(cmp));
	cr_assert_eq(ret_code, EXIT_SUCCESS, "cour25.pgm did NOT match reference output.");
}

Test(blackbox_tests, pgm_2_birp_cat, .timeout=10){

	system("mkdir -p test_output");

	char *cmd = "ulimit -t 10; bin/birp -i pgm < rsrc/newImages/cat.pgm > test_output/cat.birp";
	char *cmp = "cmp test_output/cat.birp rsrc/newImages/cat.birp";


	int ret_code = WEXITSTATUS(system(cmd));
	cr_assert_eq(ret_code, EXIT_SUCCESS, "Program exited with return code %d instead of EXIT_SUCCESS", ret_code);

	ret_code = WEXITSTATUS(system(cmp));
	cr_assert_eq(ret_code, EXIT_SUCCESS, "cat.birp did NOT match reference output.");
}

Test(black_box_tests, birp_to_pgm_cat, .timeout=10){

	system("mkdir -p test_output");

	char *cmd = "ulimit -t 10; bin/birp -o pgm < rsrc/newImages/cat.birp > test_output/cat.pgm";
	char *cmp = "cmp test_output/cat.pgm rsrc/newImages/test_cat.pgm";


	int ret_code = WEXITSTATUS(system(cmd));
	cr_assert_eq(ret_code, EXIT_SUCCESS, "Program exited with return code %d instead of EXIT_SUCCESS", ret_code);

	ret_code = WEXITSTATUS(system(cmp));
	cr_assert_eq(ret_code, EXIT_SUCCESS, "cat.pgm did NOT match reference output.");

}

Test(blackbox_tests, both_to_ascii, .timeout=10){

	system("mkdir -p test_output");
	char *cmd1 = "ulimit -t 10; bin/birp -o ascii < rsrc/cour25.birp > test_output/cour25.asc";
	char *cmd2 = "ulimit -t 10; bin/birp -i pgm -o ascii < rsrc/cour25.pgm > test_output/cour25-2.asc";
	char *cmp = "diff -B test_output/cour25.asc test_output/cour25-2.asc";


	int retCode1 = WEXITSTATUS(system(cmd1));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "birp to ascii did not return with EXIT_SUCCESS on cour25. Returned with %d", retCode1);
	int retCode2 = WEXITSTATUS(system(cmd2));
	cr_assert_eq(retCode2, EXIT_SUCCESS, "pgm to ascii did not return with EXIT_SUCCESS on cour25. Returned with %d", retCode2);

	retCode1 = WEXITSTATUS(system(cmp));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "pgm_to_ascii and birp_to_ascii do not match output");
}


Test(blackbox_tests, allWhite_to_pgm, .timeout=10){

	system("mkdir -p test_output");

	char *cmd1 = "ulimit -t 10; bin/birp -o pgm < rsrc/newImages/AllWhite.birp > test_output/AllWhite.pgm";
	char *cmp = "cmp rsrc/newImages/test_AllWhite.pgm test_output/AllWhite.pgm";



	int retCode1 = WEXITSTATUS(system(cmd1));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "birp to pgm did not return with EXIT_SUCCESS on AllWhite. Returned with %d", retCode1);
	//int retCode2 = WEXITSTATUS(system(cmd2));
	//c/r_assert_eq(retCode2, EXIT_SUCCESS, "pgm to ascii did not return with EXIT_SUCCESS on cour25. Returned with %d", retCode2);

	retCode1 = WEXITSTATUS(system(cmp));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "test_output/AllWhite.pgm does not match reference output");

}

Test(blackbox_tests, allWhite_to_ASCII, .timeout=10){

	system("mkdir -p test_output");

	char *cmd1 = "ulimit -t 10; bin/birp -o ascii < rsrc/newImages/AllWhite.birp > test_output/AllWhite.asc";
	char *cmp = "diff -B rsrc/newImages/AllWhite.asc test_output/AllWhite.asc";



	int retCode1 = WEXITSTATUS(system(cmd1));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "birp to asc did not return with EXIT_SUCCESS on AllWhite. Returned with %d", retCode1);
	//int retCode2 = WEXITSTATUS(system(cmd2));
	//c/r_assert_eq(retCode2, EXIT_SUCCESS, "pgm to ascii did not return with EXIT_SUCCESS on cour25. Returned with %d", retCode2);

	retCode1 = WEXITSTATUS(system(cmp));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "test_output/AllWhite.asc does not match reference output");

}


Test(blackbox_tests, allWhite_to_birp, .timeout=10){

	system("mkdir -p test_output");

	char *cmd1 = "ulimit -t 10; bin/birp -i pgm < rsrc/newImages/AllWhite.pgm > test_output/AllWhite.birp";
	char *cmp = "cmp rsrc/newImages/AllWhite.birp test_output/AllWhite.birp";



	int retCode1 = WEXITSTATUS(system(cmd1));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "birp to pgm did not return with EXIT_SUCCESS on AllWhite. Returned with %d", retCode1);
	//int retCode2 = WEXITSTATUS(system(cmd2));
	//c/r_assert_eq(retCode2, EXIT_SUCCESS, "pgm to ascii did not return with EXIT_SUCCESS on cour25. Returned with %d", retCode2);

	retCode1 = WEXITSTATUS(system(cmp));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "test_output/AllWhite.birp does not match reference output");

}


Test(blackbox_tests, allBlack_to_ASCII, .timeout=10){

	system("mkdir -p test_output");

	char *cmd1 = "ulimit -t 10; bin/birp -o ascii < rsrc/newImages/allBlack.birp > test_output/allBlack.asc";
	char *cmp = "diff -B rsrc/newImages/allBlack.asc test_output/allBlack.asc";



	int retCode1 = WEXITSTATUS(system(cmd1));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "birp to asc did not return with EXIT_SUCCESS on allBlack. Returned with %d", retCode1);
	//int retCode2 = WEXITSTATUS(system(cmd2));
	//c/r_assert_eq(retCode2, EXIT_SUCCESS, "pgm to ascii did not return with EXIT_SUCCESS on cour25. Returned with %d", retCode2);

	retCode1 = WEXITSTATUS(system(cmp));
	cr_assert_eq(retCode1, 0, "test_output/allBlack.asc does not match reference output (asc file should be empty)");

}


Test(blackbox_tests, allBlack_to_birp, .timeout=10){

	system("mkdir -p test_output");

	char *cmd1 = "ulimit -t 10; bin/birp -i pgm < rsrc/newImages/allBlack.pgm > test_output/allBlack.birp";
	char *cmp = "cmp rsrc/newImages/allBlack.birp test_output/allBlack.birp";



	int retCode1 = WEXITSTATUS(system(cmd1));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "birp to pgm did not return with EXIT_SUCCESS on allBlack. Returned with %d", retCode1);
	//int retCode2 = WEXITSTATUS(system(cmd2));
	//c/r_assert_eq(retCode2, EXIT_SUCCESS, "pgm to ascii did not return with EXIT_SUCCESS on cour25. Returned with %d", retCode2);

	retCode1 = WEXITSTATUS(system(cmp));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "test_output/allBlack.birp does not match reference output");

}


Test(blackbox_tests, allBlack_to_pgm, .timeout=10){

	system("mkdir -p test_output");

	char *cmd1 = "ulimit -t 10; bin/birp -o pgm < rsrc/newImages/allBlack.birp > test_output/allBlack.pgm";
	char *cmp = "cmp rsrc/newImages/test_allBlack.pgm test_output/allBlack.pgm";



	int retCode1 = WEXITSTATUS(system(cmd1));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "birp to pgm did not return with EXIT_SUCCESS on allBlack. Returned with %d", retCode1);
	//int retCode2 = WEXITSTATUS(system(cmd2));
	//c/r_assert_eq(retCode2, EXIT_SUCCESS, "pgm to ascii did not return with EXIT_SUCCESS on cour25. Returned with %d", retCode2);

	retCode1 = WEXITSTATUS(system(cmp));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "test_output/allBlack.pgm does not match reference output");

}



//Half-black-vertical



Test(blackbox_tests, BWVert_to_PGM, .timeout=10){

	system("mkdir -p test_output");

	char *cmd1 = "ulimit -t 10; bin/birp -o pgm < rsrc/newImages/BWVert.birp > test_output/BWVert.pgm";
	char *cmp = "cmp rsrc/newImages/test_BWVert.pgm test_output/BWVert.pgm";


	int retCode1 = WEXITSTATUS(system(cmd1));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "birp to pgm did not return with EXIT_SUCCESS on BWVert. Returned with %d", retCode1);
	//int retCode2 = WEXITSTATUS(system(cmd2));
	//c/r_assert_eq(retCode2, EXIT_SUCCESS, "pgm to ascii did not return with EXIT_SUCCESS on cour25. Returned with %d", retCode2);

	retCode1 = WEXITSTATUS(system(cmp));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "test_output/BWVert.pgm does not match reference output");

}


Test(blackbox_tests, BWVert_to_Birp, .timeout=10){

	system("mkdir -p test_output");


	char *cmd1 = "ulimit -t 10; bin/birp -i pgm < rsrc/newImages/BWVert.pgm > test_output/BWVert.birp";
	char *cmp = "cmp rsrc/newImages/BWVert.birp test_output/BWVert.birp";



	int retCode1 = WEXITSTATUS(system(cmd1));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "pgm to birp did not return with EXIT_SUCCESS on BWVert. Returned with %d", retCode1);
	//int retCode2 = WEXITSTATUS(system(cmd2));
	//c/r_assert_eq(retCode2, EXIT_SUCCESS, "pgm to ascii did not return with EXIT_SUCCESS on cour25. Returned with %d", retCode2);

	retCode1 = WEXITSTATUS(system(cmp));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "test_output/BWVert.birp does not match reference output");

}



//Half-black-vertical
Test(blackbox_tests, BWVert_to_ASCII, .timeout=10){
	system("mkdir -p test_output");

	char *cmd1 = "ulimit -t 10; bin/birp -o ascii < rsrc/newImages/BWVert.birp > test_output/BWVert.asc";
	char *cmp = "diff -B rsrc/newImages/BWVert.asc test_output/BWVert.asc";
	char *cmd2 = "ulimit -t 10; bin/birp -i pgm -o ascii < rsrc/newImages/test_BWVert.pgm > test_output/BWVert-2.asc";
	char *cmp2 = "diff -B rsrc/newImages/BWVert.asc test_output/BWVert-2.asc";
	char *cmp3 = "diff -B test_output/BWVert.asc test_output/BWVert-2.asc";



	int retCode1 = WEXITSTATUS(system(cmd1));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "birp to ascii did not return with EXIT_SUCCESS on BWVert. Returned with %d", retCode1);
	int retCode2 = WEXITSTATUS(system(cmd2));
	cr_assert_eq(retCode2, EXIT_SUCCESS, "pgm to ascii did not return with EXIT_SUCCESS on BWVert. Returned with %d", retCode2);

	retCode1 = WEXITSTATUS(system(cmp));
	cr_assert_eq(retCode1, 0, "test_output/BWVert.asc does not match reference output");
	retCode1 = WEXITSTATUS(system(cmp2));
	cr_assert_eq(retCode1, 0, "test_output/BWVert-2.asc does not match reference output");
	retCode1 = WEXITSTATUS(system(cmp3));
	cr_assert_eq(retCode1, 0, "pgm-to-ascii does not equal birp-to-asciit");


}

//BW-Horizontal
//
Test(blackbox_tests, BWHoriz_to_ASCII, .timeout=10){

	system("mkdir -p test_output");

	char *cmd1 = "ulimit -t 10; bin/birp -o ascii < rsrc/newImages/BWHoriz.birp > test_output/BWHoriz.asc";
	char *cmp = "diff -B rsrc/newImages/BWHoriz.asc test_output/BWHoriz.asc";
	
	char *cmd2 = "ulimit -t 10; bin/birp -i pgm -o ascii < rsrc/newImages/test_BWHoriz.pgm > test_output/BWHoriz-2.asc";
	char *cmp2 = "diff -B rsrc/newImages/BWHoriz.asc test_output/BWHoriz-2.asc";
	char *cmp3 = "diff -B test_output/BWHoriz.asc test_output/BWHoriz-2.asc";



	int retCode1 = WEXITSTATUS(system(cmd1));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "birp to ascii did not return with EXIT_SUCCESS on BWHoriz. Returned with %d", retCode1);
	int retCode2 = WEXITSTATUS(system(cmd2));
	cr_assert_eq(retCode2, EXIT_SUCCESS, "pgm to ascii did not return with EXIT_SUCCESS on BWHoriz. Returned with %d", retCode2);

	retCode1 = WEXITSTATUS(system(cmp));
	cr_assert_eq(retCode1, 0, "test_output/BWHoriz.asc does not match reference output");
	retCode1 = WEXITSTATUS(system(cmp2));
	cr_assert_eq(retCode1, 0, "test_output/BWHoriz-2.asc does not match reference output");
	retCode1 = WEXITSTATUS(system(cmp3));
	cr_assert_eq(retCode1, 0, "pgm-to-ascii does not equal birp-to-asciit");


}


Test(blackbox_tests, BWHoriz_to_PGM, .timeout=10){

	system("mkdir -p test_output");

	char *cmd1 = "ulimit -t 10; bin/birp -o pgm < rsrc/newImages/BWHoriz.birp > test_output/BWHoriz.pgm";
	char *cmp = "cmp rsrc/newImages/test_BWHoriz.pgm test_output/BWHoriz.pgm";



	int retCode1 = WEXITSTATUS(system(cmd1));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "birp to pgm did not return with EXIT_SUCCESS on BWHoriz. Returned with %d", retCode1);
	//int retCode2 = WEXITSTATUS(system(cmd2));
	//c/r_assert_eq(retCode2, EXIT_SUCCESS, "pgm to ascii did not return with EXIT_SUCCESS on cour25. Returned with %d", retCode2);

	retCode1 = WEXITSTATUS(system(cmp));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "test_output/BWHoriz.pgm does not match reference output");

}


Test(blackbox_tests, BWHoriz_to_Birp, .timeout=10){

	system("mkdir -p test_output");

	char *cmd1 = "ulimit -t 10; bin/birp -i pgm < rsrc/newImages/BWHoriz.pgm > test_output/BWHoriz.birp";
	char *cmp = "cmp rsrc/newImages/BWHoriz.birp test_output/BWHoriz.birp";



	int retCode1 = WEXITSTATUS(system(cmd1));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "pgm to birp did not return with EXIT_SUCCESS on BWHoriz. Returned with %d", retCode1);
	//int retCode2 = WEXITSTATUS(system(cmd2));
	//c/r_assert_eq(retCode2, EXIT_SUCCESS, "pgm to ascii did not return with EXIT_SUCCESS on cour25. Returned with %d", retCode2);

	retCode1 = WEXITSTATUS(system(cmp));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "test_output/BWHoriz.birp does not match reference output");

}

Test(blackbox_tests, birp_to_birp_1, .timeout=10){

	system("mkdir -p test_output");

	char *cmd1 = "ulimit -t 10; bin/birp -i birp -o birp < rsrc/M.birp > test_output/M_birp.birp";
	char *cmp = "cmp test_output/M_birp.birp rsrc/M.birp";


	int retCode1 = WEXITSTATUS(system(cmd1));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "birp to birp did not return with EXIT_SUCCESS on M.birp. Returned with %d", retCode1);

	retCode1 = WEXITSTATUS(system(cmp));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "test_output/M_birp.birp does not match reference output");
}

Test(blackbox_tests, birp_to_birp_2, .timeout=10){

	system("mkdir -p test_output");

	char *cmd1 = "ulimit -t 10; bin/birp -i birp -o birp < rsrc/checker.birp > test_output/checker_birp.birp";
	char *cmp = "cmp test_output/checker_birp.birp rsrc/checker.birp";


	int retCode1 = WEXITSTATUS(system(cmd1));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "birp to birp did not return with EXIT_SUCCESS on checker.birp. Returned with %d", retCode1);

	retCode1 = WEXITSTATUS(system(cmp));
	cr_assert_eq(retCode1, EXIT_SUCCESS, "test_output/checker_birp.birp does not match reference output");
}
