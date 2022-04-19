/* 
* This is a unit test for cmdproc module
*
* Author: Andre Brandao - 93021
* Author: Emanuel Pereira - 93235 
*/

#include <unity.h>
#include "cmdproc.h"

void setUp(void)
{
	return;
}

void tearDown(void)
{
	return;
}

/* Check if the commands work */
void test_1(void)
{
	resetCmdString();
	newCmdChar('#');
	newCmdChar('P');
	newCmdChar('1');
	newCmdChar('2');
	newCmdChar('3');
	newCmdChar((unsigned char)('P'+'1'+'2'+'3'));
	newCmdChar('!');

	TEST_ASSERT_EQUAL_INT(0, cmdProcessor());

	newCmdChar('#');
	newCmdChar('S');
	newCmdChar((unsigned char)('S'));
	newCmdChar('!');

	TEST_ASSERT_EQUAL_INT(0, cmdProcessor());
}

/* Check cmd string full return error */
void test_2(void)
{
	resetCmdString();
	newCmdChar('#');
	newCmdChar('R');
	newCmdChar('1');
	newCmdChar('2');
	newCmdChar('3');
	newCmdChar('3');
	newCmdChar('3');
	newCmdChar('3');
	newCmdChar('3');
	newCmdChar('3');

	TEST_ASSERT_EQUAL_INT(-1, newCmdChar('3'));
}

void test_3(void)
{
	/* Check empty command */
	resetCmdString();
	TEST_ASSERT_EQUAL_INT(-1, cmdProcessor());
	
	/* Check Invalid command */
	resetCmdString();
	newCmdChar('#');
	newCmdChar('R');
	newCmdChar('1');
	newCmdChar('2');
	newCmdChar('3');
	newCmdChar((unsigned char)('R'+'1'+'2'+'3'));
	newCmdChar('!');

	TEST_ASSERT_EQUAL_INT(-2, cmdProcessor());
}

void test_4(void)
{
	/* Check string format wrong */
	resetCmdString();
	newCmdChar('W');
	newCmdChar('P');
	newCmdChar('1');
	newCmdChar('2');
	newCmdChar('3');
	newCmdChar((unsigned char)('P'+'1'+'2'+'3'));
	newCmdChar('!');

	TEST_ASSERT_EQUAL_INT(-4, cmdProcessor());

	/* Check EOF_SYM error */
	resetCmdString();
	newCmdChar('#');
	newCmdChar('S');
	newCmdChar((unsigned char)('S'));
	newCmdChar('\n');

	TEST_ASSERT_EQUAL_INT(-4, cmdProcessor());
}

void test_5(void)
{
	/* Check Checksum error */
	resetCmdString();
	newCmdChar('#');
	newCmdChar('P');
	newCmdChar('1');
	newCmdChar('2');
	newCmdChar('3');
	newCmdChar((unsigned char)('P'+'1'+'1'+'1'));
	newCmdChar('!');

	TEST_ASSERT_EQUAL_INT(-3, cmdProcessor());

	resetCmdString();
	newCmdChar('#');
	newCmdChar('S');
	newCmdChar((unsigned char)('Q'));
	newCmdChar('!');

	TEST_ASSERT_EQUAL_INT(-3, cmdProcessor());
}

int main(void)
{
	UNITY_BEGIN();
	
	RUN_TEST(test_1);
	RUN_TEST(test_2);
	RUN_TEST(test_3);
	RUN_TEST(test_4);
	RUN_TEST(test_5);
			
	return UNITY_END();
}