#include <criterion/criterion.h>
#include "hi.h"

/**
 * This is Criterion, an excellent testing framwork for C.
 * It is also really well documented.
 * http://criterion.readthedocs.io/en/master/starter.html
 */
Test(CSE320_Suite, test_it_really_does_say_hi) {

    cr_assert_str_not_empty(say_hi(),
                            "say_hi() function returns empty string!");

    cr_assert_str_eq(say_hi(), "Hi",
                     "say_hi() function did not say 'Hi'");
}
