#include <stdio.h>
#include <string.h>

#include "test_utlt.h"
#include "utlt_debug.h"

static TestCase utltTestList[] = { 
    {"3gppTypesTest", _3gppTypesTest, NULL},
    {"BuffTest", BuffTest, NULL},
    {"DebugTest", DebugTest, NULL},
    {"EventTest", EventTest, NULL},
    {"HashTest", HashTest, NULL},
    {"IndexTest", IndexTest, NULL},
    {"ListTest", ListTest, NULL},
    {"MqTest", MqTest, NULL},
    {"NetworkTest", NetworkTest, NULL},
    {"PoolTest", PoolTest, NULL},
    {"ThreadTest", ThreadTest, NULL},
    {"TimeTest", TimeTest, NULL},
    {"TimerTest", TimerTest, NULL},
    {"YamlTest", YamlTest, NULL}
};

// TODO : Add Alarm to prevent timeout
int main(int argc, char *argv[]) {
    //Status status;

    UTLT_Assert(TestInit() == STATUS_OK, return -1, "TestInit fial");
    int sizeOfTestList = sizeof(utltTestList)/sizeof(TestCase);
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            int idx = TestCaseArrayFindByName(utltTestList, sizeOfTestList, argv[i]);
            if (idx >= 0) TestAdd(&utltTestList[idx]);
        }
    } else {
        TestAddList(utltTestList, sizeOfTestList);
    }

    TestRun();

    UTLT_Assert(TestTerminate() == STATUS_OK, return -1, "TestTerminate fial");

    return 0;
}
